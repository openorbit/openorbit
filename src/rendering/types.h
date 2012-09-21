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

#ifndef orbit_types_h
#define orbit_types_h

#include <vmath/vmath.h>
#include <vmath/lwcoord.h>
#include <gencds/array.h>
#include "physics/reftypes.h"

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl3.h>
#endif

#define SG_OBJ_MAX_TEXTURES 4
#define SG_OBJ_MAX_LIGHTS 4

typedef enum {
  SG_CAMERA_FREE, // Free moving camera
  SG_CAMERA_FIXED, // Fixed at location relative to obj, local rotation allowed
  SG_CAMERA_ORBITING // Rotating around target object
} sg_camera_type_t;

typedef struct sg_render_info_t sg_render_info_t;
typedef struct sg_overlay_t sg_overlay_t;
typedef struct sg_camera_t sg_camera_t;
typedef struct sg_light_ids_t sg_light_ids_t;
typedef struct sg_material_ids_t sg_material_ids_t;
typedef struct sg_shader_t sg_shader_t;
typedef struct sg_star_t sg_star_t;
typedef struct sg_background_t sg_background_t;
typedef struct sg_scene_t sg_scene_t;
typedef struct sg_viewport_t sg_viewport_t;
typedef struct sg_window_t sg_window_t;
typedef struct sg_material_t sg_material_t;
typedef GLuint sg_texture_ref_t;
typedef struct sg_texture_t sg_texture_t;
typedef struct sg_light_t sg_light_t;
typedef struct sg_geometry_t sg_geometry_t;
typedef struct sg_object_t sg_object_t;

typedef void (*sg_draw_overlay_t)(sg_overlay_t *, void *obj);


#endif
