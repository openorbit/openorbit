/*
 Copyright 2010,2011,2012 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef RENDERING_SHADER_MANAGER_H
#define RENDERING_SHADER_MANAGER_H

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL3/gl3.h>
#endif

#include <stdbool.h>

#include "rendering/object.h"
#include "rendering/location.h"
#include "rendering/texture.h"
#include "rendering/light.h"

void sg_load_all_shaders(void);

sg_shader_t* sg_load_shader(const char *key, const char *vspath, const char *fspath,
                            const char *gspath);
sg_shader_t* sg_get_shader(const char *key);
sg_shader_t* sg_get_shader_without_warnings(const char *key);

GLint sg_shader_get_location(sg_shader_t *program, sg_param_id_t param, bool required);

void sg_shader_bind(sg_shader_t *program);

void sg_shader_set_projection(sg_shader_t *shader, const float4x4 proj);
void sg_shader_set_model_view(sg_shader_t *shader, const float4x4 proj);

void sg_shader_bind_texture(sg_shader_t *shader, unsigned tex_num,
                            sg_texture_t *tex);

void sg_shader_bind_light(sg_shader_t *shader, unsigned light_num,
                          sg_light_t *light);

#endif /* !RENDERING_SHADER_MANAGER_H */
