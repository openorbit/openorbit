/*
  Copyright 2006,2009,2012,2013 Mattias Holm <lorrden(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdbool.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/types.h"
#include <vmath/vmath.h>
#include "common/moduleinit.h"
#include <openorbit/log.h>
#include "io-manager.h"
#include "settings.h"
#include "rendering/scenegraph.h"
#include "common/palloc.h"

/*
 Note, due to the large world and floating point precision, when rotating the
 camera, we are rotating the world around the camera, not the camera itself.
 Second, which may be confusing, for the matrix math here, the y axis is up,
 thus, this is not really what is used for planets and the physics system where
 the z axis is up. We should perhaps fix this. One way would be to add an
 identity matrix which shuffles the y and z axises.
 */
struct sg_camera_t {
  sg_scene_t *scene;
  double4x4 proj_matrix;
  double4x4 view_matrix;

  lwcoord_t p0;        // Initial camera position
  lwcoord_t p1;        // End position of camera
  lwcoord_t p;         // Interpolated position of camera
  double3    dp;        // Velocity of camera

  quatd_t q;   // Quaternion used for view matrix (slerped from q0, q1)
  quatd_t q0;  // Quaternion from.
  quatd_t q1;  // Quaternion to (q0 * dq).
  quatd_t dq;  // Quaternion rot per time unit

  // If there is a target object, rotation is relative to that objects location
  // the source object constrains the lwc variable.
  sg_object_t *tgt; // Target object, must have rigid obj backing
  sg_object_t *src; // Source object, must have rigid obj backing

  double3 src_offset;  // Source object, must have rigid obj backing
};


#define ASSERT_CAM(c)\
  assert(!isnan(cam->q0.x));\
  assert(!isnan(cam->q1.x));\
  assert(!isnan(cam->q.x));\
  assert(!isnan(cam->dq.x));\
  assert(!isnan(cam->q0.y));\
  assert(!isnan(cam->q1.y));\
  assert(!isnan(cam->q.y));\
  assert(!isnan(cam->dq.y));\
  assert(!isnan(cam->q0.z));\
  assert(!isnan(cam->q1.z));\
  assert(!isnan(cam->q.z));\
  assert(!isnan(cam->dq.z));\
  assert(!isnan(cam->q0.w));\
  assert(!isnan(cam->q1.w));\
  assert(!isnan(cam->q.w));\
  assert(!isnan(cam->dq.w));


lwcoord_t
sg_camera_pos(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  return cam->p;
}

quatd_t
sg_camera_quat(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  return cam->q;
}

const double4x4*
sg_camera_project(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  return &cam->proj_matrix;
}

void
sg_camera_update_modelview(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  double4x4 qm;

  md4_ident_z_up(cam->view_matrix);
  qd_md4_convert_inv(qm, cam->q);
  md4_mul2(cam->view_matrix, qm);
}

const double4x4*
sg_camera_modelview(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  return &cam->view_matrix;
}

sg_camera_t*
sg_new_camera(sg_scene_t *scene)
{
  sg_camera_t *cam = smalloc(sizeof(sg_camera_t));

  // Initial rotation correspond to a vector pointing down the negative x axis.
  cam->dq = QD_IDENT;
  cam->q0 = QD_IDENT;
  cam->q1 = QD_IDENT;
  cam->q = QD_IDENT;

  sg_camera_set_perspective(cam, 1.0);

  sg_camera_update_modelview(cam);

  cam->scene = scene;
  if (scene) {
    sg_scene_set_cam(scene, cam);
  }
  ASSERT_CAM(cam);

  return cam;
}

void
sg_camera_track_object(sg_camera_t *cam, sg_object_t *obj)
{
  ASSERT_CAM(cam);

  cam->tgt = obj;
}

sg_object_t*
sg_camera_get_tracked_object(sg_camera_t *cam)
{
  return cam->tgt;
}


void
sg_camera_follow_object(sg_camera_t *cam, sg_object_t *obj)
{
  ASSERT_CAM(cam);

  cam->src = obj;

  if (cam->tgt == NULL) {
    cam->q0 = sg_object_get_q0(obj);
    cam->q1 = qd_normalise(qd_mul(cam->q0, cam->dq));
  }

  ASSERT_CAM(cam);
}

void
sg_camera_set_follow_offset(sg_camera_t *cam, double3 offs)
{
  ASSERT_CAM(cam);

  if (cam->src) {
    cam->src_offset = offs;
  }
  ASSERT_CAM(cam);
}


