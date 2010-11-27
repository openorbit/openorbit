/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SCENEGRAPH_PRIVATE_H_8YHHHPCN
#define SCENEGRAPH_PRIVATE_H_8YHHHPCN

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <gl/gl.h>
#endif
#include <gencds/array.h>
#include "scenegraph.h"

static GLenum sgLightNumberMap[] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
                                    GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
struct SGscene {
  char *name;

  float amb[4]; ///< Ambient light for this scene, initially [0.2,0.2,0.2,1.0]

  SGlight **lights;

  SGscenegraph *sg;

  obj_array_t objs;   // Objects in this scene
};

struct SGoctreescene {
  SGscene super;

};

struct SGscenegraph {
  SGcam *currentCam;
  obj_array_t cams;
  obj_array_t scenes;   // All scenes in the scene graph

  GLint maxLights;

  int usedLights;

  SGdrawable *sky;
  obj_array_t overlays; // TODO: Consolidate OOobjvector and obj_array_t
};



#endif /* end of include guard: SCENEGRAPH_PRIVATE_H_8YHHHPCN */
