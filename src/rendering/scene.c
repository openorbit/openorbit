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
#include "rendering/scene.h"
#include "rendering/camera.h"
#include "rendering/gl3drawable.h"

void
sg_scene_set_background(sg_scene_t *sc, sg_background_t *bg)
{
  sc->bg = bg;
}


void
sg_scene_render(sg_scene_t *scene, float dt)
{
  sgAnimateCam(scene->cam, dt);
  sgDrawBackground(scene->bg);
  //sgMoveCam(scene->cam);
  ARRAY_FOR_EACH(i, scene->objects) {
    sgRecomputeModelViewMatrix(ARRAY_ELEM(scene->objects, i));
    sgAnimateObject(ARRAY_ELEM(scene->objects, i), dt);
    sgDrawObject(ARRAY_ELEM(scene->objects, i));
  }
}

sg_scene_t*
sg_new_scene(void)
{
  sg_scene_t *scene = malloc(sizeof(sg_scene_t));
  memset(scene, 0, sizeof(sg_scene_t));
  return scene;
}