void
sg_camera_set_perspective(sg_camera_t *cam, float perspective)
{
  ASSERT_CAM(cam);

  md4_perspective(cam->proj_matrix, M_PI_4, perspective,
                  0.1, 1000000000000.0);

  ASSERT_CAM(cam);
}

void
sg_camera_interpolate(sg_camera_t *cam, float t)
{
  assert(t >= 0.0);
  assert(t <= 1.0);

  if (cam->tgt) {
    cam->q = qd_slerp(cam->q0, cam->q1, t);
    double3x3 R;
    qd_md3_convert(R, cam->q);
    cam->src_offset = md3_v_mul(R, vd3_set(vd3_abs(cam->src_offset), 0.0, 0.0));

    cam->p0 = sg_object_get_p0(cam->src);
    lwc_translate3dv(&cam->p0, cam->src_offset);

    cam->p1 = sg_object_get_p1(cam->src);
    lwc_translate3dv(&cam->p1, cam->src_offset);
    lwc_translate3dv(&cam->p1, cam->dp);

    cam->p = cam->p0;
    double3 d = vd3_s_mul(lwc_dist(&cam->p1, &cam->p0), t);
    lwc_translate3dv(&cam->p, d);
    ASSERT_CAM(cam);
  } else if (cam->src) {
    quatd_t q = qd_slerp(cam->q0, cam->q1, t);
    cam->q = sg_object_get_quat(cam->src);
    cam->q = qd_mul(cam->q, q);

    cam->p = cam->p0;
    double3 d = vd3_s_mul(lwc_dist(&cam->p1, &cam->p0), t);
    lwc_translate3dv(&cam->p, d);
  } else {
    cam->q = qd_slerp(cam->q0, cam->q1, t);

    cam->p = cam->p0;
    double3 d = vd3_s_mul(lwc_dist(&cam->p1, &cam->p0), t);
    lwc_translate3dv(&cam->p, d);
  }

  ASSERT_CAM(cam);
}

// Synchronises camera with target and follow objects, t=0.0
void
sg_camera_sync(sg_camera_t *cam)
{
  ASSERT_CAM(cam);
  cam->src_offset += cam->dp;

  if (cam->src) {
    // Following an object
    cam->p0 = sg_object_get_p0(cam->src);
    lwc_translate3dv(&cam->p0, cam->src_offset);

    cam->p1 = sg_object_get_p1(cam->src);
    lwc_translate3dv(&cam->p1, cam->src_offset);
    lwc_translate3dv(&cam->p1, cam->dp);

    if (cam->tgt) {
      // Orbiting an object
      cam->q0 = cam->q1;
      cam->q1 = qd_normalise(qd_mul(cam->q0, cam->dq));
      cam->q = qd_slerp(cam->q0, cam->q1, 0.0);

      double3x3 R;
      qd_md3_convert(R, cam->q);
      cam->src_offset = md3_v_mul(R, vd3_set(vd3_abs(cam->src_offset), 0.0, 0.0));

      cam->p0 = sg_object_get_p0(cam->src);
      lwc_translate3dv(&cam->p0, cam->src_offset);

      cam->p1 = sg_object_get_p1(cam->src);
      lwc_translate3dv(&cam->p1, cam->src_offset);
      lwc_translate3dv(&cam->p1, cam->dp);

      cam->p = cam->p0;
      //double3 d = lwc_dist(&cam->p1, &cam->p0);
      //lwc_translate3fv(&cam->p, vf3_s_mul(d, 0.0));
      ASSERT_CAM(cam);
    } else {
      cam->q0 = cam->q1;
      cam->q1 = qd_normalise(qd_mul(cam->q0, cam->dq));
      quatd_t q = qd_slerp(cam->q0, cam->q1, 0.0);

      cam->q = sg_object_get_q0(cam->src);
      cam->q = qd_mul(cam->q, q);
      ASSERT_CAM(cam);
    }
  } else {
    cam->q0 = cam->q1;
    cam->q1 = qd_normalise(qd_mul(cam->q0, cam->dq));
    cam->q = qd_slerp(cam->q0, cam->q1, 0.0);
    ASSERT_CAM(cam);
  }
  ASSERT_CAM(cam);
}

/* Camera actions, registered as action handlers */

// TODO: Cleanup, move camera rotate io handler to sim module
sg_scene_t* sim_get_scene(void);

