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



#ifndef orbit_viewport_h
#define orbit_viewport_h

#include "rendering/types.h"

void sg_viewport_bind(sg_viewport_t *viewport);
sg_viewport_t* sg_new_viewport(sg_window_t *window, unsigned x, unsigned y,
                               unsigned w, unsigned h);

sg_camera_t* sg_viewport_get_cam(sg_viewport_t *vp);
void sg_viewport_add_overlay(sg_viewport_t *vp, sg_overlay_t *ov);
void sg_viewport_draw(sg_viewport_t *vp, float dt);

#endif
