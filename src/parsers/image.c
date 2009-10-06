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


#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "image.h"

#include "tga.h"
#include "jpg.h"
#include "pngfile.h"

int
img_load(image_t * restrict img, const char * restrict fileName)
{
  assert(img && "not null");
  assert(fileName && "not null");
  
  char *dot = strrchr(fileName, '.');
  if (dot) {
    char *endptr = strchr(dot, '\0');
    assert(endptr != NULL);
    
    if (!strcmp(dot, ".tga")) {
      tga_image_t tgaimg;
      FILE *fp = fopen(fileName, "rb");
      if (!fp) return -1;
      int res = tga_read_file(&tgaimg, fp);
      if (res == 0) {
        img->w = tgaimg.header.img_spec.width;
        img->h = tgaimg.header.img_spec.height;
        if (tgaimg.header.img_spec.depth == 32 &&
            tgaimg.header.img_spec.alpha_bits == 8) {
          img->kind = IMG_BGRA;
        } else if (tgaimg.header.img_spec.depth == 24 &&
                   tgaimg.header.img_spec.alpha_bits == 0) {
          img->kind = IMG_BGR;
        }
        img->data = tgaimg.data;
      } else {
        return -1;
      }
    } else if (!strcmp(dot, ".png")) {
      png_image_t pngimg;
      FILE *fp = fopen(fileName, "rb");
      if (!fp) return -1;
      int res = png_read_file(&pngimg, fp);
      if (res == 0) {
        switch (pngimg.kind) {
        case PNG_RGBA:
          img->kind = IMG_RGBA;
          break;
        case PNG_RGB:
          img->kind = IMG_RGB;
          break;
        default:
          assert(0 && "invalid case");
        }
        img->w = pngimg.w;
        img->h = pngimg.h;
        img->data = pngimg.data;
      } else {
        return -1;
      }
    } else if (!strcmp(dot, ".jpg")) {
      jpg_image_t jpgimg;
      int res = jpeg_load(&jpgimg, fileName);
      if (res == 0) {
        switch (jpgimg.kind) {
        case JPG_RGB:
          img->kind = IMG_RGB;
          break;
        case JPG_GRAY:
          img->kind = IMG_GRAY8;
          fprintf(stderr, "set img to gray8\n");
          break;
        default:
          assert(0 && "invalid case");
        }
        img->w = jpgimg.w;
        img->h = jpgimg.h;
        img->data = jpgimg.data;        
      } else {
        return -1;
      }
    }
  }
  return 0;
}
