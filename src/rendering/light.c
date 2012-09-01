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

#include "rendering/types.h"

#include "light.h"
#include "scenegraph-private.h"
#include "camera.h"
#include "physics/object.h"
#include <stdlib.h>
void
sgSetLightPos3f(sg_light_t *light, float x, float y, float z)
{
  light->pos[0] = vf3_x(x);
  light->pos[1] = vf3_y(y);
  light->pos[2] = vf3_z(z);
  light->pos[3] = 1.0;
}

void
sgSetLightPosv(sg_light_t *light, float3 v)
{
  light->pos[0] = vf3_x(v);
  light->pos[1] = vf3_y(v);
  light->pos[2] = vf3_z(v);
  light->pos[3] = 1.0;
}

void
sgSetLightPosLW(sg_light_t *light, lwcoord_t *lwc)
{
  sg_scene_t *sc = light->scene;
  sg_camera_t *cam = sc->cam;

  if (cam->type == SG_CAMERA_FREE) {
    float3 relPos = lwc_relvec(lwc, cam->free.lwc.seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  } else if (cam->type == SG_CAMERA_ORBITING) {
    float3 relPos = lwc_relvec(lwc, cam->orbiting.obj->rigidBody->p.seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  }
}


void
sgLightSetAmbient4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->ambient[0] = r;
  light->ambient[1] = g;
  light->ambient[2] = b;
  light->ambient[3] = a;

}
void
sgLightSetSpecular4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->specular[0] = r;
  light->specular[1] = g;
  light->specular[2] = b;
  light->specular[3] = a;

}

void
sgLightSetDiffuse4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->diffuse[0] = r;
  light->diffuse[1] = g;
  light->diffuse[2] = b;
  light->diffuse[3] = a;
}



sg_light_t*
sgNewPointlight3f(sg_scene_t *sc, float x, float y, float z)
{
  sg_light_t *light = malloc(sizeof(sg_light_t));
  light->scene = sc;

  light->pos[0] = x;
  light->pos[1] = y;
  light->pos[2] = z;
  light->pos[3] = 1.0;

  light->ambient[0] = 0.0;
  light->ambient[1] = 0.0;
  light->ambient[2] = 0.0;
  light->ambient[3] = 1.0;

  light->specular[0] = 1.0;
  light->specular[1] = 1.0;
  light->specular[2] = 1.0;
  light->specular[3] = 1.0;

  light->diffuse[0] = 1.0;
  light->diffuse[1] = 1.0;
  light->diffuse[2] = 1.0;
  light->diffuse[3] = 1.0;

  sgSceneAddLight(sc, light);

  return light;
}

sg_light_t*
sgNewPointlight(sg_scene_t *sc, float3 p)
{
  return sgNewPointlight3f(sc, vf3_x(p), vf3_y(p), vf3_z(p));
}
