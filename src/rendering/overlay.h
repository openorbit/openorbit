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
#ifndef orbit_overlay_h
#define orbit_overlay_h

#include "rendering/types.h"

sg_overlay_t* sg_new_overlay(void);

void sg_overlay_init(sg_overlay_t *overlay, sg_draw_overlay_t drawfunc,
                     void *obj,
                     float x, float y,
                     float w, float h,
                     unsigned rw, unsigned rh);

void sg_overlay_draw(sg_overlay_t *overlay);
void* sg_overlay_get_object(const sg_overlay_t *overlay);

void sg_overlay_set_clear_color(sg_overlay_t *overlay, float4 col);
void sg_overlay_set_shader(sg_overlay_t *overlay, sg_shader_t *shader);

#endif
