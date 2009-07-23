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


#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#include "jpg.h"

// TODO: handle color spaces

int
jpeg_load(jpg_image_t * restrict img, const char * restrict filename)
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * infile;  /* source file */
  JSAMPARRAY buffer;  /* Output row buffer */
  
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
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
    fprintf(stderr, "loading colour jpg %s\n", filename);
  } else if (cinfo.output_components == 1) {
    img->kind = JPG_GRAY;
    fprintf(stderr, "loading grayscale jpg %s\n", filename);
  } else {
    assert(0 && "no support for non RGB or GRAYSCALE jpgs");
  }
  
  img->data = malloc(cinfo.output_width * cinfo.output_height *
                     cinfo.output_components);
  uint8_t *line;
  for (int i = 0 ; i < cinfo.output_height; i ++) {
    line = img->data + i * cinfo.output_width * cinfo.output_components;
    jpeg_read_scanlines(&cinfo, &line, 1);
  }
  assert(cinfo.output_scanline == cinfo.output_height);

  (void) jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  fclose(infile);

  return 0;
}