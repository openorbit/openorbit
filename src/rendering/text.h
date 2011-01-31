/*
  Copyright 2009,2011 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef TEXT_H_CNEFSHUK
#define TEXT_H_CNEFSHUK

#include "texture.h"
//#include "SDL_ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  FT_Face face;
  int size;
} SGfont;

typedef struct {
  SGfont *font;
  unsigned w, h, stride;
  unsigned char *data;
} SGtextbuffer;

SGfont* sgLoadFont(const char *fontName, int sz);
void sgUnloadFont(SGfont *font);

SGtextbuffer* sgNewTextBuffer(const char *fontName, unsigned fontSize,
                              unsigned width, unsigned rows);
void sgDeleteTextBuffer(SGtextbuffer *buff);

void sgPrintBuffer(SGtextbuffer *buff, const char *text);
void sgPrintfBuffer(SGtextbuffer *buff, const char *fmt, ...);

#endif /* end of include guard: TEXT_H_CNEFSHUK */
