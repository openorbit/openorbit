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
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */

#ifndef __SKY_H__
#define __SKY_H__
#ifdef __cplusplus
extern "C" {
#endif 

#include <vmath/vmath.h>

#include "scenegraph.h"

#include "SDL_opengl.h"

#define STAR_CNT 5000
    
typedef struct {
    unsigned char r, g, b, a;
    float x, y, z;
} OOstar;

typedef struct {
    size_t n_stars; //!< Number of stars loaded
    size_t a_len; //!< Length of data
    OOstar *data;
} OOstars;


/*!
    @function   equ_cart_convert
    @abstract   Converts equatorial coordinates to cartesian unit coordinates
    @discussion The function converts an equatorial coordinates given in radians to cartesian unit coordinates that
        lies on the unit sphere
    @param      cart The resulting cartesian vector.
    @param      ra Right ascension in radians
    @param      dec Declination in radians
*/
vector_t ooEquToCart(angle_t ra, angle_t dec);
OOstars* ooSkyInitStars(int starCount);
OOstars *ooSkyRandomStars(void);
    
void ooSkyAddStar(OOstars *stars, double ra, double dec, double mag, double bv);
    
void ooSkyDrawStars(OOstars *stars);

OOdrawable *ooSkyNewDrawable();


#ifdef __cplusplus
}
#endif 

#endif
