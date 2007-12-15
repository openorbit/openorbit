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

 
#include "sky.h"
#include <tgmath.h>
#include <stdio.h>

void
equ_cart_convert(vector_t *cart, angle_t ra, angle_t dec)
{
    scalar_t cosdec = cos(dec);
    cart->a[0] = cosdec * cos(ra);
    cart->a[1] = cosdec * sin(ra);
    cart->a[2] = sin(dec);
}

void
add_star(double ra, double dec, double mag)
{
    vector_t cart;
    
    equ_cart_convert(&cart, ra, dec);
    
}

star_list_t*
initialise_star_list(const char *path)
{
    FILE *catalogue = NULL;
    
    if (catalogue = fopen(path, "r")) {
        
    }
    
    return NULL;
}

void
paint_sky(star_list_t *stars)
{
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    
    //glVertexPointer(4, GL_FLOAT, 0, stars->coords);
    //glColorPointer(4, GL_BYTE, 0, stars->colours);
    
    //glDrawElements(GL_POINTS, stars->n_stars, GL_UNSIGNED_SHORT, stars->octant0);
    
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
}