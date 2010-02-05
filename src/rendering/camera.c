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
ooSgNewFixedCam(OOscenegraph *sg, OOscene *sc, dBodyID body,
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
ooSgNewOrbitCam(OOscenegraph *sg, OOscene *sc, float dx, float dy, float dz)
{
  OOorbitcam *cam = malloc(sizeof(OOorbitcam));
  cam->super.kind = OOCam_Orbit;
  cam->super.scene = sc;

  ooLwcSet(&cam->lwc, 0.0, 0.0, 0.0);
  cam->r = vf3_set(dx,dy,dz);

  obj_array_push(&sg->cams, cam);
  return (OOcam*)cam;
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
    assert(0 && "not supported yet");
    {
      //      const dReal *pos = dBodyGetPosition(((OOorbitcam*)cam->camData)->body);
      //const dReal *rot = dBodyGetRotation(((OOorbitcam*)cam->camData)->body);
      //gluLookAt(vf3_x(((OOorbitcam*)cam->camData)->r) + pos[0],
      //          vf3_y(((OOorbitcam*)cam->camData)->r) + pos[1],
      //          vf3_z(((OOorbitcam*)cam->camData)->r) + pos[2],
      //          pos[0], pos[1], pos[2], // center
      //            0.0, 1.0, 0.0); // up
    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = (OOfixedcam*)cam;
      const dReal *quat = dBodyGetQuaternion(fix->body);
      quaternion_t q = vf4_set(quat[1], quat[2], quat[3], quat[0]);
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
    assert(0 && "not supported yet");
    break;
  case OOCam_Fixed:
    assert(0 && "not supported yet");
    break;
  case OOCam_Free:
    {
      OOfreecam *freecam = (OOfreecam*)cam;
      ooSgCamAxisUpdate(cam);

      ooLwcTranslate(&freecam->lwc, freecam->dp);
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

    }
    break;
  case OOCam_Fixed:
    {
      OOfixedcam* fix = (OOfixedcam*)cam;
      const dReal *pos = dBodyGetPosition(fix->body);
      const dReal *quat = dBodyGetQuaternion(fix->body);
      float3 p = vf3_set(pos[0], pos[1], pos[2]);
      p = vf3_add(p, fix->r);
      quaternion_t q = vf4_set(quat[1], quat[2], quat[3], quat[0]);
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
  }
}

void
sgCamSetLwc(OOcam *cam, OOlwcoord *lwc)
{
  if (cam->kind == OOCam_Orbit) {
    OOorbitcam *ocam = (OOorbitcam*)cam;
    ocam->lwc = *lwc;
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
