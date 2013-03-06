/*
  Copyright 2006,2009,2012,2013 Mattias Holm <lorrden(at)openorbit.org>

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

  sg_camera_t* sg_new_camera(sg_scene_t *scene);
  lwcoord_t sg_camera_pos(sg_camera_t *cam);
  quaternion_t sg_camera_quat(sg_camera_t *cam);
  const float4x4* sg_camera_project(sg_camera_t *cam);
  const float4x4* sg_camera_modelview(sg_camera_t *cam);

  void sg_camera_track_object(sg_camera_t *cam, sg_object_t *obj);
  void sg_camera_follow_object(sg_camera_t *cam, sg_object_t *obj);
  void sg_camera_set_perspective(sg_camera_t *cam, float perspective);
  sg_object_t* sg_camera_get_tracked_object(sg_camera_t *cam);

  void sg_camera_update_modelview(sg_camera_t *cam);
  void sg_camera_set_follow_offset(sg_camera_t *cam, float3 offs);

  // Interpolate camera position and rotation based on WCT
  void sg_camera_interpolate(sg_camera_t *cam, float t);
  void sg_camera_sync(sg_camera_t *cam);

#ifdef __cplusplus
}
#endif

#endif
