/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef ELLIPSE_H_1304815C
#define ELLIPSE_H_1304815C

#include <vmath/vmath.h>

// TODO: Move to common location
typedef struct {
    size_t asize; //!< Size of elems array in objects
    size_t length; //!< Usage of elems array in objects
    v4f_t *elems;
} OOvecarray;

typedef struct {
  double ecc;
  double semiMajor;
  double semiMinor;
  double inc;
  double ascendingNode;
  double periapsis;
  double anomaly;
  
  OOvecarray vec;
} OOellipse;

/*!
 * Create a new ellipse with cached vertices separated with a equally sized
 * area segments. This is usefull when one is after orbital data.
 *
 * \param segs The number of segments in the created ellipse
 * \param semiMajor Semi-major axis
 * \param semiMinor Semi-minor axis
 * \bug This function use a very primitive solver that will take over 1000
 *      iterations to converge for each segment in the ellipse.
 * \bug Does not verify that semiMajor >= semiMinor.
 * \todo Replace sovler with more efficient method.
 */
OOellipse* ooGeoEllipseAreaSeg(size_t segs, float semiMajor, float semiMinor);
double ooGeoEllipseCircumferenceFast(OOellipse *e);
double ooGeoEllipseCircumferenceAccurate(OOellipse *e);
double ooGeoEllipseArea(OOellipse *e);
double ooGeoEllipseTrueAnomality(OOellipse *e, double fi);
double ooGeoEllipseSegmentArea(OOellipse *e, double fi0, double fi1);
double ooGeoEllipseEcc(OOellipse *e);
v4f_t ooGeoEllipseSegPoint(OOellipse *e, double t);

static inline double ooGeoComputeSemiMinor(double semiMajor, double ecc)
{
  return semiMajor * sqrt(1 - ecc * ecc);
}

void ooGeoEllipseDraw(OOellipse *e);

#endif /* end of include guard: ELLIPSE_H_1304815C */

