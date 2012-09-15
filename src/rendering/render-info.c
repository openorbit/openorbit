//
//  render-info.c
//  orbit
//
//  Created by Mattias Holm on 2012-09-03.
//
//

#include <stdio.h>

struct sg_render_info_t {
  unsigned w;
  unsigned h;
  float aspect;
  float fovy;
  unsigned gl_major_vers;
  unsigned gl_minor_vers;
  unsigned glsl_major_vers;
  unsigned glsl_minor_vers;
};

