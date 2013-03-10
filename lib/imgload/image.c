/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit. Open Orbit is free software: you can
 redistribute it and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

 Some files of Open Orbit have relaxed licensing conditions. This file is
 licenced under the 2-clause BSD licence.

 Redistribution and use of this file in source and binary forms, with or
 without modification, are permitted provided that the following conditions are
 met:

 - Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <imgload/image.h>
#include <openorbit/log.h>

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
          log_trace("set img to gray8\n");
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
