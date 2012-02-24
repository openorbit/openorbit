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

#include <assert.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include "text.h"
#include "texture.h"
#include "res-manager.h"
#include "common/moduleinit.h"

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library library;

INIT_PRIMARY_MODULE
{
  int error = FT_Init_FreeType( &library );
  if ( error ) {
    fprintf(stderr, "freetype failed init\n");
    exit(1);
  }  
}

SGfont*
sgLoadFont(const char *fontName, int sz)
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
sgUnloadFont(SGfont *font)
{
  free(font->face);
  free(font);
}

#if 0
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void
copy_character(SGtextbuffer  *image,
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
#endif

SGtextbuffer*
sgNewStaticTextBuffer(const char *fontName, unsigned fontSize,
                      unsigned width, unsigned rows)
{
  return sgNewTextBuffer(fontName, fontSize, width, rows);
}


SGtextbuffer*
sgNewTextBuffer(const char *fontName, unsigned fontSize, unsigned width, unsigned rows)
{
//  SGtextbuffer *buff = malloc(sizeof(SGtextbuffer));

//  buff->font = sgLoadFont(fontName, fontSize);
//  buff->h = rows * fontSize;
//  buff->w = width * fontSize;
//  buff->stride = width * fontSize;
//  buff->data = calloc(buff->w * buff->h, sizeof(char));
  SGtextbuffer *buff = text_bitmap_create(fontName, fontSize, width, rows);
  return buff;
}

SGtextbuffer*
sgNewTextLabel(const char *fontName, unsigned fontSize, const char *label)
{
  SGtextbuffer *buff = text_bitmap_create_label(fontName, fontSize, label);
  return buff;
}

SGtextbuffer* sgNewTextLabelf(const char *fontName, unsigned fontSize,
                              const char *label, ...)
{
  va_list vaList;
  va_start(vaList, label);

  char *str = NULL;

  vasprintf(&str, label, vaList);
  SGtextbuffer* buff = sgNewTextLabel(fontName, fontSize, str);
  free(str);

  va_end(vaList);
  return buff;
}

void
sgDeleteTextBuffer(SGtextbuffer *buff)
{
  text_bitmap_dispose(buff, true);
//  sgUnloadFont(buff->font);
//  free(buff->data);
//  free(buff);
}

void
sgPrintBuffer(SGtextbuffer *buff, const char *text)
{
  text_bitmap_drawtext(buff, text);
#if 0
  FT_GlyphSlot slot = buff->font->face->glyph;
  unsigned max_x = 0, max_y = 0;
  unsigned pen_x = 0, pen_y = buff->h - buff->font->size;
  for (; *text ; text ++) {
    if (*text == '\n') {
      pen_y -= buff->font->size;
      pen_x = 0;
      max_y = MAX(max_y, pen_y);
      continue;
    }

    int error = FT_Load_Char( buff->font->face, *text, FT_LOAD_RENDER );
    if ( error )
      continue;  /* ignore errors */

    copy_character(buff,
                   pen_x + slot->bitmap_left,
                   pen_y + slot->bitmap_top,
                   &slot->bitmap);

    // Increment pen position
    pen_x += slot->advance.x >> 6;
    pen_y += slot->advance.y >> 6;
    max_x = MAX(max_x, pen_x);
    max_y = MAX(max_y, pen_y);
  }
  buff->stride = buff->w;
  buff->w = max_x;
#endif
}

void
sgPrintfBuffer(SGtextbuffer *buff, const char *fmt, ...)
{
  va_list vaList;
  va_start(vaList, fmt);

  char *str = NULL;

  vasprintf(&str, fmt, vaList);
  sgPrintBuffer(buff, str);
  free(str);

  va_end(vaList);
}

void
sgBindTextBuffer(SGtextbuffer *buff)
{
  GLuint texID = text_bitmap_texid(buff);
  glBindTexture(GL_TEXTURE_2D, texID);
}

GLuint
sgTextTexture(SGtextbuffer *buff)
{
  return text_bitmap_texid(buff);
}

