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
#include "SDL_ttf.h"

typedef struct OOfont {
  TTF_Font *font;
} OOfont;

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
OOfont* ooLoadFont(const char *fontName, int sz);
OOtexture* ooRenderText(OOfont *font, const char * restrict str);
void ooPrintfAtPos(OOfont *font, float x, float y, const char *fmt, ...);
void ooPrintfQuad(OOfont *font, OOprintquadrant quad, const char *fmt, ...);

#endif /* end of include guard: TEXT_H_CNEFSHUK */
