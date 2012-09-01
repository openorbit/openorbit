/*
  Copyright 2008,2009,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SCENEGRAPH_H_
#define SCENEGRAPH_H_

#include <vmath/vmath.h>
#include <gencds/array.h>

#include "geo/geo.h"

//#include <openorbit/openorbit.h>
//#include "physics/orbit.h"
#include "sim/simtime.h"
#include <vmath/lwcoord.h>
#include "rendering/types.h"
#include "camera.h"
#include "particles.h"
#include "material.h"
#include "light.h"
#include "sky.h"

void sgSetSceneAmb4f(sg_scene_t *sc, float r, float g, float b, float a);

sg_scene_t* sgSceneGetParent(sg_scene_t *sc);

void sgSceneAddChild(sg_scene_t *parent, sg_scene_t *child);
void sgSceneAddObj(sg_scene_t *sc, sg_scene_t *object);

void sgSceneAddLight(sg_scene_t *sc, sg_light_t *light);

void sgAssertNoGLError(void);
void sgCheckGLError(const char *file, int line);
void sgClearGLError(void);

#define SG_CHECK_ERROR sgCheckGLError(__FILE__, __LINE__)

void sgInitOverlay(sg_overlay_t *overlay, sg_draw_overlay_t drawfunc,
                   float x, float y,
                   float w, float h,
                   unsigned rw, unsigned rh);



sg_scene_t* sg_new_scene(void);
void sg_scene_render(sg_scene_t *scene, float dt);
void sg_scene_set_background(sg_scene_t *sc, sg_background_t *bg);
void sg_window_render(sg_window_t *window, float dt);
sg_window_t* sg_new_window(void);

void sg_set_viewport(sg_viewport_t *viewport);
sg_viewport_t* sg_new_viewport(sg_window_t *window, unsigned x, unsigned y,
                             unsigned w, unsigned h);


#endif /* SCENEGRAPH_H_ */
