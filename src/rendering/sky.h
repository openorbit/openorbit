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

#include <math/types.h>
#include <tgmath.h>

#include "SDL_opengl.h"

typedef struct {
    size_t n_stars; //!< Number of stars loaded
    size_t n_stars_allocated; //!< The size of the coords block
    
    GLfloat *coords;
    GLubyte *colours; // includes the magnetude in the alpha component
    
    GLushort *octant[8];
} star_list_t;

/*!
    @function   equ_cart_convert
    @abstract   Converts equatorial coordinates to cartesian unit coordinates
    @discussion The function converts an equatorial coordinates given in radians to cartesian unit coordinates that
        lies on the unit sphere
    @param      cart The resulting cartesian vector.
    @param      ra Right ascension in radians
    @param      dec Declination in radians
*/
void equ_cart_convert(vector_t *cart, angle_t ra, angle_t dec);
star_list_t *initialise_star_list(const char *path);
void add_star(double ra, double dec, double mag);
void paint_sky(star_list_t *stars);

#ifdef __cplusplus
}
#endif 

#endif
