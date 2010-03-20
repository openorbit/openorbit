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


void ooSgCamFwd(bool up, void *data);
void ooSgCamBack(bool up, void *data);
void ooSgCamLeft(bool up, void *data);
void ooSgCamRight(bool up, void *data);
void ooSgCamUp(bool up, void *data);
void ooSgCamDown(bool up, void *data);
void ooSgCamRollLeft(bool up, void *data);
void ooSgCamRollRight(bool up, void *data);
void ooSgCamYawLeft(bool up, void *data);
void ooSgCamYawRight(bool up, void *data);
void ooSgCamPitchDown(bool up, void *data);
void ooSgCamPitchUp(bool up, void *data);


struct str_action_triplet {
  const char *confKey;
  const char *ioKey;
  OObuttonhandlerfunc action;
};

void
ooSgCamInit(void)
{
  const char *key;
  static const struct str_action_triplet keyBindings[] = {
    {"openorbit/controls/keyboard/cam-fwd", "cam-fwd", ooSgCamFwd},
    {"openorbit/controls/keyboard/cam-back", "cam-back", ooSgCamBack},
    {"openorbit/controls/keyboard/cam-left", "cam-left", ooSgCamLeft},
    {"openorbit/controls/keyboard/cam-right", "cam-right", ooSgCamRight},
    {"openorbit/controls/keyboard/cam-up", "cam-up", ooSgCamUp},
    {"openorbit/controls/keyboard/cam-down", "cam-down", ooSgCamDown},
    {"openorbit/controls/keyboard/cam-roll-left", "cam-roll-left", ooSgCamRollLeft},
    {"openorbit/controls/keyboard/cam-roll-right", "cam-roll-right", ooSgCamRollRight},
    {"openorbit/controls/keyboard/cam-yaw-left", "cam-yaw-left", ooSgCamYawLeft},
    {"openorbit/controls/keyboard/cam-yaw-right", "cam-yaw-right", ooSgCamYawRight},
    {"openorbit/controls/keyboard/cam-pitch-down", "cam-pitch-down", ooSgCamPitchDown},
    {"openorbit/controls/keyboard/cam-pitch-up", "cam-pitch-up", ooSgCamPitchUp}
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

OOcam*
ooSgNewFreeCam(OOscenegraph *sg, OOscene *sc,
               float x, float y, float z, float rx, float ry, float rz)
{
  assert(sg != NULL);
  OOfreecam *cam = malloc(sizeof(OOfreecam));
  cam->super.kind = OOCam_Free;
  cam->super.scene = sc;

  cam->q = q_rot(rx,ry,rz, 0.0f);
  cam->dp = vf3_set(0.0,0.0,0.0);
  cam->dq = q_rot(rx,ry,rz, 0.0f);

  ooLwcSet(&cam->lwc, x, y, z);

  obj_array_push(&sg->cams, cam);
  return (OOcam*)cam;
}

OOcam*
ooSgNewFixedCam(OOscenegraph *sg, OOscene *sc, PLobject *body,
                float dx, float dy, float dz, float rx, float ry, float rz)
{
    OOfixedcam *cam = malloc(sizeof(OOfixedcam));
    cam->super.kind = OOCam_Fixed;

    cam->super.scene = sc;
    cam->body = body;
    cam->r = vf3_set(dx,dy,dz);
    cam->q = q_rot(rx,ry,rz, 0.0f);

    obj_array_push(&sg->cams, cam);
    return (OOcam*)cam;
}

OOcam*
ooSgNewOrbitCam(OOscenegraph *sg, OOscene *sc, PLobject *body,
                float ra, float dec, float r)
{
  OOorbitcam *ocam = malloc(sizeof(OOorbitcam));
  ocam->super.kind = OOCam_Orbit;
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
  return (OOcam*)ocam;
}

void
sgSetCamTarget(OOcam *cam, PLobject *body)
{
  assert(cam != NULL);
  assert(body != NULL);

  if (cam->kind == OOCam_Fixed) {
    OOfixedcam *fixCam = (OOfixedcam*)cam;
    fixCam->body = body;
  } else if (cam->kind == OOCam_Orbit) {
    OOorbitcam *orbCam = (OOorbitcam*)cam;
    orbCam->body = body;
  }
}


// Only rotate, used for things like sky painting that requires camera rotation but not
// translation
void
ooSgCamRotate(OOcam *cam)
{
  assert(cam != NULL && "cam not set");
  glMatrixMode(GL_MODELVIEW);
  switch (cam->kind) {
  case OOCam_Orbit:
    {
      OOorbitcam* ocam = (OOorbitcam*)cam;
      float3 cogOffset = mf3_v_mul(ocam->body->R, ocam->body->m.cog);
      gluLookAt(0.0, 0.0, 0.0,
                -ocam->r*cos(ocam->dec),
                -ocam->r*sin(ocam->dec),
                -ocam->r*sin(ocam->ra),
                0.0, 0.0, 1.0);
    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = (OOfixedcam*)cam;
      quaternion_t q = fix->body->q;
      q = q_mul(q, fix->q);
      matrix_t m;
      q_m_convert(&m, q);
      matrix_t mt;
      m_transpose(&mt, &m);
      glMultMatrixf((GLfloat*)&mt);
    }
    break;
  case OOCam_Free:
    {
      OOfreecam* freec = (OOfreecam*)cam;
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
ooSgCamStep(OOcam *cam, float dt)
{
  assert(cam != NULL && "cam not set");
  switch (cam->kind) {
  case OOCam_Orbit:
    {
      OOorbitcam *ocam = (OOorbitcam*)cam;
      ooSgCamAxisUpdate(cam);

      ocam->ra += ocam->dra;
      ocam->ra = fmod(ocam->ra, 2.0*M_PI);
      ocam->dec += ocam->ddec;
      ocam->dec = fmod(ocam->dec, 2.0*M_PI);

      ocam->zoom += ocam->dr; if (ocam->zoom < 0.0) ocam->zoom = 0.0;
      ocam->r = ocam->zoom;
    }
    break;
  case OOCam_Fixed:
    assert(0 && "not supported yet");
    break;
  case OOCam_Free:
    {
      OOfreecam *freecam = (OOfreecam*)cam;
      ooSgCamAxisUpdate(cam);

      ooLwcTranslate3fv(&freecam->lwc, freecam->dp);
      freecam->q = q_mul(freecam->q, freecam->dq);
    }
    break;
  default:
    assert(0 && "invalid cam type");
  }
}

void
ooSgCamMove(OOcam *cam)
{
  assert(cam != NULL && "cam not set");
//    glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  switch (cam->kind) {
  case OOCam_Orbit:
    {
      OOorbitcam *ocam = (OOorbitcam*)cam;
      float p[3] = {ocam->r*cos(ocam->dec),
                    ocam->r*sin(ocam->dec),
                    ocam->r*sin(ocam->ra)};
      float3 cogOffset = mf3_v_mul(ocam->body->R, ocam->body->m.cog);
      glTranslatef(-(p[0]+ocam->body->p.offs.x + cogOffset.x),
                   -(p[1]+ocam->body->p.offs.y + cogOffset.y),
                   -(p[2]+ocam->body->p.offs.z + cogOffset.z));
    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = (OOfixedcam*)cam;

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
  case OOCam_Free:
    {
      OOfreecam *freecam = (OOfreecam*)cam;

      glTranslatef(-vf3_x(freecam->lwc.offs),
                   -vf3_y(freecam->lwc.offs),
                   -vf3_z(freecam->lwc.offs));
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
ooSgCamAxisUpdate(OOcam *cam)
{
  if (cam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)cam;
    // Nice thing is that these return 0.0 if they are not assigned
    float yaw = ooIoGetAxis("yaw");
    float pitch = ooIoGetAxis("pitch");
    float roll = ooIoGetAxis("roll");
    float horizontal = ooIoGetAxis("horizontal");
    float vertical = ooIoGetAxis("vertical");
    float thrust = ooIoGetAxis("thrust");

    float3 v = vf3_set(1.0 * horizontal,
                       1.0 * vertical,
                       1.0 * thrust);
    fcam->dp = v_q_rot(v, fcam->q);

    fcam->dq = q_rot(0.0f,0.0f,1.0f, -0.02f * roll);
    fcam->dq = q_mul(fcam->dq, q_rot(1.0f,0.0f,0.0f, 0.02f * pitch));
    fcam->dq = q_mul(fcam->dq, q_rot(0.0f,1.0f,0.0f, -0.02f * yaw));
  } else if (cam->kind == OOCam_Orbit) {
    float yaw = ooIoGetAxis("camyaw");
    float pitch = ooIoGetAxis("campitch");
    float zoom = ooIoGetAxis("camzoom");
    OOorbitcam *ocam = (OOorbitcam*)cam;
    ocam->dra = pitch * 0.02;
    ocam->ddec = yaw * 0.02;
    ocam->dr = zoom * 2.0; // TODO: exponential
  }
}


/* Camera actions, registered as action handlers */
void
ooSgCamFwd(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 0.0, -10000.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}

void
ooSgCamBack(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 0.0, 10000.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(-10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamUp(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, 10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamDown(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dp = vf3_set(0.0, 0.0, 0.0);
    } else {
      float3 v = vf3_set(0.0, -10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamRollLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.01f);
    }
  }
}
void
ooSgCamRollRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,0.0f,1.0f, -0.01f);
    }
  }
}

void
ooSgCamYawLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.01f);
    }
  }
}

void
ooSgCamYawRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(0.0f,1.0f,0.0f, -0.01f);
    }
  }
}

void
ooSgCamPitchDown(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, -0.01f);
    }
  }
}


void
ooSgCamPitchUp(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = (OOfreecam*)gSIM_state.sg->currentCam;
    if (up) {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.01f);
    }
  }
}
