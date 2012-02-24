/*
  Copyright 2009,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef TEXT_H_CNEFSHUK
#define TEXT_H_CNEFSHUK

#include <stdbool.h>
#include "texture.h"
//#include "SDL_ttf.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// Text bitmap API, must conform to these prototypes. The core graphics
// implementation is in platform/macosx/cgbitmaps.c.

typedef struct text_bitmap_t text_bitmap_t;
text_bitmap_t* text_bitmap_create(const char *font, float fontsize, unsigned w, unsigned h);
text_bitmap_t* text_bitmap_create_label(const char *fontName, float fontsize,
                                        const char *text);

void text_bitmap_drawtext(text_bitmap_t *bitmap, const char *text);
void text_bitmap_dispose(text_bitmap_t *bitmap, bool disposeGL);
unsigned text_bitmap_width(text_bitmap_t *bitmap);
unsigned text_bitmap_height(text_bitmap_t *bitmap);
char* text_bitmap_data(text_bitmap_t *bitmap);
GLuint text_bitmap_texid(text_bitmap_t *bitmap);

typedef struct {
  FT_Face face;
  int size;
} SGfont;

//typedef struct {
//  SGfont *font;
//  unsigned w, h, stride;
//  unsigned char *data;
//} SGtextbuffer;
typedef void SGtextbuffer;

SGfont* sgLoadFont(const char *fontName, int sz);
void sgUnloadFont(SGfont *font);

SGtextbuffer* sgNewStaticTextBuffer(const char *fontName, unsigned fontSize,
                                    unsigned width, unsigned rows);

SGtextbuffer* sgNewTextBuffer(const char *fontName, unsigned fontSize,
                              unsigned width, unsigned rows);

SGtextbuffer* sgNewTextLabel(const char *fontName, unsigned fontSize,
                             const char *label);
SGtextbuffer* sgNewTextLabelf(const char *fontName, unsigned fontSize,
                              const char *label, ...)
  __attribute__ ((format (printf, 3, 4)));

void sgDeleteTextBuffer(SGtextbuffer *buff);

void sgPrintBuffer(SGtextbuffer *buff, const char *text);
void sgPrintfBuffer(SGtextbuffer *buff, const char *fmt, ...)
  __attribute__ ((format (printf, 2, 3)));

void sgBindTextBuffer(SGtextbuffer *buff);
GLuint sgTextTexture(SGtextbuffer *buff);

#endif /* end of include guard: TEXT_H_CNEFSHUK */
