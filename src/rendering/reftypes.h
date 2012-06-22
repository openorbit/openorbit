/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SG_REFTYPES_H
#define SG_REFTYPES_H

typedef struct SGdrawable SGdrawable;
typedef struct SGscene SGscene;
typedef struct SGoverlay SGoverlay;
typedef struct SGscenegraph SGscenegraph;
typedef struct SGmaterial SGmaterial;

typedef struct SGlight SGlight;
typedef struct SGspotlight SGspotlight;
typedef struct SGpointlight SGpointlight;

typedef struct SGsphere SGsphere;
typedef struct SGcylinder SGcylinder;

typedef struct SGparticles SGparticles;

typedef void (*SGdrawfunc)(SGdrawable*);



typedef enum SGcamtype SGcamtype;
typedef struct SGfreecam SGfreecam;
typedef struct SGfixedcam SGfixedcam;
typedef struct SGorbitcam SGorbitcam;
typedef struct SGcam SGcam;

typedef struct SGwindow SGwindow;
typedef struct SGviewport SGviewport;
typedef struct SGscenegraph2 SGscenegraph2;
typedef struct SGscene2 SGscene2;

typedef struct SGcamera SGcamera;
typedef struct SGshader SGshader;
typedef struct SGtexture SGtexture;
typedef struct SGgeometry SGgeometry;
typedef struct SGobject SGobject;
typedef struct SGbackground SGbackground;

typedef struct SGlightids SGlightids;
typedef struct SGmaterialids SGmaterialids;


#define SG_OBJ_MAX_LIGHTS 4
#define SG_OBJ_MAX_TEXTURES 4


#endif /* !SG_REFTYPES_H */
