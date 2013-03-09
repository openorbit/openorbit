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


#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <openorbit/log.h>
#include "jpg.h"

// TODO: handle color spaces

int
jpeg_load(jpg_image_t * restrict img, const char * restrict filename)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile;  /* source file */

  if ((infile = fopen(filename, "rb")) == NULL) {
    ooLogError("can't open %s\n", filename);
    return -1;
  }
  cinfo.err = jpeg_std_error(&jerr);

  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);
  img->w = cinfo.output_width;
  img->h = cinfo.output_height;
  if (cinfo.output_components == 3) {
    img->kind = JPG_RGB;
    ooLogTrace("loading colour jpg %s\n", filename);
  } else if (cinfo.output_components == 1) {
    img->kind = JPG_GRAY;
    ooLogTrace("loading grayscale jpg %s\n", filename);
  } else {
    assert(0 && "no support for non RGB or GRAYSCALE jpgs");
  }

  img->data = malloc(cinfo.output_width * cinfo.output_height *
                     cinfo.output_components);
  if (img->data == NULL) {
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return -1;
  }

  uint8_t *line;
  // Read lines in reverse to compensate for opengl coordinate system
  for (int i = cinfo.output_height - 1 ; i >= 0 ; --i) {
    line = img->data + i * cinfo.output_width * cinfo.output_components;
    jpeg_read_scanlines(&cinfo, &line, 1);
  }

  assert(cinfo.output_scanline == cinfo.output_height);

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  return 0;
}
