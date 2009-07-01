/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
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

vector_t
ooEquToCart(angle_t ra, angle_t dec)
{
    vector_t cart;
    scalar_t cosdec = cos(dec);
    cart.x = cosdec * cos(ra);
    cart.y = cosdec * sin(ra);
    cart.z = sin(dec);
    cart.w = 1.0;
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
    vector_t cart;
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
    uint8_t *tempRGB = get_temp_colour((int)temp);
    
    stars->data[stars->n_stars].x = cart.x;
    stars->data[stars->n_stars].y = cart.y;
    stars->data[stars->n_stars].z = cart.z;
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
  OOstars *stars = ooSkyInitStars(4096);
  FILE *f = ooResGetFile(file);
  assert(f != NULL);
  double vmag, ra, dec, btmag, vtmag, bv, vi;
  while (fscanf(f, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n",
                &vmag, &ra, &dec, &btmag, &vtmag, &bv, &vi) == 7) {
    ooSkyAddStar(stars, deg2rad(ra), deg2rad(dec), vmag, bv);
  }

  ooLogInfo("loaded %d stars from %s", stars->n_stars, file);
  return stars;
}


OOstars*
ooSkyRandomStars(void)
{
  assert(sizeof(OOstar) == 4*sizeof(char)+3*sizeof(float));
  vector_t cart;
  float ra, dec;

  OOstars *stars = malloc(sizeof(OOstars) +  STAR_CNT*sizeof(OOstar));

  for (int i = 0; i < STAR_CNT ; i ++) {
    ra = deg2rad(random() % 360-180);
    dec = deg2rad(random() % 180-90);
    cart = ooEquToCart(ra, dec);
    stars->data[i].x = cart.x;
    stars->data[i].y = cart.y;
    stars->data[i].z = cart.z;
    stars->data[i].r = 255;
    stars->data[i].g = 255;
    stars->data[i].b = 255;
    stars->data[i].a = 255;
  }
  
  stars->n_stars = STAR_CNT;
  stars->a_len = STAR_CNT;

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

#include "camera.h"
void
ooSkyDrawStars(OOstars *stars)
{
  ooLogTrace("draw %d stars", stars->n_stars);

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glPointSize(1.0f);
  glInterleavedArrays(GL_C4UB_V3F, 0, stars->data);
  glDrawArrays(GL_POINTS, 0, stars->n_stars);

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
}


OOdrawable*
ooSkyNewDrawable(const char *file)
{
  OOdrawable *sky = malloc(sizeof(OOdrawable));
  sky->obj = ooSkyLoadStars(file);
  sky->draw = (OOdrawfunc)ooSkyDrawStars;
  
  return sky;
}

