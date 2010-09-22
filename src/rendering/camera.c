/*
  Copyright 2006,2009 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdbool.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif

#include <vmath/vmath.h>
#include "log.h"
#include "io-manager.h"
#include "camera.h"
#include "settings.h"
#include "scenegraph.h"
#include "scenegraph-private.h"

/* Camera actions, registered as action handlers */


void sgCamFwd(bool up, void *data);
void sgCamBack(bool up, void *data);
void sgCamLeft(bool up, void *data);
void sgCamRight(bool up, void *data);
void sgCamUp(bool up, void *data);
void sgCamDown(bool up, void *data);
void sgCamRollLeft(bool up, void *data);
void sgCamRollRight(bool up, void *data);
void sgCamYawLeft(bool up, void *data);
void sgCamYawRight(bool up, void *data);
void sgCamPitchDown(bool up, void *data);
void sgCamPitchUp(bool up, void *data);


struct str_action_triplet {
  const char *confKey;
  const char *ioKey;
  OObuttonhandlerfunc action;
};

void
sgCamInit(void)
{
  const char *key;
  static const struct str_action_triplet keyBindings[] = {
    {"openorbit/controls/keyboard/cam-fwd", "cam-fwd", sgCamFwd},
    {"openorbit/controls/keyboard/cam-back", "cam-back", sgCamBack},
    {"openorbit/controls/keyboard/cam-left", "cam-left", sgCamLeft},
    {"openorbit/controls/keyboard/cam-right", "cam-right", sgCamRight},
    {"openorbit/controls/keyboard/cam-up", "cam-up", sgCamUp},
    {"openorbit/controls/keyboard/cam-down", "cam-down", sgCamDown},
    {"openorbit/controls/keyboard/cam-roll-left", "cam-roll-left", sgCamRollLeft},
    {"openorbit/controls/keyboard/cam-roll-right", "cam-roll-right", sgCamRollRight},
    {"openorbit/controls/keyboard/cam-yaw-left", "cam-yaw-left", sgCamYawLeft},
    {"openorbit/controls/keyboard/cam-yaw-right", "cam-yaw-right", sgCamYawRight},
    {"openorbit/controls/keyboard/cam-pitch-down", "cam-pitch-down", sgCamPitchDown},
    {"openorbit/controls/keyboard/cam-pitch-up", "cam-pitch-up", sgCamPitchUp}
  };

  // Register camera actions
  for (int i = 0 ; i < sizeof(keyBindings)/sizeof(struct str_action_triplet); ++ i) {
    ooIoRegCKeyHandler(keyBindings[i].ioKey, keyBindings[i].action);
    ooConfGetStrDef(keyBindings[i].confKey, &key, NULL);
    if (key) {
      ooIoBindKeyHandler(key, keyBindings[i].ioKey, 0, 0);
      ooIoBindKeyHandler(key, keyBindings[i].ioKey, 1, 0);
    }
  }
}

SGcam*
sgNewFreeCam(SGscenegraph *sg, SGscene *sc,
               float x, float y, float z, float rx, float ry, float rz)
{
  assert(sg != NULL);
  SGfreecam *cam = malloc(sizeof(SGfreecam));
  cam->super.kind = SGCam_Free;
  cam->super.scene = sc;

  cam->q = q_rot(rx,ry,rz, 0.0f);
  cam->dp = vf3_set(0.0,0.0,0.0);
  cam->dq = q_rot(rx,ry,rz, 0.0f);

  ooLwcSet(&cam->lwc, x, y, z);

  obj_array_push(&sg->cams, cam);
  return (SGcam*)cam;
}

SGcam*
sgNewFixedCam(SGscenegraph *sg, SGscene *sc, PLobject *body,
                float dx, float dy, float dz, float rx, float ry, float rz)
{
    SGfixedcam *cam = malloc(sizeof(SGfixedcam));
    cam->super.kind = SGCam_Fixed;

    cam->super.scene = sc;
    cam->body = body;
    cam->r = vf3_set(dx,dy,dz);
    cam->q = q_rot(rx,ry,rz, 0.0f);

    obj_array_push(&sg->cams, cam);
    return (SGcam*)cam;
}

