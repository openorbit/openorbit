/*
  Copyright 2006,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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
struct sg_camera_t {
  float4x4 proj_matrix;
  float4x4 view_matrix;

  sg_camera_type_t type;

  union {
    struct {
      lwcoord_t lwc;
      float3 dp;
      quaternion_t q;
      quaternion_t dq;
    } free;
    struct {
      sg_object_t *obj;
      float3 r; // With this offset
      quaternion_t q; // and this rotation (rotate before translation)
      quaternion_t dq; // Delta rotation
    } fixed;
    struct {
      sg_object_t *obj;
      float ra, dec;
      float dra, ddec, dr;
      float r, zoom;
    } orbiting;
  };
};

sg_camera_t*
sg_new_free_camera(const lwcoord_t *pos)
{
  sg_camera_t *cam = smalloc(sizeof(sg_camera_t));
  cam->type = SG_CAMERA_FREE;
  cam->free.lwc = *pos;
  mf4_perspective(cam->proj_matrix, DEG_TO_RAD(90.0), 1.0, 0.1, 1000000000000.0);
  mf4_lookat(cam->view_matrix, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

  return cam;
}

sg_camera_t*
sg_new_fixed_camera(sg_object_t *obj)
{
  sg_camera_t *cam = smalloc(sizeof(sg_camera_t));
  cam->type = SG_CAMERA_FIXED;
  cam->fixed.obj = obj;

  mf4_perspective(cam->proj_matrix, DEG_TO_RAD(90.0), 1.0, 0.1, 1000000000000.0);
  mf4_lookat(cam->view_matrix, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

  return cam;
}

sg_camera_t*
sg_new_orbiting_camera(sg_object_t *obj)
{
  sg_camera_t *cam = smalloc(sizeof(sg_camera_t));
  cam->type = SG_CAMERA_ORBITING;
  cam->orbiting.obj = obj;
  
  mf4_perspective(cam->proj_matrix, DEG_TO_RAD(90.0), 1.0, 0.1, 1000000000000.0);
  mf4_lookat(cam->view_matrix, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);

  return cam;
}


sg_camera_type_t
sg_camera_get_type(sg_camera_t *cam)
{
  return cam->type;
}

const float4x4*
sg_camera_get_projection(sg_camera_t *cam)
{
  return &cam->proj_matrix;
}

const float4x4*
sg_camera_get_view(sg_camera_t *cam)
{
  return &cam->view_matrix;
}

lwcoord_t
sg_camera_free_get_lwc(sg_camera_t *cam)
{
  return cam->free.lwc;
}

float3
sg_camera_free_get_velocity(sg_camera_t *cam)
{
  return cam->free.dp;
}

void
sg_camera_free_set_velocity(sg_camera_t *cam, float3 v)
{
  cam->free.dp = v;
}

void
sg_camera_free_increase_velocity(sg_camera_t *cam, float3 dv)
{
  cam->free.dp += dv;
}


quaternion_t
sg_camera_free_get_quaternion(sg_camera_t *cam)
{
  return cam->free.q;
}

quaternion_t
sg_camera_free_get_delta_quaternion(sg_camera_t *cam)
{
  return cam->free.dq;
}

sg_object_t*
sg_camera_fixed_get_obj(sg_camera_t *cam)
{
  return cam->fixed.obj;
}

void
sg_camera_fixed_set_obj(sg_camera_t *cam, sg_object_t *obj)
{
  cam->fixed.obj = obj;
}

float3
sg_camera_fixed_get_offset(sg_camera_t *cam)
{
  return cam->fixed.r;
}

quaternion_t
sg_camera_fixed_get_quaternion(sg_camera_t *cam)
{
  return cam->fixed.q;
}
quaternion_t
sg_camera_fixed_get_delta_quaternion(sg_camera_t *cam)
{
  return cam->fixed.dq;
}

sg_object_t*
sg_camera_orbiting_get_obj(sg_camera_t *cam)
{
  return cam->orbiting.obj;
}
void
sg_camera_orbiting_set_obj(sg_camera_t *cam, sg_object_t *obj)
{
  cam->orbiting.obj = obj;
}

float
sg_camera_orbiting_get_ra(const sg_camera_t *cam)
{
  return cam->orbiting.ra;
}
float
sg_camera_orbiting_get_dec(const sg_camera_t *cam)
{
  return cam->orbiting.dec;
}
float
sg_camera_orbiting_get_delta_ra(const sg_camera_t *cam)
{
  return cam->orbiting.dra;
}
float
sg_camera_orbiting_get_delta_dec(const sg_camera_t *cam)
{
  return cam->orbiting.ddec;
}

float
sg_camera_orbiting_get_delta_radius(const sg_camera_t *cam)
{
  return cam->orbiting.dr;
}
float
sg_camera_orbiting_get_radius(const sg_camera_t *cam)
{
  return cam->orbiting.r;
}
float
sg_camera_orbiting_get_zoom(const sg_camera_t *cam)
{
  return cam->orbiting.zoom;
}


/* Camera actions, registered as action handlers */


void sg_camera_rotate_hat(int state, void *data);

struct str_action_triplet {
  const char *confKey;
  const char *ioKey;
  IObuttonhandlerfunc action;
};

