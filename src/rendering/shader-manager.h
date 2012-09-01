/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "rendering/gl3drawable.h"
#include "rendering/location.h"

void sgLoadAllShaders(void);

sg_shader_t* sgLoadProgram(const char *key, const char *vspath, const char *fspath,
                        const char *gspath);
sg_shader_t* sgGetProgram(const char *key);
void sgEnableProgram(GLuint programId);
void sgDisableProgram(void);
sg_shader_t* sgShaderFromKey(const char *key);



#endif /* !RENDERING_SHADER_MANAGER_H */
