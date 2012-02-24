/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef RENDER_H
#define RENDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct {
  unsigned w;
  unsigned h;
  float aspect;
  float fovy;
  unsigned gl_major_vers;
  unsigned gl_minor_vers;
  unsigned glsl_major_vers;
  unsigned glsl_minor_vers;
} SGrenderinfo;

extern SGrenderinfo sgRenderInfo;

void ooSetPerspective(float fovy, int width, int height);
void ooSetVideoDefaults(void);
void ooResizeScreen(int x, int y, int width, int height, bool fullscreen);

/*! Predicate for glsl version checks */
bool sgGlslVersionIs(unsigned major, unsigned minor);
bool sgGlslVersionIsAtMost(unsigned major, unsigned minor);
bool sgGlslVersionIsAtLeast(unsigned major, unsigned minor);

#ifdef __cplusplus
}
#endif

#endif /* ! RENDER_H */
