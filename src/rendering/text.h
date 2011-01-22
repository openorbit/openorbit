/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

typedef FT_Face SGfont;

typedef struct {
  unsigned w, h, stride;
  unsigned char *data;
} SGtextbitmap;

typedef struct OOsgtextnode {
  OOtexture *text;
} OOsgtextnode;

typedef enum OOprintquadrant {
  OO_Up_Left,
  OO_Up_Right,
  OO_Down_Left,
  OO_Down_Right
} OOprintquadrant;
void ooTextInit(void);
SGfont ooLoadFont(const char *fontName, int sz);
OOtexture* ooRenderText(SGfont font, const char * restrict str);
void ooPrintfAtPos(SGfont font, float x, float y, const char *fmt, ...);
void ooPrintfQuad(SGfont font, OOprintquadrant quad, const char *fmt, ...);

void ooPrint(SGfont font, SGtextbitmap *image, const char *text);


#endif /* end of include guard: TEXT_H_CNEFSHUK */
