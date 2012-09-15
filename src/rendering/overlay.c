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

#include <assert.h>
#include <stdio.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include "rendering/overlay.h"
#include "rendering/types.h"
#include "rendering/shader-manager.h"
#include "rendering/scenegraph.h"

struct sg_overlay_t {
  bool enabled;
  float x, y;
  float w, h;
  GLuint fbo;

  GLuint vba;
  GLuint vbo;

  GLuint tex;

  float4 clear_color;

  void *obj;
  sg_draw_overlay_t draw;
  sg_shader_t *shader;
};

sg_overlay_t*
sg_new_overlay(void)
{
  sg_overlay_t *overlay = malloc(sizeof(sg_overlay_t));
  memset(overlay, 0, sizeof(sg_overlay_t));
  return overlay;
}

void
sg_overlay_init(sg_overlay_t *overlay, sg_draw_overlay_t drawfunc, void *obj,
                float x, float y, float w, float h, unsigned rw, unsigned rh)
{
  overlay->enabled = true;
  overlay->draw = drawfunc;
  overlay->obj = obj;
  overlay->x = x;
  overlay->y = y;
  overlay->w = w;
  overlay->h = h;
  glGenFramebuffers(1, &overlay->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, overlay->fbo);
  glGenTextures(1, &overlay->tex);
  glBindTexture(GL_TEXTURE_2D, overlay->tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, rw, rh, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  SG_CHECK_ERROR;

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         overlay->tex, 0);
  SG_CHECK_ERROR;

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  assert(status == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // TODO: Create vbo and vba
}

void
sg_overlay_set_clear_color(sg_overlay_t *overlay, float4 col)
{
  overlay->clear_color = col;
}

void
sg_overlay_draw(sg_overlay_t *overlay)
{
  // First render the texture that we place in the overlay
  sg_shader_bind(overlay->shader);
  glBindFramebuffer(GL_FRAMEBUFFER, overlay->fbo);
  glClearColor(overlay->clear_color[0], overlay->clear_color[1],
               overlay->clear_color[2], overlay->clear_color[3]);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  overlay->draw(overlay, overlay->obj);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Render the overlay using the given texture
  glBindBuffer(GL_ARRAY_BUFFER, overlay->vbo);
  glBindVertexArray(overlay->vba);

  //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void
sg_overlay_set_shader(sg_overlay_t *overlay, sg_shader_t *shader)
{
  overlay->shader = shader;
}

void*
sg_overlay_get_object(const sg_overlay_t *overlay)
{
  return overlay->obj;
}

