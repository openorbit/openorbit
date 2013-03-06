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
#include "common/palloc.h"
#include "common/monotonic-time.h"
#include "log.h"
#include "settings.h"

struct sg_scene_t {
  char *name;
  sg_camera_t *cam;
  sg_background_t *bg;
  obj_array_t objects_sorted_by_name;
  obj_array_t objects;
  obj_array_t lights; // Scene global lights
  float4 amb; // Ambient light for the scene
  obj_array_t shaders;

  uint64_t sync_stamp;
  uint64_t next_sync_estimate;
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
  obj_array_push(&sc->objects_sorted_by_name, obj);
  sg_object_set_scene(obj, sc);

  qsort_b(sc->objects_sorted_by_name.elems, sc->objects_sorted_by_name.length,
          sizeof(sg_object_t*),
          ^int(const void *a, const void *b) {
            const sg_object_t **ap = (const sg_object_t **)a;
            const sg_object_t **bp = (const sg_object_t **)b;
            return strcmp(sg_object_get_name(*ap), sg_object_get_name(*bp));
          });
}

sg_object_t*
sg_scene_get_object(sg_scene_t *sc, const char *name)
{
  sg_object_t **obj =
    bsearch_b(name, sc->objects_sorted_by_name.elems,
              sc->objects_sorted_by_name.length,
              sizeof(sg_object_t*),

              ^int(const void *a, const void *b) {
                const sg_object_t **bp = (const sg_object_t **)b;
                return strcmp(a, sg_object_get_name(*bp));
              });

  return (obj) ? *obj : NULL;
}

void
sg_scene_add_light(sg_scene_t *sc, sg_light_t *light)
{
  obj_array_push(&sc->lights, light);
}

sg_light_t*
sg_scene_get_light(sg_scene_t *sc, int i)
{
  return ARRAY_ELEM(sc->lights, i);
}

int
sg_scene_get_num_lights(sg_scene_t *sc)
{
  return ARRAY_LEN(sc->lights);
}



void
sg_scene_set_amb4f(sg_scene_t *sc, float r, float g, float b, float a)
{
  sc->amb = vf4_set(r, g, b, a);
}

float4
sg_scene_get_amb(sg_scene_t *sc)
{
  return sc->amb;
}



// Synchronises all objects with physics sim
void
sg_scene_sync(sg_scene_t *scene)
{
  scene->sync_stamp = getmonotimestamp();

  // TODO: Instead of rereading the freq and period every step, we want to
  //       listen for some type of event that the config variable was modified.
  float freq;
  ooConfGetFloatDef("openorbit/sim/freq", &freq, 20.0); // Hz

  scene->next_sync_estimate = scene->sync_stamp + nstomonotime((1.0/freq)*1.0e9);

  float t = 0.0; // Period
  ooConfGetFloatDef("openorbit/sim/period", &t, 1.0/freq); // Seconds

  ARRAY_FOR_EACH(i, scene->objects) {
    sg_object_sync(ARRAY_ELEM(scene->objects, i), t);
  }

  sg_camera_sync(scene->cam);
}

void
sg_scene_interpolate(sg_scene_t *scene)
{
  uint64_t ts = getmonotimestamp();
  uint64_t ns = subtractmonotime(ts, scene->sync_stamp);
  uint64_t T = subtractmonotime(scene->next_sync_estimate, scene->sync_stamp);

  double normalised_time = (double)ns/(double)T;
  if (normalised_time > 1.0) normalised_time = 1.0;
  assert(normalised_time >= 0.0);

  ooLogTrace("intepolate to: %f", normalised_time);

  ARRAY_FOR_EACH(i, scene->objects) {
    sg_object_interpolate(ARRAY_ELEM(scene->objects, i), normalised_time);
  }

  sg_camera_interpolate(scene->cam, normalised_time);
}

void
sg_scene_distance_sort(sg_scene_t *sc)
{
  lwcoord_t cam_pos = sg_camera_pos(sc->cam);
  // This is not really optimal to do every frame, but works for now.
  qsort_b(sc->objects.elems, sc->objects.length,
          sizeof(sg_object_t*),
          ^int(const void *a, const void *b) {
            const sg_object_t **ap = (const sg_object_t **)a;
            const sg_object_t **bp = (const sg_object_t **)b;

            lwcoord_t pa = sg_object_get_p(*ap);
            lwcoord_t pb = sg_object_get_p(*bp);

            float3 va = lwc_dist(&pa, &cam_pos);
            float3 vb = lwc_dist(&pb, &cam_pos);

            int res = 0;
            if (vf3_abs_square(va) < vf3_abs_square(vb)) res = 1;
            else if (vf3_abs_square(va) > vf3_abs_square(vb)) res = -1;

            return res;
          });
}


void
sg_scene_draw(sg_scene_t *scene, float dt)
{
  SG_CHECK_ERROR;
  sg_scene_interpolate(scene);
  sg_camera_update_modelview(scene->cam);

  SG_CHECK_ERROR;
  sg_background_draw(scene->bg);
  SG_CHECK_ERROR;

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  SG_CHECK_ERROR;

  sg_scene_distance_sort(scene);

  ARRAY_FOR_EACH(i, scene->objects) {
    sg_object_recompute_modelviewmatrix(ARRAY_ELEM(scene->objects, i));
    SG_CHECK_ERROR;
    sg_object_draw(ARRAY_ELEM(scene->objects, i));
    SG_CHECK_ERROR;
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
  obj_array_init(&scene->objects_sorted_by_name);
  obj_array_init(&scene->lights);
  obj_array_init(&scene->shaders);

  // Default ambient light
  scene->amb = vf4_set(0.2, 0.2, 0.2, 1.0);

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