SGcam*
sgNewOrbitCam(SGscenegraph *sg, SGscene *sc, PLobject *body,
                float ra, float dec, float r)
{
  SGorbitcam *ocam = malloc(sizeof(SGorbitcam));
  ocam->super.kind = SGCam_Orbit;
  ocam->super.scene = sc;
  ocam->body = body;
  ocam->ra = ra;
  ocam->dec = dec;

  ocam->dr = 0.0;
  ocam->dra = 0.0;
  ocam->ddec = 0.0;

  ocam->zoom = r;
  ocam->r = r;
  obj_array_push(&sg->cams, ocam);
  return (SGcam*)ocam;
}

void
sgSetCamTarget(SGcam *cam, PLobject *body)
{
  assert(cam != NULL);
  assert(body != NULL);

  if (cam->kind == SGCam_Fixed) {
    SGfixedcam *fixCam = (SGfixedcam*)cam;
    fixCam->body = body;
  } else if (cam->kind == SGCam_Orbit) {
    SGorbitcam *orbCam = (SGorbitcam*)cam;
    orbCam->body = body;
  }
}


// Only rotate, used for things like sky painting that requires camera rotation but not
// translation
void
sgCamRotate(SGcam *cam)
{
  assert(cam != NULL && "cam not set");
  glMatrixMode(GL_MODELVIEW);
  switch (cam->kind) {
  case SGCam_Orbit:
    {
      SGorbitcam* ocam = (SGorbitcam*)cam;
      gluLookAt(0.0, 0.0, 0.0,
                -ocam->r*cos(ocam->dec),
                -ocam->r*sin(ocam->dec),
                -ocam->r*sin(ocam->ra),
                0.0, 0.0, 1.0);
    }
    break;
  case SGCam_Fixed:
    {
      SGfixedcam* fix = (SGfixedcam*)cam;
      quaternion_t q = fix->body->q;
      q = q_mul(q, fix->q);
      matrix_t m;
      q_m_convert(&m, q);
      matrix_t mt;
      m_transpose(&mt, &m);
      glMultMatrixf((GLfloat*)&mt);
    }
    break;
  case SGCam_Free:
    {
      SGfreecam* freec = (SGfreecam*)cam;
      quaternion_t q = freec->q;
      matrix_t m;
      q_m_convert(&m, q);
      //      matrix_t mt;
      //m_transpose(&mt, &m);

      glMultMatrixf((GLfloat*)&m);
    }
    break;
  default:
    assert(0 && "invalid cam type");
  }
}

void
sgCamStep(SGcam *cam, float dt)
{
  assert(cam != NULL && "cam not set");
  switch (cam->kind) {
  case SGCam_Orbit:
    {
      SGorbitcam *ocam = (SGorbitcam*)cam;
      sgCamAxisUpdate(cam);

      ocam->ra += ocam->dra;
      ocam->ra = fmod(ocam->ra, 2.0*M_PI);
      ocam->dec += ocam->ddec;
      ocam->dec = fmod(ocam->dec, 2.0*M_PI);

      ocam->zoom += ocam->dr; if (ocam->zoom < 1.0) ocam->zoom = 1.0;
      ocam->r = ocam->zoom;
    }
    break;
  case SGCam_Fixed:
    assert(0 && "not supported yet");
    break;
  case SGCam_Free:
    {
      SGfreecam *freecam = (SGfreecam*)cam;
      sgCamAxisUpdate(cam);

      ooLwcTranslate3fv(&freecam->lwc, freecam->dp);
      freecam->q = q_mul(freecam->q, freecam->dq);
    }
    break;
  default:
    assert(0 && "invalid cam type");
  }
}

