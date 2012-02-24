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

#ifndef __CAMERA_H__
#define __CAMERA_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <vmath/vmath.h>
#include "physics/reftypes.h"
#include "rendering/reftypes.h"
#include "common/lwcoord.h"

  enum SGcamtype {
    SGCam_Free,
    SGCam_Fixed,
    SGCam_Orbit
  };

  struct SGcam {
    SGcamtype kind;
    SGscene *scene;
  };

  struct SGfreecam {
    SGcam super;
    OOlwcoord lwc;
    float3 dp;
    quaternion_t q;
    quaternion_t dq;
  };

  struct SGfixedcam {
    SGcam super;
    PLobject *body;
    float3 r; // With this offset
    quaternion_t q; // and this rotation (rotate before translation)
  };

  struct SGorbitcam {
    SGcam super;
    PLobject *body;

    float ra, dec;
    float dra, ddec, dr;
    float r, zoom;
  };


  void sgCamInit(void);

  SGcam* sgNewFreeCam(SGscenegraph *sg, SGscene *sc,
                        float x, float y, float z,
                        float rx, float ry, float rz);

  SGcam* sgNewFixedCam(SGscenegraph *sg, SGscene *sc, PLobject *body,
                         float dx, float dy, float dz,
                         float rx, float ry, float rz);

  SGcam* sgNewOrbitCam(SGscenegraph *sg, SGscene *sc, PLobject *body,
                         float ra, float dec, float r);

  void sgSetCamTarget(SGcam *cam, PLobject *body);

  void sgCamMove(SGcam *cam);
  void sgCamRotate(SGcam *cam);
  void sgCamStep(SGcam *cam, float dt);
  void sgCamAxisUpdate(SGcam *cam);

#ifdef __cplusplus
}
#endif

#endif
