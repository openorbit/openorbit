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

#include "rendering/light.h"
#include "rendering/scenegraph-private.h"
#include "rendering/camera.h"

#include "physics/object.h"
#include <stdlib.h>

struct sg_light_t {
  sg_scene_t *scene;
  int lightId;

  float4 pos;
  float4 ambient;
  float4 specular;
  float4 diffuse;
  float3 dir; // Only used for spotlights

  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  float4 globAmbient;
};


void
sg_light_set_pos3f(sg_light_t *light, float x, float y, float z)
{
  light->pos[0] = vf3_x(x);
  light->pos[1] = vf3_y(y);
  light->pos[2] = vf3_z(z);
  light->pos[3] = 1.0;
}

void
sg_light_set_posv(sg_light_t *light, float3 v)
{
  light->pos[0] = vf3_x(v);
  light->pos[1] = vf3_y(v);
  light->pos[2] = vf3_z(v);
  light->pos[3] = 1.0;
}

float3
sg_light_get_pos(const sg_light_t *light)
{
  return light->pos;
}

float4
sg_light_get_ambient(const sg_light_t *light)
{
  return light->ambient;
}

float4
sg_light_get_specular(const sg_light_t *light)
{
  return light->specular;
}

float4
sg_light_get_diffuse(const sg_light_t *light)
{
  return light->diffuse;
}

void
sg_light_set_poslw(sg_light_t *light, lwcoord_t *lwc)
{
  sg_scene_t *sc = light->scene;
  sg_camera_t *cam = sg_scene_get_cam(sc);

  if (sg_camera_get_type(cam) == SG_CAMERA_FREE) {
    float3 relPos = lwc_relvec(lwc, sg_camera_free_get_lwc(cam).seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  } else if (sg_camera_get_type(cam) == SG_CAMERA_ORBITING) {
    sg_object_t *obj = sg_camera_orbiting_get_obj(cam);
    PLobject *pobj = sg_object_get_rigid_body(obj);
    float3 relPos = lwc_relvec(lwc, pobj->p.seg);

    light->pos[0] = vf3_x(relPos);
    light->pos[1] = vf3_y(relPos);
    light->pos[2] = vf3_z(relPos);
    light->pos[3] = 1.0;
  }
}


void
sg_light_set_ambient4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->ambient[0] = r;
  light->ambient[1] = g;
  light->ambient[2] = b;
  light->ambient[3] = a;

}
void
sg_light_set_specular4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->specular[0] = r;
  light->specular[1] = g;
  light->specular[2] = b;
  light->specular[3] = a;

}

void
sg_light_set_diffuse4f(sg_light_t *light, float r, float g, float b, float a)
{
  light->diffuse[0] = r;
  light->diffuse[1] = g;
  light->diffuse[2] = b;
  light->diffuse[3] = a;
}

void
sg_light_set_attenuation(sg_light_t *light, float const_att, float lin_att,
                         float quad_att)
{
  light->constantAttenuation = const_att;
  light->linearAttenuation = lin_att;
  light->quadraticAttenuation = quad_att;
}



sg_light_t*
sg_new_light3f(sg_scene_t *sc, float x, float y, float z)
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

  sg_scene_add_light(sc, light);

  return light;
}

sg_light_t*
sg_new_light(sg_scene_t *sc, float3 p)
{
  return sg_new_light3f(sc, vf3_x(p), vf3_y(p), vf3_z(p));
}

float
sg_light_get_const_attenuation(const sg_light_t *light)
{
  return light->constantAttenuation;
}

float
sg_light_get_linear_attenuation(const sg_light_t *light)
{
  return light->linearAttenuation;
}


float
sg_light_get_quadratic_attenuation(const sg_light_t *light)
{
  return light->quadraticAttenuation;
}


