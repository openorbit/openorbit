/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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



#include "sky.h"
#include "colour.h"
#include "res-manager.h"
#include "common/moduleinit.h"
#include "sim.h"
#include "io-manager.h"

#include <openorbit/log.h>
#include <tgmath.h>

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "scenegraph.h"
#include "palloc.h"

struct sg_star_t {
  unsigned char r, g, b, a;
  float x, y, z;
};


struct sg_background_t {
  sg_scene_t *scene;
  sg_shader_t *shader;

  GLuint vbo;
  GLuint vba;

  size_t a_len;
  size_t n_stars;
  sg_star_t *data;

  bool icrf_enabled;

  GLuint icrf_vbo;
  GLuint icrf_vba;
  size_t icrf_num_lines;
  sg_shader_t *icrf_shader;
};

void
sg_new_icrf_grid(sg_background_t *bg)
{
  float_array_t arr;
  float_array_init(&arr);
  // We want to show the grid with slices and stacks of M_PI_4
  // We are not generating triangles, but lines.
#define STACKS 10
#define SLICES 16
  double az_sz = (2.0 * M_PI) / (double)SLICES;
  double inc_sz = M_PI / (double)STACKS;

  // Handle non polar stacks, note that the bottom stack is handled specially
  for (int i = 1 ; i < STACKS ; i ++) {
    double inc = i * M_PI / (double)STACKS;

    for (int j = 0 ; j < SLICES ; j ++) {
      // For every slice we generate two lines
      double az = j * (2.0 * M_PI) / (double)SLICES;

      // Line 1 longitude
      float3 p[4];
      p[0].x = sin(inc) * cos(az);
      p[0].y = sin(inc) * sin(az);
      p[0].z = cos(inc);

      p[1].x = sin(inc) * cos(az+az_sz);
      p[1].y = sin(inc) * sin(az+az_sz);
      p[1].z = cos(inc);

      // Line 2 latitude
      if (i != STACKS - 1) {
        p[2].x = sin(inc) * cos(az);
        p[2].y = sin(inc) * sin(az);
        p[2].z = cos(inc);

        p[3].x = sin(inc+inc_sz) * cos(az);
        p[3].y = sin(inc+inc_sz) * sin(az);
        p[3].z = cos(inc+inc_sz);
        for (int k = 0 ; k < 4 ; k ++) {
          float_array_push(&arr, p[k].x);
          float_array_push(&arr, p[k].y);
          float_array_push(&arr, p[k].z);
        }
      } else {
        for (int k = 0 ; k < 2 ; k ++) {
          float_array_push(&arr, p[k].x);
          float_array_push(&arr, p[k].y);
          float_array_push(&arr, p[k].z);
        }
      }
    }
  }
  bg->icrf_num_lines = arr.length/3/2;

  glGenVertexArrays(1, &bg->icrf_vba);
  glBindVertexArray(bg->icrf_vba);

  glGenBuffers(1, &bg->icrf_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, bg->icrf_vbo);
  glBufferData(GL_ARRAY_BUFFER, bg->icrf_num_lines*sizeof(float)*3*2, arr.elems,
               GL_STATIC_DRAW);
  SG_CHECK_ERROR;

  sg_shader_t *shader = sg_get_shader("flat");
  bg->icrf_shader = shader;
  glVertexAttribPointer(sg_shader_get_location(shader, SG_VERTEX, true),
                        3, GL_FLOAT, GL_FALSE,
                        sizeof(float)*3, 0);
  glEnableVertexAttribArray(sg_shader_get_location(shader, SG_VERTEX, true));
  SG_CHECK_ERROR;

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);


  float_array_dispose(&arr);
}


void
sg_background_set_scene(sg_background_t *bg, sg_scene_t *sc)
{
  bg->scene = sc;
}

float3
ooEquToCart(float ra, float dec)
{
  float3 cart;
  float cosdec = cos(dec);
  cart = vf3_set(cosdec * cos(ra), cosdec * sin(ra), sin(dec));
  return cart;
}

// the minimum and maximum magnetude values that correspond to alpha 255 and 0
// respectivelly
#define MIN_MAG -1.5
#define MAX_MAG 6.5
#define MAG_BASE 2.512
#define TOTAL_MAG_DIFF (MAX_MAG-MIN_MAG)
double
ooVmagToAlpha(double vmag)
{
    double total_diff = pow(MAG_BASE, TOTAL_MAG_DIFF);
    double vmag_diff = pow(MAG_BASE, MAX_MAG-vmag);
    double normalised_lin_diff = (1.0-vmag_diff) / total_diff;

    if (normalised_lin_diff > 1.0) return 1.0;
    return normalised_lin_diff;
}