void
sgCamMove(SGcam *cam)
{
  assert(cam != NULL && "cam not set");
//    glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  switch (cam->kind) {
  case SGCam_Orbit:
    {
      SGorbitcam *ocam = (SGorbitcam*)cam;
      float p[3] = {ocam->r*cos(ocam->dec),
                    ocam->r*sin(ocam->dec),
                    ocam->r*sin(ocam->ra)};
      float3 cogOffset = mf3_v_mul(ocam->body->R, ocam->body->m.cog);
      glTranslatef(-(p[0] + cogOffset.x),
                   -(p[1] + cogOffset.y),
                   -(p[2] + cogOffset.z));
    }
    break;
  case SGCam_Fixed:
    {
      SGfixedcam* fix = (SGfixedcam*)cam;

      float3 p = fix->body->p.offs;
      p = vf3_add(p, fix->r);
      quaternion_t q = fix->body->q;
      q = q_mul(q, fix->q);
      matrix_t m;
      q_m_convert(&m, q);
      //      matrix_t mt;
      //      m_transpose(&mt, &m);

      glMultMatrixf((GLfloat*)&m);
      glTranslatef(-vf3_x(p), -vf3_y(p), -vf3_z(p));
    }
    break;
  case SGCam_Free:
    {
//      SGfreecam *freecam = (SGfreecam*)cam;

//      glTranslatef(-vf3_x(freecam->lwc.offs),
//                   -vf3_y(freecam->lwc.offs),
//                   -vf3_z(freecam->lwc.offs));
    }
    break;
  default:
      assert(0 && "illegal case statement");
  }
}


#include "sim.h"
extern SIMstate gSIM_state;

// Axis checker
void
sgCamAxisUpdate(SGcam *cam)
{
  if (cam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)cam;
    // Nice thing is that these return 0.0 if they are not assigned
    float yaw = ooIoGetAxis(NULL, "yaw");
    float pitch = ooIoGetAxis(NULL, "pitch");
    float roll = ooIoGetAxis(NULL, "roll");
    float horizontal = ooIoGetAxis(NULL, "horizontal");
    float vertical = ooIoGetAxis(NULL, "vertical");
    float thrust = ooIoGetAxis(NULL, "thrust");

    float3 v = vf3_set(1.0 * horizontal,
                       1.0 * vertical,
                       1.0 * thrust);
    fcam->dp = v_q_rot(v, fcam->q);

    fcam->dq = q_rot(0.0f,0.0f,1.0f, -0.02f * roll);
    fcam->dq = q_mul(fcam->dq, q_rot(1.0f,0.0f,0.0f, 0.02f * pitch));
    fcam->dq = q_mul(fcam->dq, q_rot(0.0f,1.0f,0.0f, -0.02f * yaw));
  } else if (cam->kind == SGCam_Orbit) {
    float yaw = ooIoGetAxis(NULL, "camyaw");
    float pitch = ooIoGetAxis(NULL, "campitch");
    float zoom = ooIoGetAxis(NULL, "camzoom");
    SGorbitcam *ocam = (SGorbitcam*)cam;
    ocam->dra = pitch * 0.02;
    ocam->ddec = yaw * 0.02;
    ocam->dr = zoom * 2.0; // TODO: exponential
  }
}


/* Camera actions, registered as action handlers */
void
sgCamFwd(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 0.0, -10000.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}

void
sgCamBack(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 0.0, 10000.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
sgCamLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(-10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
sgCamRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
sgCamUp(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
sgCamDown(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, -10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
sgCamRollLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.01f);
    }
  }
}
void
sgCamRollRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, -0.01f);
    }
  }
}

void
sgCamYawLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.01f);
    }
  }
}

void
sgCamYawRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, -0.01f);
    }
  }
}

void
sgCamPitchDown(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, -0.01f);
    }
  }
}


void
sgCamPitchUp(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == SGCam_Free) {
    SGfreecam *fcam = (SGfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.01f);
    }
  }
}
