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

static GLenum sgLightNumberMap[] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3,
                                    GL_LIGHT4, GL_LIGHT5, GL_LIGHT6, GL_LIGHT7};
struct OOscene {
  struct OOscene *parent;
  char *name;

  float amb[4]; ///< Ambient light for this scene, initially [0.2,0.2,0.2,1.0]

  SGlight *lights[SG_MAX_LIGHTS];
  float3 t;

  OOscenegraph *sg;

  obj_array_t scenes; // Subscenes
  obj_array_t objs;   // Objects in this scene
};

struct OOoverlay {
//  OOtexture *tex;
  float x, y;
  float w, h;
};

struct OOscenegraph {
  OOscene *root;
  OOcam *currentCam;
  obj_array_t cams;

  int usedLights;

  SGdrawable *sky;
  obj_array_t overlays; // TODO: Consolidate OOobjvector and obj_array_t
};

typedef struct SGellipsis {
  SGdrawable super;
  double semiMajor;
  double semiMinor;
  double ecc;
  float colour[3];
  size_t vertCount;
  float verts[];
} SGellipsis;


typedef void (*SGenable_light_func)(SGlight *light, GLenum lightId);
typedef void (*SGdisable_light_func)(SGlight *light);

struct SGlight {
  struct OOscene *scene;
  int lightId;

  float pos[4];

  float ambient[4];
  float specular[4];
  float diffuse[4];

  SGenable_light_func enable;
  SGdisable_light_func disable;
};

struct SGspotlight {
  SGlight super;
  float dir[3];
};

struct SGpointlight {
  SGlight super;
};


#endif /* end of include guard: SCENEGRAPH_PRIVATE_H_8YHHHPCN */
