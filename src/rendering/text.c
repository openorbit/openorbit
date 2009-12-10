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

#if 0

#include "text.h"
#include "texture.h"
#include "res-manager.h"
#include "SDL_ttf.h"

void
ooTextInit(void)
{
  if (!TTF_WasInit()) {
    if(TTF_Init()==-1) {
      printf("TTF_Init: %s\n", TTF_GetError());
      exit(2);
    }
  }
}

OOfont*
ooLoadFont(const char *fontName, int sz)
{
  OOfont *fnt = malloc(sizeof(OOfont));
  char *path = ooResGetPath(fontName);
  
  fnt->font = TTF_OpenFont(path, sz);
  if(!fnt) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    // handle error
  }
  
  free(path);
  
  return fnt;
}
void ooUnloadFont(OOfont *fnt)
{
  TTF_CloseFont(fnt->font);
  free(fnt);
}

OOtexture*
ooRenderText(OOfont *font, const char * restrict str)
{
  SDL_Color fg = {0,255,0}, bg = {255, 255, 255}; // r, g, b  (uint8_t) 
  
  //SDL_Surface *surface = TTF_RenderUTF8_Solid(font->font, str, fg);
  //surface = TTF_RenderUTF8_Shaded(font->font, str, fg, bg);
  //surface = TTF_RenderUTF8_Blended(font->font, str, fg);
}

void
ooPrintfAtPos(OOfont *font, float x, float y, const char *fmt, ...)
{
  va_list vaList;
  va_start(vaList, fmt);
  char *str = NULL;
  int res = vasprintf(&str, fmt, vaList);

  free(str);
  va_end(vaList);
}

void
ooPrintfQuad(OOfont *font, OOprintquadrant quad, const char *fmt, ...)
{
  va_list vaList;
  va_start(vaList, fmt);
  char *str = NULL;
  int res = vasprintf(&str, fmt, vaList);

  free(str);
  va_end(vaList);
}


#endif