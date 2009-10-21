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
#include "pngfile.h"

// Include the libpng header
#include <png.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int
png_read_file(png_image_t *img, FILE *file)
{
  assert(img != NULL);
  assert(file != NULL);

  uint8_t sig[8];

  fread(sig, 1, 8, file);
  if (!png_check_sig(sig, 8)) {
    return 0;
  }

  png_structp png_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_struct == NULL) {
    return -1;
  }
  
  png_infop png_info = png_create_info_struct(png_struct);
  if (png_info == NULL) {
    png_destroy_read_struct(&png_struct, NULL, NULL);
    return -1;
  }

  png_infop png_end_info = png_create_info_struct(png_struct);
  if (png_end_info == NULL) {
    png_destroy_read_struct(&png_struct, &png_info, NULL);
    return -1;
  }

  if (setjmp(png_jmpbuf(png_struct))) {
    png_destroy_read_struct(&png_struct, &png_info, &png_end_info);
    return -1;//TODO: is this for errors? If so, return -1
  }

  png_struct->io_ptr = (png_voidp)file;

  // Tell libpng that we already read the header
  png_set_sig_bytes(png_struct, 8);



  int           bit_depth;
  int           color_type;
  unsigned long width;            /* PNG image width in pixels */
  unsigned long height;           /* PNG image height in pixels */
  unsigned int rowbytes;         /* raw bytes at row n in image */

  png_read_info(png_struct, png_info);
  png_get_IHDR(png_struct, png_info, &width, &height, &bit_depth, 
      &color_type, NULL, NULL, NULL);

  png_read_update_info(png_struct, png_info);

  if (bit_depth > 8) {
    png_set_strip_16(png_struct);
  }
  if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png_struct);
  }
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
     png_set_palette_to_rgb(png_struct);
  }
  rowbytes = png_get_rowbytes(png_struct, png_info);

//  png_read_update_info(png_struct, png_info);

  void *data = malloc(rowbytes * height);
  if (!data) {
    png_destroy_read_struct(&png_struct, &png_info, NULL);
    return -1;
  }

  png_bytepp row_pointers = NULL;
  if ((row_pointers = (png_bytepp)alloca(height*sizeof(png_bytep))) == NULL) {
       png_destroy_read_struct(&png_struct, &png_info, NULL);
       //free(image_data);
       //image_data = NULL;
       return -1;
  }

  for (int i = 0;  i < height;  ++i) {
    row_pointers[i] = data + i*rowbytes;
  }
  png_read_image(png_struct, row_pointers);
  //free(row_pointers);
  png_destroy_read_struct(&png_struct, &png_info, NULL);
  
  // Data should be in RGB, here we check whether the picture had an alpha channel
  // and update the resulting structure accordingly
  if (color_type & PNG_COLOR_MASK_ALPHA) {
    img->kind = PNG_RGBA;
  } else {
    img->kind = PNG_RGB;
  }
  img->w = width;
  img->h = height;
  img->data = data;

  return 0;
}