/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */
#include <stdbool.h>
#include <stdlib.h>

#include "SDL_opengl.h"
#include <vmath/vmath.h>
#include "log.h"
#include "io-manager.h"
#include "camera.h"
#include "settings.h"
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
    if (key) ooIoBindKeyHandler(key, keyBindings[i].ioKey, 0, 0);
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
  ((OOfreecam*)cam->camData)->p = v_set(x,y,z,1.0f);
  ((OOfreecam*)cam->camData)->q = q_rot(rx,ry,rz, 0.0f);
  
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

void
ooSgCamMove(OOcam *cam)
{
    assert(cam != NULL && "cam not set");
    glPushMatrix();
    
    switch (cam->kind) {
    case OOCam_Orbit:
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
            const dReal *pos = dBodyGetPosition(((OOfixedcam*)cam->camData)->body);
            const dReal *rot = dBodyGetRotation(((OOfixedcam*)cam->camData)->body);
            glTranslatef(((OOfixedcam*)cam->camData)->r.x + pos[0],
                         ((OOfixedcam*)cam->camData)->r.y + pos[1],
                         ((OOfixedcam*)cam->camData)->r.z + pos[2]);
            
        }
        break;
    case OOCam_Free:
        glTranslatef(((OOfreecam*)cam->camData)->p.x,
                     ((OOfreecam*)cam->camData)->p.y,
                     ((OOfreecam*)cam->camData)->p.z);
        break;
    default:
        assert(0 && "illegal case statement");
    }
}




#include "sim.h"
extern SIMstate gSIM_state;
/* Camera actions, registered as action handlers */
void
ooSgCamFwd(bool up, void *data)
{
}

void
ooSgCamBack(bool up, void *data)
{
}
void
ooSgCamLeft(bool up, void *data)
{
}
void
ooSgCamRight(bool up, void *data)
{
}
void
ooSgCamUp(bool up, void *data)
{
}
void
ooSgCamDown(bool up, void *data)
{
}
void
ooSgCamRollLeft(bool up, void *data)
{
}
void
ooSgCamRollRight(bool up, void *data)
{
}

void
ooSgCamYawLeft(bool up, void *data)
{
}

void
ooSgCamYawRight(bool up, void *data)
{
}

void
ooSgCamPitchDown(bool up, void *data)
{
}


void
ooSgCamPitchUp(bool up, void *data)
{
}
