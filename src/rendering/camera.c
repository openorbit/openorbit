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
#include "camera.h"
#include "settings.h"
#include "scenegraph.h"
#include "palloc.h"

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
  float4x4 proj_matrix;
  float4x4 view_matrix;

  lwcoord_t p;        // Actual pos of camera (lwc + lwc_offset)
  lwcoord_t lwc;      // Pos of camera source, may be constrained by src object
  float3 lwc_offset;  // Offset from target lwc (this is the thing moving)

  float3 dp;          // Velocity

  quaternion_t aq;  // Actual quaternion (tq * rq)
  quaternion_t tq;  // Rotation quaternion, this is the target direction
  quaternion_t rq;  // Relative rotation to tq
  quaternion_t dq;  // Rotation per time unit of rq

  // If there is a target object, rotation is relative to that objects location
  // the source object constrains the lwc variable.
  sg_object_t *tgt; // Target object, must have rigid obj backing
  sg_object_t *src; // Source object, must have rigid obj backing

  float3 src_dir; // Source direction
  float3 src_offset; // Source object, must have rigid obj backing
};


lwcoord_t
sg_camera_pos(sg_camera_t *cam)
{
  return cam->p;
}

quaternion_t
sg_camera_quat(sg_camera_t *cam)
{
  return cam->aq;
}

const float4x4*
sg_camera_project(sg_camera_t *cam)
{
  return &cam->proj_matrix;
}

void
sg_camera_update_modelview(sg_camera_t *cam)
{
  float4x4 qm;
  q_mf4_convert(qm, cam->aq);
  mf4_ident_z_up(cam->view_matrix);
  mf4_mul2(cam->view_matrix, qm);
}

const float4x4*
sg_camera_modelview(sg_camera_t *cam)
{
  return &cam->view_matrix;
}

void
sg_camera_step(sg_camera_t * cam, float dt)
{
  quaternion_t q = q_s_mul(cam->dq, dt);
  cam->rq = q_normalise(q_mul(q, cam->rq));
  cam->aq = q_normalise(q_mul(cam->rq, cam->tq));

  cam->lwc_offset += cam->dp * dt;

  if (cam->src) {
    sg_object_get_lwc(cam->src, &cam->lwc);
  }
  cam->p = cam->lwc;
  lwc_translate3fv(&cam->p, cam->lwc_offset);

  // Tracking the follow object
  if ((cam->src == cam->tgt) && cam->src) {
    lwcoord_t tgt_lwc;
    sg_object_get_lwc(cam->tgt, &tgt_lwc);

    // Old target vector
    float3 target = vf3_normalise(lwc_dist(&tgt_lwc, &cam->p));
    float4x4 matrix;

    q_mf4_convert(matrix, cam->aq);
    float4 v = mf4_v_mul(matrix, vf4_set(-1.0, 0.0, 0.0, 0.0));
    v = vf4_s_mul(v, vf3_abs(target));
    cam->lwc = tgt_lwc;
    lwc_translate3fv(&cam->lwc, v);
    cam->p = cam->lwc;
    // We have the camera quaternion, we need to make a vector point in its
    // direction
    // Orbiting object, this means that we do not rotate the rq quat, but move
    // and ensure that the new tq points at the target
    //quaternion_t qtarget = q_rotv(target, 0.0);

  }

  sg_scene_camera_moved(cam->scene, cam->dp * dt);

  sg_camera_update_modelview(cam);
}

void
sg_camera_update_constraints(sg_camera_t *cam)
{
  // Update lwc for the object we follow
  if (cam->src) {
    sg_object_get_lwc(cam->src, &cam->lwc);
    cam->p = cam->lwc;
    lwc_translate3fv(&cam->p, cam->lwc_offset);
    //cam->dp = sg_object_get_vel(cam->src);
  }

  // Are we pointing somewhere, if so we need to get the target vector
  if (cam->tgt) {
    lwcoord_t target;

    sg_object_get_lwc(cam->tgt, &target);
    float3 dir = vf3_normalise(lwc_dist(&target, &cam->p)); // Points toward dir
    cam->tq = q_rotv(dir, 0.0);
    cam->aq = q_normalise(q_mul(cam->rq, cam->tq));
  } else if (cam->src) {
    // We are pointing relative to follow object
    cam->tq = sg_object_get_quat(cam->src);
    cam->aq = q_normalise(q_mul(cam->rq, cam->tq));
  }
  sg_camera_update_modelview(cam);
}

sg_camera_t*
sg_new_camera(sg_scene_t *scene)
{
  sg_camera_t *cam = smalloc(sizeof(sg_camera_t));

  cam->dq = Q_IDENT;
  cam->rq = q_mul(q_rot(0.0, 1.0, 0.0, M_PI_2), q_rot(1.0, 0.0, 0.0, M_PI_2));
  cam->tq = Q_IDENT;
  cam->aq = cam->rq;

  mf4_perspective(cam->proj_matrix, M_PI_2, 1.0, 0.1, 1000000000000.0);
  mf4_ident_z_up(cam->view_matrix);
  float4x4 tmp;
  mf4_lookat(tmp, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
  mf4_mul2(cam->view_matrix, tmp);


  sg_camera_update_modelview(cam);

  cam->scene = scene;
  if (scene) {
    sg_scene_set_cam(scene, cam);
  }

  return cam;
}

void
sg_camera_track_object(sg_camera_t *cam, sg_object_t *obj)
{
  cam->tgt = obj;
}

void
sg_camera_follow_object(sg_camera_t *cam, sg_object_t *obj)
{
  cam->src = obj;

  if (cam->tgt == NULL) {
    cam->tq = sg_object_get_quat(obj);
  }
}

void
sg_camera_set_follow_offset(sg_camera_t *cam, float3 offs)
{
  if (cam->src) {
    cam->lwc_offset = offs;
  }
}


void
sg_camera_set_perspective(sg_camera_t *cam, float perspective)
{
  mf4_perspective(cam->proj_matrix, DEG_TO_RAD(90.0), perspective,
                  0.1, 1000000000000.0);
}

/* Camera actions, registered as action handlers */

// TODO: Cleanup, move camera rotate io handler to sim module
sg_scene_t* sim_get_scene(void);

void
sg_camera_rotate_hat(int buttonVal, void *data)
{
  //ooLogInfo("hat pushed %d", buttonVal);
  sg_scene_t *sc = sim_get_scene();
  sg_camera_t *cam = sg_scene_get_cam(sc);

  if ((cam->src == cam->tgt) && cam->src) {
    // We are targeting our follow object this means orbiting it
    if (buttonVal == -1) {
      cam->dq = Q_IDENT;
    } else {
      cam->dq = q_rot(0.0,
                      -cosf(DEG_TO_RAD(buttonVal)),
                      -sinf(DEG_TO_RAD(buttonVal)),
                      0.1);
    }
  } else {
    if (buttonVal == -1) {
      cam->dq = Q_IDENT;
    } else {
      cam->dq = q_rot(0.0,
                      cosf(DEG_TO_RAD(buttonVal)),
                      sinf(DEG_TO_RAD(buttonVal)),
                      0.1);
    }
  }
}

// TODO: Should move to sim part, where we will keep all the io stuff
MODULE_INIT(sgcamera, "iomanager", NULL) {
  ioRegActionHandler("cam-rotate", sg_camera_rotate_hat, IO_BUTTON_HAT, NULL);
}