void
sgAddStar(sg_background_t *stars, double ra, double dec, double mag, double bv)
{
  float3 cart;
  cart = ooEquToCart(ra, dec);

  // star list full, then extend block?
  if (stars->a_len <= stars->n_stars) {
    sg_star_t *newData = realloc(stars->data, stars->a_len * 2 * sizeof(sg_star_t));
    stars->data = newData;
    stars->a_len *= 2;
    return;
  }

  double temp = bv_to_temp(bv);
  if (temp < 1000.0) temp = 1000.0;
  if (temp > 40000.0) temp = 40000.0;
  const uint8_t *tempRGB = get_temp_colour((int)temp);

  stars->data[stars->n_stars].x = 100.0*vf3_x(cart);
  stars->data[stars->n_stars].y = 100.0*vf3_y(cart);
  stars->data[stars->n_stars].z = 100.0*vf3_z(cart);
  stars->data[stars->n_stars].r = *tempRGB;
  stars->data[stars->n_stars].g = *(tempRGB+1);
  stars->data[stars->n_stars].b = *(tempRGB+2);
  stars->data[stars->n_stars].a = ooVmagToAlpha(mag) * 255;
  if (stars->data[stars->n_stars].a < 40) stars->data[stars->n_stars].a = 40;

  stars->n_stars ++;
}


#define STAR_COUNT 4096
sg_background_t*
sgCreateBackgroundFromFile(const char *file)
{
  SG_CHECK_ERROR;

  sg_background_t *stars = smalloc(sizeof(sg_background_t));
  memset(stars, 0, sizeof(sg_background_t));

  stars->data = calloc(STAR_COUNT, sizeof(sg_star_t));
  stars->a_len = STAR_COUNT;
  stars->n_stars = 0;

  FILE *f = rsrc_get_file(file);
  assert(f != NULL);
  double vmag, ra, dec, btmag, vtmag, bv, vi;
  while (fscanf(f, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                &vmag, &ra, &dec, &btmag, &vtmag, &bv, &vi) == 7) {
    sgAddStar(stars, ra*VMATH_RAD_PER_DEG, dec*VMATH_RAD_PER_DEG, vmag, bv);
  }


  log_info("loaded %d stars from %s", stars->n_stars, file);
  glGenVertexArrays(1, &stars->vba);
  glBindVertexArray(stars->vba);

  glGenBuffers(1, &stars->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, stars->vbo);
  glBufferData(GL_ARRAY_BUFFER, stars->n_stars*sizeof(sg_star_t), stars->data,
               GL_STATIC_DRAW);
  SG_CHECK_ERROR;

  sg_shader_t *shader = sg_get_shader("sky");
  stars->shader = shader;
  glVertexAttribPointer(sg_shader_get_location(shader, SG_VERTEX, true),
                        3, GL_FLOAT, GL_FALSE,
                        sizeof(sg_star_t), (void*)offsetof(sg_star_t, x));
  glEnableVertexAttribArray(sg_shader_get_location(shader, SG_VERTEX, true));
  SG_CHECK_ERROR;

  glVertexAttribPointer(sg_shader_get_location(shader, SG_COLOR, true),
                        4, GL_UNSIGNED_BYTE,
                        GL_TRUE, // Yes normalize
                        sizeof(sg_star_t), offsetof(sg_star_t, r));
  glEnableVertexAttribArray(sg_shader_get_location(shader, SG_COLOR, true));
  SG_CHECK_ERROR;

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //glBindBuffer(GL_ARRAY_BUFFER, 0);
  free(stars->data); // GL has copied arrays over
  stars->data = NULL;
  SG_CHECK_ERROR;

  sg_new_icrf_grid(stars);
  return stars;
}

void
sg_background_draw(sg_background_t *bg)
{
  SG_CHECK_ERROR;
  glDisable(GL_DEPTH_TEST);
  // Background drawing needs to shortcut the normal camera, as the bg
  // is at infinite distance (e.g. regarding translation)
  // Here we rotate the camera as needed
  sg_camera_t *cam = sg_scene_get_cam(bg->scene);

  SG_CHECK_ERROR;
  sg_shader_bind(bg->shader);
  sg_shader_set_projection(bg->shader, *sg_camera_project(cam));
  sg_shader_set_model_view(bg->shader, *sg_camera_modelview(cam));

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindVertexArray(bg->vba);
  glDrawArrays(GL_POINTS, 0, bg->n_stars);
  glBindVertexArray(0);
  SG_CHECK_ERROR;

  // Draw icrf grid
  if (bg->icrf_enabled) {
    sg_shader_bind(bg->icrf_shader);
    sg_shader_set_projection(bg->icrf_shader, *sg_camera_project(cam));
    sg_shader_set_model_view(bg->icrf_shader, *sg_camera_modelview(cam));
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(bg->icrf_vba);
    glDrawArrays(GL_LINES, 0, bg->icrf_num_lines*2);
    glBindVertexArray(0);
    SG_CHECK_ERROR;
  }
}

void
sg_icrf_toggle(int button_val, void *data)
{
  sg_scene_t *sc = sim_get_scene();
  sg_background_t *bg = sg_scene_get_bg(sc);

  if (button_val == 1) {
    bg->icrf_enabled = !bg->icrf_enabled;
  }
}

MODULE_INIT(sky, "iomanager", NULL) {
  io_reg_action_handler("icrf-grid-toggle", sg_icrf_toggle, IO_BUTTON_PUSH, NULL);
}


