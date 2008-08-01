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

#include <tgmath.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

star_list_t *gSKY_stars = NULL;

void
equ_cart_convert(vector_t *cart, angle_t ra, angle_t dec)
{
    scalar_t cosdec = cos(dec);
    cart->a[0] = cosdec * cos(ra);
    cart->a[1] = cosdec * sin(ra);
    cart->a[2] = sin(dec);
}

// the minimum and maximum magnetude values that correspond to alpha 255 and 0
// respectivelly
#define MIN_MAG -1.5
#define MAX_MAG 6.5
#define MAG_BASE 2.512
#define TOTAL_MAG_DIFF (MAX_MAG-MIN_MAG)
double
vmag_to_alpha(double vmag)
{
    double total_diff = pow(MAG_BASE, TOTAL_MAG_DIFF);
    double vmag_diff = pow(MAG_BASE, MAX_MAG-vmag);
    double normalised_lin_diff = (1.0-vmag_diff) / total_diff;
    
    if (normalised_lin_diff > 1.0) return 1.0;
    return normalised_lin_diff;
}

int
add_star(double ra, double dec, double mag, double bv)
{
    vector_t cart;
    equ_cart_convert(&cart, ra, dec);
    
    // star list full?
    if (gSKY_stars->a_len <= gSKY_stars->n_stars) {
        return -1;
    }
    
    double temp = bv_to_temp(bv);
    if (temp < 1000.0) temp = 1000.0;
    if (temp > 40000.0) temp = 40000.0;
    uint8_t *tempRGB = get_temp_colour((int)temp);
    
    gSKY_stars->data[gSKY_stars->n_stars].x = cart.a[0];
    gSKY_stars->data[gSKY_stars->n_stars].y = cart.a[1];
    gSKY_stars->data[gSKY_stars->n_stars].z = cart.a[2];
    gSKY_stars->data[gSKY_stars->n_stars].r = *tempRGB;
    gSKY_stars->data[gSKY_stars->n_stars].g = *(tempRGB+1);
    gSKY_stars->data[gSKY_stars->n_stars].b = *(tempRGB+2);
    gSKY_stars->data[gSKY_stars->n_stars].a = vmag_to_alpha(mag) * 255;
    if (gSKY_stars->data[gSKY_stars->n_stars].a < 40) gSKY_stars->data[gSKY_stars->n_stars].a = 40;
    
    gSKY_stars->n_stars ++;
    
    return 0;
}

double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}


star_list_t*
random_stars(void)
{
    assert(sizeof(star_t) == 4*sizeof(char)+3*sizeof(float));
    vector_t cart;
    float ra, dec;
    
    star_list_t *stars = malloc(sizeof(star_list_t) +  STAR_CNT*sizeof(star_t));
    
    for (int i = 0; i < STAR_CNT ; i ++) {
        ra = deg2rad(random() % 360-180);
        dec = deg2rad(random() % 180-90);
        equ_cart_convert(&cart, ra, dec);
        stars->data[i].x = cart.a[0];
        stars->data[i].y = cart.a[1];
        stars->data[i].z = cart.a[2];
        stars->data[i].r = 255;
        stars->data[i].g = 255;
        stars->data[i].b = 255;
        stars->data[i].a = 255;
    }
    
    stars->n_stars = STAR_CNT;
    stars->a_len = STAR_CNT;

    return stars;
}


star_list_t*
initialise_star_list(int star_count)
{
    assert(sizeof(star_t) == 4*sizeof(char)+3*sizeof(float));

    star_list_t *stars = malloc(sizeof(star_list_t) + star_count*sizeof(star_t));
    stars->a_len = star_count;
    stars->n_stars = 0;
    return stars;
}

void
init_sky(void) 
{
    gSKY_stars = initialise_star_list(STAR_CNT);
}

#include "camera.h"
void
paint_sky(star_list_t *stars)
{
    glPushMatrix();
    glLoadIdentity();

    extern camera_t *gCam;
    cam_rotate(gCam->free_cam.rq);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glInterleavedArrays(GL_C4UB_V3F, 0, stars->data);
    glDrawArrays(GL_POINTS, 0, stars->n_stars);
    
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}