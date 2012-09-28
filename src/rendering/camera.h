/*
  Copyright 2006,2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef __CAMERA_H__
#define __CAMERA_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <vmath/vmath.h>
#include <vmath/lwcoord.h>
#include "rendering/types.h"

sg_camera_t* sg_new_free_camera(const lwcoord_t *pos);
sg_camera_t* sg_new_fixed_camera(sg_object_t *obj);
sg_camera_t* sg_new_orbiting_camera(sg_object_t *obj);

sg_camera_type_t sg_camera_get_type(sg_camera_t *cam);

void sg_camera_animate(sg_camera_t *cam, float dt);
void sg_camera_move(sg_camera_t *cam);

const float4x4* sg_camera_get_projection(sg_camera_t *cam);
const float4x4* sg_camera_get_view(sg_camera_t *cam);

lwcoord_t sg_camera_free_get_lwc(sg_camera_t *cam);
float3 sg_camera_free_get_velocity(sg_camera_t *cam);
void sg_camera_free_set_velocity(sg_camera_t *cam, float3 v);
void sg_camera_free_increase_velocity(sg_camera_t *cam, float3 dv);
quaternion_t sg_camera_free_get_quaternion(sg_camera_t *cam);
quaternion_t sg_camera_free_get_delta_quaternion(sg_camera_t *cam);

sg_object_t* sg_camera_fixed_get_obj(sg_camera_t *cam);
void sg_camera_fixed_set_obj(sg_camera_t *cam, sg_object_t *obj);
float3 sg_camera_fixed_get_offset(sg_camera_t *cam);
quaternion_t sg_camera_fixed_get_quaternion(sg_camera_t *cam);
quaternion_t sg_camera_fixed_get_delta_quaternion(sg_camera_t *cam);

sg_object_t* sg_camera_orbiting_get_obj(sg_camera_t *cam);
void sg_camera_orbiting_set_obj(sg_camera_t *cam, sg_object_t *obj);

float sg_camera_orbiting_get_ra(const sg_camera_t *cam) __attribute__((pure));
float sg_camera_orbiting_get_dec(const sg_camera_t *cam) __attribute__((pure));
float sg_camera_orbiting_get_delta_ra(const sg_camera_t *cam)
  __attribute__((pure));
float sg_camera_orbiting_get_delta_dec(const sg_camera_t *cam)
  __attribute__((pure));
float sg_camera_orbiting_get_delta_radius(const sg_camera_t *cam)
  __attribute__((pure));
float sg_camera_orbiting_get_radius(const sg_camera_t *cam)
  __attribute__((pure));
float sg_camera_orbiting_get_zoom(const sg_camera_t *cam)
  __attribute__((pure));

#ifdef __cplusplus
}
#endif

#endif
