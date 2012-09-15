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

#include "rendering/window.h"
#include "rendering/viewport.h"
#include "rendering/scene.h"
#include "rendering/scenegraph.h"

struct sg_window_t {
  sg_scenegraph_t *sg;
  obj_array_t viewports;
  unsigned w, h;
};

void
sg_window_draw(sg_window_t *window, float dt)
{
  ARRAY_FOR_EACH(i, window->viewports) {
    sg_viewport_draw(ARRAY_ELEM(window->viewports, i), dt);
  }
}



sg_window_t*
sg_new_window(sg_scenegraph_t *sg)
{
  sg_window_t *window = malloc(sizeof(sg_window_t));
  memset(window, 0, sizeof(sg_window_t));
  obj_array_init(&window->viewports);

  window->sg = sg;
  sg_scenegraph_add_window(sg, window);

  return window;
}
