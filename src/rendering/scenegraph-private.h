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

typedef struct SGlight {
  float3 pos;
  int lightId;
  float colour[4];
} SGlight;

typedef struct SGspotlight {
  SGlight super;
  float dir[3];
} SGspotlight;

typedef struct SGpointlight {
  SGlight super;
} SGpointlight;


struct OOscene {
  struct OOscene *parent;
  char *name;
  SGlight *lights[SG_MAX_LIGHTS];
  float3 t;
//  quaternion_t q;
//  scalar_t s; // scale with respect to parent s
//  scalar_t si; // inverse of s

  OOscenegraph *sg;

  OOobjvector scenes; // Subscenes
  OOobjvector objs;   // Objects in this scene
};

struct OOoverlay {
//  OOtexture *tex;
  float x, y;
  float w, h;
};

struct OOscenegraph {
  OOscene *root;
  OOcam *currentCam;
  OOobjvector cams;

  OOdrawable *sky;
  OOobjvector overlays;
};

typedef struct SGellipsis {
  OOdrawable super;
  double semiMajor;
  double semiMinor;
  double ecc;
  float colour[3];
  size_t vertCount;
  float verts[];
} SGellipsis;



#endif /* end of include guard: SCENEGRAPH_PRIVATE_H_8YHHHPCN */
