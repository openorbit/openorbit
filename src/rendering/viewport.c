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



#include <stdio.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include <assert.h>
#include "rendering/viewport.h"
#include "rendering/scene.h"
#include "rendering/overlay.h"

struct sg_viewport_t {
  sg_scene_t *scene;
  obj_array_t overlays;
  unsigned x, y;
  unsigned w, h;
};

sg_viewport_t*
sg_new_viewport(sg_window_t *window, unsigned x, unsigned y,
                unsigned w, unsigned h)
{
  sg_viewport_t *viewport = malloc(sizeof(sg_viewport_t));
  memset(viewport, 0, sizeof(sg_viewport_t));
  obj_array_init(&viewport->overlays);
  viewport->x = x;
  viewport->y = y;
  viewport->w = w;
  viewport->h = h;
  return viewport;
}

void
sg_viewport_reshape(sg_viewport_t *viewport, unsigned x, unsigned y,
                    unsigned w, unsigned h)
{
  viewport->x = x;
  viewport->y = y;
  viewport->w = w;
  viewport->h = h;
}

void
sg_viewport_bind(sg_viewport_t *viewport)
{
  glViewport(viewport->x, viewport->y, viewport->w, viewport->h);
}

sg_camera_t*
sg_viewport_get_cam(sg_viewport_t *vp)
{
  assert(vp != NULL);

  return sg_scene_get_cam(vp->scene);
}

void
sg_viewport_add_overlay(sg_viewport_t *vp, sg_overlay_t *ov)
{
  obj_array_push(&vp->overlays, ov);
}
void
sg_viewport_set_scene(sg_viewport_t *vp, sg_scene_t *sc)
{
  vp->scene = sc;
}


void
sg_viewport_draw(sg_viewport_t *vp, float dt)
{
  sg_viewport_bind(vp);

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  sg_scene_draw(vp->scene, dt);

  ARRAY_FOR_EACH(i, vp->overlays) {
    sg_overlay_draw(ARRAY_ELEM(vp->overlays, i));
  }
}
