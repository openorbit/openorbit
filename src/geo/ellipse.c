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
    created by the Initial Developer are Copyright (C) 2009 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

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

#include <stdlib.h>
#include <math.h>

#include "ellipse.h"
#include "../log.h"

void
ooVecArrayInit(OOvecarray *vec)
{
    vec->asize = 16;
    vec->length = 0;
    vec->elems = calloc(vec->asize, sizeof(v4f_t));
}

void
ooVecArrayPushV(OOvecarray *vec, v4f_t obj)
{
    if (vec->length >= vec->asize) {
        v4f_t *newVec = realloc(vec->elems,
                                vec->asize * sizeof(v4f_t) * 2);
        if (newVec == NULL) ooLogFatal("realloc of vector failed");
        vec->asize *= 2;
        vec->elems = newVec;
    }
    vec->elems[vec->length ++] = obj;
}

void
ooVecArrayPushC(OOvecarray *vec, float x, float y, float z, float w)
{
  v4f_t obj = v4f_make(x,y,z,w);
  if (vec->length >= vec->asize) {
    v4f_t *newVec = realloc(vec->elems,
                            vec->asize * sizeof(v4f_t) * 2);
    if (newVec == NULL) ooLogFatal("realloc of vector failed");
    vec->asize *= 2;
    vec->elems = newVec;
  }
  vec->elems[vec->length ++] = obj;
}



v4f_t
ooVecArrayPop(OOvecarray *vec)
{    
    return vec->elems[vec->length --];
}

v4f_t
ooVecArrayGet(OOvecarray *vec, size_t i)
{
  if (vec->length <= i)
    ooLogFatal("vector out of bounds length = %d idx = %d", vec->length, i);
  else
    return vec->elems[i];
}

void
ooVecArraySet(OOvecarray *vec, size_t i, v4f_t obj)
{
  if (vec->length <= i)
    ooLogFatal("vector out of bounds length = %d idx = %d", vec->length, i);
  else
    vec->elems[i] = obj;  
}

#define ITERSTOP 100000
#define STEPSIZE 20.0
OOellipse* ooGeoEllipseAreaSeg(size_t segs, float semiMajor, float semiMinor)
{
  OOellipse *e = malloc(sizeof(OOellipse));
  ooVecArrayInit(&e->vec);
  //uint64_t totalIterations = 0;
  e->semiMajor = semiMajor;
  e->semiMinor = semiMinor;
  e->ecc = ooGeoEllipseEcc(e);
  
  double area = ooGeoEllipseArea(e);
  
  double sweep = area / (double)segs;
  
  ooVecArrayPushC(&e->vec,
                  semiMajor * cos(0.0), semiMinor * sin(0.0), 0.0f, 0.0f);
  double segArea, tol;
  double prevAngle = 0.0f;
  double newAngle = prevAngle + 1.0*DEG_TO_RAD(360.0/(double)segs);
  double delta;
  
  for (size_t i = 1 ; i < segs ; i ++) {
    int count = 0;
    segArea = ooGeoEllipseSegmentArea(e, prevAngle, newAngle);
    delta = (newAngle-prevAngle);
    do {
      if (segArea > sweep) {
        delta -= delta/STEPSIZE;
      } else {
        delta += delta/STEPSIZE;
      }
      newAngle = prevAngle + delta;
      segArea = ooGeoEllipseSegmentArea(e, prevAngle, newAngle);
      tol = fabs(1.0 - segArea/sweep);
        count ++;
    } while (tol > 0.00001 && count < ITERSTOP);

    if (count >= ITERSTOP) {
      ooLogWarn("ellipse segment did not converge in %d iterations", ITERSTOP);
    }
    //segArea = segmentArea(prevAngle, newAngle, semimajor, ecc);

    ooVecArrayPushC(&e->vec,
                    semiMajor * cos(newAngle), semiMinor * sin(newAngle),
                    0.0f, 0.0f);


    double nextNewAngle = newAngle + (newAngle-prevAngle);
    prevAngle = newAngle;
    newAngle = nextNewAngle;
  }
  
  return e;
}


double
ooGeoEllipseCircumferenceFast(OOellipse *e)
{
  return M_PI * (3.0f*(e->semiMajor + e->semiMinor) -
                 sqrtf((3.0f*e->semiMajor+e->semiMinor) *
                       (e->semiMajor+3.0f*e->semiMinor)));
}

double
ooGeoEllipseCircumferenceAccurate(OOellipse *e)
{
  double abOverAb = (e->semiMajor-e->semiMinor)/(e->semiMajor+e->semiMinor);
  double abOverAbSquare3 = 3.0f * abOverAb * abOverAb;
  
  return M_PI * (e->semiMajor + e->semiMinor) *
                (1.0f + abOverAbSquare3 /
                        (10.0f + sqrtf(4.0f-abOverAbSquare3)));
}

double
ooGeoEllipseArea(OOellipse *e)
{
  return M_PI * e->semiMajor * e->semiMinor;
}

double
ooGeoEllipseTrueAnomality(OOellipse *e, double fi)
{
  return (e->semiMajor * (1.0 - e->ecc*e->ecc)) / (1.0 + e->ecc * cos(fi));
}

double
ooGeoEllipseSegmentArea(OOellipse *e, double fi0, double fi1)
{
  double r0 = ooGeoEllipseTrueAnomality(e, fi0);
  double r1 = ooGeoEllipseTrueAnomality(e, fi1);

  double x0 = r0 * cos(fi0);
  double y0 = r0 * sin(fi0);
  double x1 = r1 * cos(fi1);
  double y1 = r1 * sin(fi1);
  
  double seg = 0.5 * fabs(x0*y1-x1*y0);
  //printf("segArea: r0: %f r1: %f, v0 = (%f, %f), v1 = (%f, %f)\n", r0, r1, x0, y0, x1, y1);
  
  return seg;
  
}

double
ooGeoEllipseEcc(OOellipse *e)
{
  return sqrt(1.0-(e->semiMinor/e->semiMajor)*(e->semiMinor/e->semiMajor));
}

