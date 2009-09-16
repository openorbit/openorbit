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

#include "SDL_opengl.h"
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
  OOcam *cam = malloc(sizeof(OOcam));
  cam->camData = malloc(sizeof(OOfreecam));
  cam->kind = OOCam_Free;

  cam->scene = sc;
//  ((OOfreecam*)cam->camData)->p = v_set(x,y,z,1.0f);
  ((OOfreecam*)cam->camData)->q = q_rot(rx,ry,rz, 0.0f);

  ((OOfreecam*)cam->camData)->dp = v_set(0.0,0.0,0.0,1.0f);
  ((OOfreecam*)cam->camData)->dq = q_rot(rx,ry,rz, 0.0f);

  ooLwcSet(&((OOfreecam*)cam->camData)->lwc, x, y, z);

  ooObjVecPush(&sg->cams, cam);
  return cam;
}

OOcam*
ooSgNewFixedCam(OOscenegraph *sg, OOscene *sc, dBodyID body,
                float dx, float dy, float dz, float rx, float ry, float rz)
{
    OOcam *cam = malloc(sizeof(OOcam));
    cam->camData = malloc(sizeof(OOfixedcam));
    cam->kind = OOCam_Fixed;

    cam->scene = sc;
    ((OOfixedcam*)cam->camData)->body = body;
    ((OOfixedcam*)cam->camData)->r = v_set(dx,dy,dz,1.0f);
    ((OOfixedcam*)cam->camData)->q = q_rot(rx,ry,rz, 0.0f);

    ooObjVecPush(&sg->cams, cam);
    return cam;
}

OOcam*
ooSgNewOrbitCam(OOscenegraph *sg, OOscene *sc, dBodyID body, float dx, float dy, float dz)
{
    OOcam *cam = malloc(sizeof(OOcam));
    cam->camData = malloc(sizeof(OOorbitcam));
    cam->kind = OOCam_Orbit;

    cam->scene = sc;
    ((OOorbitcam*)cam->camData)->body = body;
    ((OOorbitcam*)cam->camData)->r = v_set(dx,dy,dz,1.0f);

    ooObjVecPush(&sg->cams, cam);
    return cam;
}

// Only rotate, used for things like sky painting that requires camera rotation but not
// translation
void
ooSgCamRotate(OOcam *cam)
{
  assert(cam != NULL && "cam not set");
  switch (cam->kind) {
  case OOCam_Orbit:
    assert(0 && "not supported yet");
    {
      const dReal *pos = dBodyGetPosition(((OOorbitcam*)cam->camData)->body);
      const dReal *rot = dBodyGetRotation(((OOorbitcam*)cam->camData)->body);
      gluLookAt(  ((OOorbitcam*)cam->camData)->r.x + pos[0],
                ((OOorbitcam*)cam->camData)->r.y + pos[1],
                ((OOorbitcam*)cam->camData)->r.x + pos[2],
                pos[0], pos[1], pos[2], // center
                  0.0, 1.0, 0.0); // up
    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = cam->camData;
      const dReal *quat = dBodyGetQuaternion(fix->body);
      quaternion_t q = v_set(quat[1], quat[2], quat[3], quat[0]);
      q = q_mul(q, fix->q);
      matrix_t m;
      q_m_convert(&m, q);
      glMultMatrixf((GLfloat*)&m);
    }
    break;
  case OOCam_Free:
    {
      quaternion_t q = ((OOfreecam*)(cam->camData))->q;
      matrix_t m;
      q_m_convert(&m, q);
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
    assert(0 && "not supported yet");
    break;
  case OOCam_Fixed:
    assert(0 && "not supported yet");
    break;
  case OOCam_Free:
    {
      ooLwcTranslate(&((OOfreecam*)cam->camData)->lwc, ((OOfreecam*)cam->camData)->dp.v);
      ((OOfreecam*)cam->camData)->q = q_mul(((OOfreecam*)cam->camData)->q,
                                            ((OOfreecam*)cam->camData)->dq);
      ooSgCamAxisUpdate(cam);
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

  switch (cam->kind) {
  case OOCam_Orbit:
    {

    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = cam->camData;
      const dReal *pos = dBodyGetPosition(fix->body);
      const dReal *quat = dBodyGetQuaternion(fix->body);
      vector_t p = v_set(pos[0], pos[1], pos[2], 0.0);
      p = v_add(p, fix->r);
      quaternion_t q = v_set(quat[1], quat[2], quat[3], quat[0]);
      q = q_mul(q, fix->q);
      matrix_t m;
      q_m_convert(&m, q);
      glMultMatrixf((GLfloat*)&m);
      glTranslatef(-p.x, -p.y, -p.z);
    }
    break;
  case OOCam_Free:
    {
      quaternion_t q = ((OOfreecam*)(cam->camData))->q;
      matrix_t m;
      q_m_convert(&m, q);
      glMultMatrixf((GLfloat*)&m);
      glTranslatef(-vf3_get(((OOfreecam*)cam->camData)->lwc.offs, 0),
                   -vf3_get(((OOfreecam*)cam->camData)->lwc.offs, 1),
                   -vf3_get(((OOfreecam*)cam->camData)->lwc.offs, 2));
      //glTranslatef(-((OOfreecam*)cam->camData)->p.x,
      //             -((OOfreecam*)cam->camData)->p.y,
      //             -((OOfreecam*)cam->camData)->p.z);
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
    OOfreecam *fcam = cam->camData;
    // Nice thing is that these return 0.0 if they are not assigned
    float yaw = ooIoGetAxis("yaw");
    float pitch = ooIoGetAxis("pitch");
    float roll = ooIoGetAxis("roll");
    float horizontal = ooIoGetAxis("horizontal");
    float vertical = ooIoGetAxis("vertical");

    vector_t v = v_set(-10000.0 * vertical, 0.0, -10000.0 * horizontal, 0.0);
    fcam->dp = v_q_rot(v, fcam->q);

    fcam->dq = q_rot(0.0f,1.0f,0.0f, 0.01f * yaw);
    fcam->dq = q_mul(fcam->dq, q_rot(1.0f,0.0f,0.0f, 0.01f * pitch));
    fcam->dq = q_mul(fcam->dq, q_rot(0.0f,0.0f,1.0f, 0.01f * roll));
  }
}

/* Camera actions, registered as action handlers */
void
ooSgCamFwd(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(0.0, 0.0, -10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}

void
ooSgCamBack(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(0.0, 0.0, 10000.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(-10000.0, 0.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamRight(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(10000.0, 0.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamUp(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(0.0, 10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamDown(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dp = v_set(0.0, 0.0, 0.0, 0.0);
    } else {
      vector_t v = v_set(0.0, -10000.0, 0.0, 0.0);
      fcam->dp = v_q_rot(v, fcam->q);
    }
  }
}
void
ooSgCamRollLeft(bool up, void *data)
{
  if (gSIM_state.sg->currentCam->kind == OOCam_Free) {
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
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
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
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
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
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
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
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
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
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
    OOfreecam *fcam = gSIM_state.sg->currentCam->camData;
    if (up) {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.00f);
    } else {
      fcam->dq = q_rot(1.0f,0.0f,0.0f, 0.01f);
    }
  }
}