MODULE_INIT(camera, "iomanager", NULL) {
  ooLogTrace("initialising 'camera' module");
  const char *key;
  static const struct str_action_triplet keyBindings[] = {
    {"openorbit/controls/hat/cam-rotate", "cam-rotate", sg_camera_rotate_hat},
  };

  ioRegActionHandler(keyBindings[0].ioKey, keyBindings[0].action,
                     IO_BUTTON_HAT, NULL);

  // Register camera actions
  //for (int i = 1 ; i < sizeof(keyBindings)/sizeof(struct str_action_triplet); ++ i) {
  //  ioRegActionHandler(keyBindings[i].ioKey, keyBindings[i].action,
  //                    IO_BUTTON_PUSH, NULL);
  //}
}

void
sgCamInit(void)
{
}


void
sg_camera_animate(sg_camera_t *cam, float dt)
{
  assert(cam && "cannot animate non existant camera");
  switch (cam->type) {
    case SG_CAMERA_FIXED: {
      // Camera is pegged to object, not much to do here
      // object animation takes care of moving the camera
      //float4 np = vf4_add(sg_object_get_pos(cam->fixed.obj), cam->fixed.r);
      mf4_ident(cam->view_matrix);
      float4x4 a;
      mf4_make_translate(a, cam->fixed.r);
      float3x3 R;

      quaternion_t q = q_s_mul(cam->fixed.dq, dt);
      cam->fixed.q = q_mul(cam->fixed.q, q);
      q_mf4_convert(R, cam->fixed.q);

      //      SGfixedcam* fix = (SGfixedcam*)cam;
      //quaternion_t q = fix->body->q;
      //q = q_mul(q, fix->q);
      //matrix_t m;
      //q_m_convert(&m, q);
      //matrix_t mt;
      //m_transpose(&mt, &m);
      //glMultMatrixf((GLfloat*)&mt);

      break;
    }
    case SG_CAMERA_FREE: {
      quaternion_t q = q_s_mul(cam->free.dq, dt);
      cam->free.q = q_mul(cam->free.q, q);
      float3 dp = vf3_s_mul(cam->free.dp, dt);
      lwc_translate3fv(&cam->free.lwc, dp);

      //      SGfreecam* freec = (SGfreecam*)cam;
      //quaternion_t q = freec->q;
      float4x4 m;
      q_mf4_convert(m, cam->free.q);
      float4x4 mt;
      mf4_transpose2(mt, m);
      //glMultMatrixf((GLfloat*)&m);

      break;
    }
    case SG_CAMERA_ORBITING: {
      cam->orbiting.ra += cam->orbiting.dra * dt;
      cam->orbiting.dec += cam->orbiting.ddec * dt;
      cam->orbiting.r += cam->orbiting.dr * dt;

      mf4_lookat(cam->view_matrix,
                 0.0f, 0.0f, 0.0f,
                 -cam->orbiting.r * cosf(cam->orbiting.dec),
                 -cam->orbiting.r * sinf(cam->orbiting.dec),
                 -cam->orbiting.r * sinf(cam->orbiting.ra),
                 0.0f, 0.0f, 1.0f);
      //SGorbitcam* ocam = (SGorbitcam*)cam;

      //gluLookAt(0.0, 0.0, 0.0,
      //          -ocam->r*cos(ocam->dec),
      //          -ocam->r*sin(ocam->dec),
      //          -ocam->r*sin(ocam->ra),
      //          0.0, 0.0, 1.0);

      break;
    }
  }
}



void
sg_camera_move(sg_camera_t *cam)
{
  switch (cam->type) {
  case SG_CAMERA_FIXED: {
    //float3 p = cam->fixed.obj->p.offs;
    //p = vf3_add(p, cam->fixed.r);
    //quaternion_t q = cam->fixed.body->q;
    //q = q_mul(q, cam->fixed.q);
    //float4x4 m;
    //q_mf4_convert(m, q);
    //      matrix_t mt;
    //      m_transpose(&mt, &m);

    //glMultMatrixf((GLfloat*)&m);
    //glTranslatef(-vf3_x(p), -vf3_y(p), -vf3_z(p));

    //  float4x4 t;
    //  mf4_translate(t, -cam->fixed.lwc.offs);
    //  mf4_mul2(cam->viewMatrix, t);

    break;}
  case SG_CAMERA_FREE: {
    //      SGfreecam *freecam = (SGfreecam*)cam;
    float4x4 t;
    mf4_make_translate(t, -cam->free.lwc.offs);
    mf4_mul2(cam->view_matrix, t);
    //      glTranslatef(-vf3_x(freecam->lwc.offs),
    //                   -vf3_y(freecam->lwc.offs),
    //                   -vf3_z(freecam->lwc.offs));

    break;}
  case SG_CAMERA_ORBITING: {
    float3 p = vf3_set(cam->orbiting.r*cos(cam->orbiting.dec),
                       cam->orbiting.r*sin(cam->orbiting.dec),
                       cam->orbiting.r*sin(cam->orbiting.ra));
    //float3 cogOffset = mf3_v_mul(cam->orbiting.obj->R, cam->orbiting.obj->m.cog);
    //glTranslatef(-(p[0] + cogOffset.x),
    //             -(p[1] + cogOffset.y),
    //             -(p[2] + cogOffset.z));
    float4x4 t;
    mf4_make_translate(t, -p);
    mf4_mul2(cam->view_matrix, t);
    break; }
  }
}

void
sg_camera_rotate_hat(int buttonVal, void *data)
{

}