void
sg_camera_rotate_hat(int buttonVal, void *data)
{
  //log_info("hat pushed %d", buttonVal);
  sg_scene_t *sc = sim_get_scene();
  sg_camera_t *cam = sg_scene_get_cam(sc);

  ASSERT_CAM(cam);

  // Note that the camera control will rotate the camera based on WCT.
  // This differ from the normal rotation of objects which is based on SRT.
  // The interpolation doesn't really care, it just expresses time as a
  // normalized value where 0 is the time of the last physics system sync
  // and 1.0 is the expected time of the next sync.
  // We thus need to take the frequency (not the SRT period) here.
  float wct_freq;
  config_get_float_def("openorbit/sim/freq", &wct_freq, 20.0); // Hz

  if ((cam->src == cam->tgt) && cam->src) {
    // We are targeting our follow object this means orbiting it
    if (buttonVal == -1) {
      cam->dq = QD_IDENT;
    } else {
      cam->dq = qd_rot(0.0,
                       cosf(DEG_TO_RAD(buttonVal)),
                       sinf(DEG_TO_RAD(buttonVal)),
                       M_PI_2 / wct_freq);
    }
    cam->q1 = qd_mul(cam->q0, cam->dq);
  } else {
    if (buttonVal == -1) {
      cam->dq = QD_IDENT;
    } else {
      cam->dq = qd_rot(0.0,
                       cosf(DEG_TO_RAD(buttonVal)),
                       sinf(DEG_TO_RAD(buttonVal)),
                       M_PI_2 / wct_freq);
    }
    cam->q1 = qd_mul(cam->q0, cam->dq);
  }

  ASSERT_CAM(cam);
}

void
sg_camera_move_forward(int buttonVal, void *data)
{
  sg_scene_t *sc = sim_get_scene();
  sg_camera_t *cam = sg_scene_get_cam(sc);

  ASSERT_CAM(cam);

  float wct_freq;
  config_get_float_def("openorbit/sim/freq", &wct_freq, 20.0); // Hz

  if ((cam->src == cam->tgt) && cam->src) {
    // We are targeting our follow object this means orbiting it
    if (buttonVal == 0) {
      cam->dp = vd3_set(0, 0, 0);
    } else {
      // Key down
      double3x3 camrot;
      qd_md3_convert(camrot, cam->q);
      double radius = sg_object_get_radius(cam->tgt);
      lwcoord_t obj_pos = sg_object_get_p(cam->tgt);
      lwcoord_t cam_pos = sg_camera_pos(cam);
      double3 distv = lwc_dist(&cam_pos, &obj_pos);
      double dist = vd3_abs(distv);
      cam->dp = md3_v_mul(camrot, vd3_set(-(dist-radius) / wct_freq, 0, 0));
    }
    cam->p1 = sg_object_get_p1(cam->src);
    lwc_translate3dv(&cam->p1, cam->src_offset);
    lwc_translate3dv(&cam->p1, cam->dp);
  } else {
    if (buttonVal == 0) {
      // Key released
    } else {
      // Key down
    }
  }
}

void
sg_camera_move_back(int buttonVal, void *data)
{
  sg_scene_t *sc = sim_get_scene();
  sg_camera_t *cam = sg_scene_get_cam(sc);

  ASSERT_CAM(cam);

  float wct_freq;
  config_get_float_def("openorbit/sim/freq", &wct_freq, 20.0); // Hz

  if ((cam->src == cam->tgt) && cam->src) {
    // We are targeting our follow object this means orbiting it
    if (buttonVal == 0) {
      cam->dp = vd3_set(0, 0, 0);
    } else {
      // Key down
      double3x3 camrot;
      qd_md3_convert(camrot, cam->q);
      float radius = sg_object_get_radius(cam->tgt);
      lwcoord_t obj_pos = sg_object_get_p(cam->tgt);
      lwcoord_t cam_pos = sg_camera_pos(cam);
      double3 distv = lwc_dist(&cam_pos, &obj_pos);
      double dist = vd3_abs(distv);
      cam->dp = md3_v_mul(camrot, vd3_set((dist-radius) / wct_freq, 0, 0));
    }
    cam->p1 = sg_object_get_p1(cam->src);
    lwc_translate3dv(&cam->p1, cam->src_offset);
    lwc_translate3dv(&cam->p1, cam->dp);
  } else {
    if (buttonVal == 0) {
      cam->dp = vd3_set(0, 0, 0);
    } else {
      // Key down
    }
  }
}

// TODO: Should move to sim part, where we will keep all the io stuff
MODULE_INIT(sgcamera, "iomanager", NULL) {
  io_reg_action_handler("cam-rotate", sg_camera_rotate_hat, IO_BUTTON_HAT, NULL);
  io_reg_action_handler("cam-move-forward", sg_camera_move_forward, IO_BUTTON_PUSH, NULL);
  io_reg_action_handler("cam-move-back", sg_camera_move_back, IO_BUTTON_PUSH, NULL);
}
