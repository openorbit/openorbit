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

#include "text.h"
#include "texture.h"
#include "res-manager.h"
//#include "SDL_ttf.h"
#include "common/moduleinit.h"
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct SGtextcontext {
#ifdef __APPLE__
  CGContextRef cgCtxt;
  void *data;
#endif
} SGtextcontext;

static FT_Library library;

SGtextcontext*
sgCreateTextContext(size_t w, size_t h)
{
#ifdef __APPLE__
  SGtextcontext *ctxt = malloc(sizeof(SGtextcontext));
  ctxt->data = malloc(w*h*4);

  ctxt->cgCtxt = CGBitmapContextCreate(ctxt->data, w, h, 8, w*4,
                                       CGColorSpaceCreateDeviceRGB(),
                                       kCGImageAlphaLast);
  CGContextSetRGBFillColor(ctxt->cgCtxt, 0.0, 0.0, 0.0, 0.0);
  return ctxt;
#endif
}

INIT_PRIMARY_MODULE
{
  int error = FT_Init_FreeType( &library );
  if ( error ) {
    fprintf(stderr, "freetype failed init\n");
    exit(1);
  }  
//  if (!TTF_WasInit()) {
//    if(TTF_Init()==-1) {
//      fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
//      exit(2);
//    }
//  }
}

SGfont*
ooLoadFont(const char *fontName, int sz)
{
  FT_Face face;
  char *fontFile = ooResGetPath(fontName);

  int error = FT_New_Face(library, fontFile, 0, &face);
  if (error == FT_Err_Unknown_File_Format) {
    fprintf(stderr, "'%s' is of unknown format\n", fontFile);
    free(fontFile);
    free(face);
    return NULL;
  } else if (error) {
    fprintf(stderr, "'%s' could not be loaded\n", fontFile);
    free(fontFile);
    free(face);
    return NULL;
  }
  
  error = FT_Set_Char_Size(face, 0, sz*64, 72, 72);
  // FT_Set_Pixel_Size...
  free(fontFile);
  
  SGfont *font = malloc(sizeof(SGfont));
  font->face = face;
  font->size = sz;
  
  return font;
}

void
ooUnloadFont(SGfont *font)
{
  free(font->face);
  free(font);
}

#if 0




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

#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void
copy_character(SGtextbitmap  *image,
               int x,
               int y,
               const FT_Bitmap *bitmap)
{
  assert(bitmap->pixel_mode == FT_PIXEL_MODE_GRAY);

  int width = MIN(image->w - x, bitmap->width);
  int height = MIN(image->h - (y - bitmap->rows), bitmap->rows);

  // Foreach row, copy pixels
  for (int i = 0 ; i < height ; i++) {
    if (y-i < 0) continue;

    memcpy(&image->data[(y-i)*image->w + x],
           &bitmap->buffer[i*bitmap->width],
           width);
  }
}


void
ooPrint(SGfont *font, SGtextbitmap *image, const char *text)
{
  FT_GlyphSlot slot = font->face->glyph;
  unsigned max_x = 0, max_y = 0;
  unsigned pen_x = 0, pen_y = image->h - font->size;
  for (; *text ; text ++) {
    if (*text == '\n') {
      pen_y -= font->size;
      pen_x = 0;
      max_y = MAX(max_y, pen_y);
      continue;
    }
    
    int error = FT_Load_Char( font->face, *text, FT_LOAD_RENDER );
    if ( error )
      continue;  /* ignore errors */
    
    copy_character(image,
                   pen_x + slot->bitmap_left,
                   pen_y + slot->bitmap_top,
                   &slot->bitmap);
    
    // Increment pen position
    pen_x += slot->advance.x >> 6;
    pen_y += slot->advance.y >> 6;
    max_x = MAX(max_x, pen_x);
    max_y = MAX(max_y, pen_y);
  }
  image->stride = image->w;
  image->w = max_x;
}

