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
#include <openorbit/log.h>
#include <tgmath.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#include "scenegraph-private.h"
#include "scenegraph.h"
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


double deg2rad(double deg) {
    return deg * M_PI / 180.0;
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
  sg_background_t *stars = malloc(sizeof(sg_background_t));
  stars->data = calloc(STAR_COUNT, sizeof(sg_star_t));
  stars->a_len = STAR_COUNT;
  stars->n_stars = 0;

  FILE *f = ooResGetFile(file);
  assert(f != NULL);
  double vmag, ra, dec, btmag, vtmag, bv, vi;
  while (fscanf(f, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                &vmag, &ra, &dec, &btmag, &vtmag, &bv, &vi) == 7) {
    sgAddStar(stars, deg2rad(ra), deg2rad(dec), vmag, bv);
  }


  ooLogInfo("loaded %d stars from %s", stars->n_stars, file);
  glGenVertexArrays(1, &stars->vba);
  glBindVertexArray(stars->vba);

  glGenBuffers(1, &stars->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, stars->vbo);
  glBufferData(GL_ARRAY_BUFFER, stars->n_stars*sizeof(sg_star_t), stars->data,
               GL_STATIC_DRAW);

  sg_shader_t *shader = sgGetProgram("stars");
  glVertexAttribPointer(sgGetLocationForParam(shader->shaderId, SG_VERTEX),
                        3, GL_FLOAT, GL_FALSE,
                        sizeof(sg_star_t), (void*)offsetof(sg_star_t, x));
  glVertexAttribPointer(sgGetLocationForParam(shader->shaderId, SG_COLOR),
                        4, GL_UNSIGNED_BYTE,
                        GL_TRUE, // Yes normalize
                        sizeof(sg_star_t), offsetof(sg_star_t, r));

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  free(stars->data); // GL has copied arrays over
  stars->data = NULL;

  SG_CHECK_ERROR;

  return stars;

}
void
sgDrawBackground(sg_background_t *bg)
{
  // Background drawing needs to shortcut the normal camera, as the bg
  // is at infinite distance (e.g. regarding translation)
  // Here we rotate the camera as needed
  sg_camera_t *cam = bg->cam;
  float4x4 rotMatrix;
  switch (bg->cam->type) {
    case SG_CAMERA_FIXED: {
      quaternion_t q = q_mul(cam->fixed.obj->q, cam->fixed.q);
      q_mf4_convert(rotMatrix, q);
      break;}
    case SG_CAMERA_FREE: {
      q_mf4_convert(rotMatrix, cam->free.q);
      break;}
    case SG_CAMERA_ORBITING: {
      mf4_lookat(rotMatrix,
                 0.0f, 0.0f, 0.0f,
                 -cam->orbiting.r * cosf(cam->orbiting.dec),
                 -cam->orbiting.r * sinf(cam->orbiting.dec),
                 -cam->orbiting.r * sinf(cam->orbiting.ra),
                 0.0f, 0.0f, 1.0f);

      break;}
  }

  glUseProgram(bg->shader->shaderId);
  glUniformMatrix4fv(bg->shader->uniforms.projectionId, 1, GL_TRUE,
                     (GLfloat*)bg->cam->proj_matrix);
  glUniformMatrix4fv(bg->shader->uniforms.modelViewId, 1, GL_TRUE,
                     (GLfloat*)rotMatrix);
  
}
