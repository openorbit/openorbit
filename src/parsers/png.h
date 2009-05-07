#ifndef PNG_H_DU136IQO
#define PNG_H_DU136IQO
#include <stdint.h>
#include <stdio.h>


typedef enum {
  PNG_RGB,
  PNG_RGBA
} png_kind_t;
typedef struct {
  png_kind_t kind;
  uint32_t w, h;
  uint8_t *data;  
} png_image_t;

int png_read_file(png_image_t *img, FILE *file);

#endif /* end of include guard: PNG_H_DU136IQO */
