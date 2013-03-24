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

#ifndef orbit_scene_h
#define orbit_scene_h

#include <stdbool.h>

#include "rendering/types.h"

sg_scene_t* sg_new_scene(const char *name);

void sg_scene_draw(sg_scene_t *scene, float dt);

void sg_scene_set_bg(sg_scene_t *sc, sg_background_t *bg);

void sg_scene_set_cam(sg_scene_t *sc, sg_camera_t *cam);
sg_camera_t* sg_scene_get_cam(sg_scene_t *sc);

void sg_scene_add_object(sg_scene_t *sc, sg_object_t *obj);
sg_object_t* sg_scene_get_object(sg_scene_t *sc, const char *name);

void sg_scene_add_light(sg_scene_t *sc, sg_light_t *light);
void sg_scene_set_amb4f(sg_scene_t *sc, float r, float g, float b, float a);
float4 sg_scene_get_amb(sg_scene_t *sc);

int sg_scene_get_num_lights(sg_scene_t *sc);
sg_light_t* sg_scene_get_light(sg_scene_t *sc, int i);


// Resynchronize all objects with physics system
//void sg_scene_update(sg_scene_t *scene);

bool sg_scene_has_name(sg_scene_t *sc, const char *name);

// Synchronise everything with physics system
void sg_scene_sync(sg_scene_t *scene);
void sg_scene_interpolate(sg_scene_t *scene);

const sg_object_t** sg_scene_get_objects(sg_scene_t *scene);
size_t sg_scene_get_object_count(sg_scene_t *scene);


#endif
