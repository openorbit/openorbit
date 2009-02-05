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

#endif /* end of include guard: ELLIPSE_H_1304815C */

