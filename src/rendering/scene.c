/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <stdbool.h>
#include <stdio.h>
#include "rendering/scene.h"
#include "rendering/camera.h"
#include "rendering/object.h"
#include "palloc.h"

struct sg_scene_t {
  char *name;
  sg_camera_t *cam;
  sg_background_t *bg;
  obj_array_t objects;
  obj_array_t lights;
  float4 amb; // Ambient light for the scene
  obj_array_t shaders;
};

void
sg_scene_set_bg(sg_scene_t *sc, sg_background_t *bg)
{
  sc->bg = bg;
  sg_background_set_scene(bg, sc);
}

void
sg_scene_set_cam(sg_scene_t *sc, sg_camera_t *cam)
{
  sc->cam = cam;
}

sg_camera_t*
sg_scene_get_cam(sg_scene_t *sc)
{
  return sc->cam;
}

void
sg_scene_add_object(sg_scene_t *sc, sg_object_t *obj)
{
  obj_array_push(&sc->objects, obj);
  sg_object_set_scene(obj, sc);
}

void
sg_scene_add_light(sg_scene_t *sc, sg_light_t *light)
{
  obj_array_push(&sc->lights, light);
}

void
sg_scene_set_amb4f(sg_scene_t *sc, float r, float g, float b, float a)
{
  sc->amb = vf4_set(r, g, b, a);
}

void
sg_scene_draw(sg_scene_t *scene, float dt)
{
  SG_CHECK_ERROR;
  sg_camera_animate(scene->cam, dt);
  sg_background_draw(scene->bg);
  //sgMoveCam(scene->cam);
  ARRAY_FOR_EACH(i, scene->objects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(scene->objects, i));
    sg_object_animate(ARRAY_ELEM(scene->objects, i), dt);
    sg_object_draw(ARRAY_ELEM(scene->objects, i));
  }
  SG_CHECK_ERROR;
}

sg_scene_t*
sg_new_scene(const char *name)
{
  sg_scene_t *scene = smalloc(sizeof(sg_scene_t));
  memset(scene, 0, sizeof(sg_scene_t));
  scene->name = strdup(name);
  obj_array_init(&scene->objects);
  obj_array_init(&scene->lights);
  obj_array_init(&scene->shaders);
  return scene;
}

bool
sg_scene_has_name(sg_scene_t *sc, const char *name)
{
  if (!strcmp(sc->name, name)) {
    return true;
  }
  return false;
}

//typedef struct {
//  sg_scenegraph_t *sg;
//  sg_camera_t *cam;
//  sg_background_t *bg;
//  obj_array_t objects;
//  obj_array_t lights;
//  obj_array_t shaders;
//} sg_scene_t;

