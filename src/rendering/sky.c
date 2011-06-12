/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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



#include "sky.h"
#include "colour.h"
#include "res-manager.h"
#include "log.h"
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

void
ooSkyAddStar(OOstars *stars, double ra, double dec, double mag, double bv)
{
    float3 cart;
    cart = ooEquToCart(ra, dec);

    // star list full, then extend block?
    if (stars->a_len <= stars->n_stars) {
      OOstar *newData = realloc(stars->data, stars->a_len * 2 * sizeof(OOstar));
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

double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

OOstars* ooSkyLoadStars(const char *file)
{
  SG_CHECK_ERROR;

  OOstars *stars = ooSkyInitStars(4096);
  FILE *f = ooResGetFile(file);
  assert(f != NULL);
  double vmag, ra, dec, btmag, vtmag, bv, vi;
  while (fscanf(f, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                &vmag, &ra, &dec, &btmag, &vtmag, &bv, &vi) == 7) {
    ooSkyAddStar(stars, deg2rad(ra), deg2rad(dec), vmag, bv);
  }

  ooLogInfo("loaded %d stars from %s", stars->n_stars, file);
  glGenBuffers(1, &stars->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, stars->vbo);
  glBufferData(GL_ARRAY_BUFFER, stars->n_stars*sizeof(OOstar), stars->data,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  free(stars->data); // GL has copied arrays over
  stars->data = NULL;

  SG_CHECK_ERROR;

  return stars;
}


OOstars*
ooSkyRandomStars(void)
{
  assert(sizeof(OOstar) == 4*sizeof(char)+3*sizeof(float));
  float3 cart;
  float ra, dec;

  OOstars *stars = malloc(sizeof(OOstars) +  STAR_CNT*sizeof(OOstar));

  for (int i = 0; i < STAR_CNT ; i ++) {
    ra = deg2rad(random() % 360-180);
    dec = deg2rad(random() % 180-90);
    cart = ooEquToCart(ra, dec);
    stars->data[i].x = 100.0*vf3_x(cart);
    stars->data[i].y = 100.0*vf3_y(cart);
    stars->data[i].z = 100.0*vf3_z(cart);
    stars->data[i].r = 255;
    stars->data[i].g = 255;
    stars->data[i].b = 255;
    stars->data[i].a = 255;
  }

  stars->n_stars = STAR_CNT;
  stars->a_len = STAR_CNT;
  glGenBuffers(1, &stars->vbo);
  glBindBuffer(GL_ARRAY_BUFFER, stars->vbo);
  glBufferData(GL_ARRAY_BUFFER, stars->n_stars*sizeof(OOstar), stars->data,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  free(stars->data); // GL has copied arrays over
  stars->data = NULL;
  SG_CHECK_ERROR;

  return stars;
}


OOstars*
ooSkyInitStars(int star_count)
{
  assert(sizeof(OOstar) == 4*sizeof(char)+3*sizeof(float));

  OOstars *stars = malloc(sizeof(OOstars));
  stars->data = malloc(star_count*sizeof(OOstar));
  stars->a_len = star_count;
  stars->n_stars = 0;
  return stars;
}

void
ooSkyDrawStars(OOstars *stars)
{
  SG_CHECK_ERROR;

  ooLogTrace("draw %d stars", stars->n_stars);
  glMatrixMode(GL_MODELVIEW);
  glPushAttrib(GL_ENABLE_BIT);
  glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindBuffer(GL_ARRAY_BUFFER, stars->vbo);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);

  glPointSize(2.0f);
  glVertexPointer(3, GL_FLOAT, sizeof(OOstar), (GLvoid*)offsetof(OOstar, x));
  glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(OOstar), (GLvoid*)offsetof(OOstar, r));

  glDrawArrays(GL_POINTS, 0, stars->n_stars);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glPopAttrib();
  glPopClientAttrib();
  SG_CHECK_ERROR;
}


SGdrawable*
ooSkyNewDrawable(const char *file)
{
  //  OOdrawable *sky = malloc(sizeof(OOdrawable));
  OOstars *sky = ooSkyLoadStars(file);
  SGdrawable *drawable = sgNewDrawable((SGdrawable*)sky, "sky", (SGdrawfunc)ooSkyDrawStars);

  return drawable;
}

