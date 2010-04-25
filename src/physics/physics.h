/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*!
  Physics simulation system

  The physics library of open orbit is used to simulate astronomnical systems of
  large objects orbiting eachother and smaller objects (like spacecrafts)
  scattered around in the world.

  The physics library has five major object types that need to be known of.
  1. PLworld that encapsulate all the state of the simulation, including a root
     system.
  2. PLsystem that identify a system of astronomical bodies with one body
     associated with the system.
  3. PLastrobody that represents a large astronomical body, like the sun, a
     planet or a moon.
  4. PLobject that represent a small object.
  5. PLmass that represent mass and inertia parameters.

  A feature with the physics library is that it is integrated with the
  scenegraph, that means that the objects keep track of their respective model
  and potential lights associated with the system (e.g. point light for a star,
  and reflective light from a planet / moon).
 */

#ifndef PHYSICS_H_MW8CHG7V
#define PHYSICS_H_MW8CHG7V

#include <vmath/vmath.h>

#include "common/lwcoord.h"
#include "physics/reftypes.h"

#include "rendering/scenegraph.h"

#ifndef __has_feature
#define __has_feature(x) 0  // Compatibility with non-clang compilers.
#endif

#if __has_feature(attribute_ext_vector_type)
typedef float __attribute__((ext_vector_type (4))) PLfloat3;
typedef double __attribute__((ext_vector_type (4))) PLdouble3;
typedef int64_t __attribute__((ext_vector_type (4))) PLlong3;
typedef int32_t __attribute__((ext_vector_type (4))) PLint3;
typedef int16_t __attribute__((ext_vector_type (4))) PLshort3;
typedef PLfloat3 __attribute__((ext_vector_type (4))) PLfloat3x3;
#else
typedef float __attribute__((vector_size (16))) PLfloat3;
typedef double __attribute__((vector_size (32))) PLdouble3;
typedef int64_t __attribute__((vector_size (32))) PLlong3;
typedef int32_t __attribute__((vector_size (16))) PLint3;
typedef int16_t __attribute__((vector_size (8))) PLshort3;

#endif

// Speed of light
#define PL_C 299792458.0
// Universial gravitation m^3/kg/s^2
#define PL_G 6.67428e-11
// Planck constant
#define PL_H 6.62606896e-34
// Universal gas constant
#define PL_UGC 8.31432
// Magnetic permeability
#define PL_U0 1.256637061e-6
// Electric permittivity
#define PL_E0 8.854187817e-12
// Avogrados Number
#define PL_NA 6.0221415e23
// Boltzmann
#define PL_KB 1.3806504e-23
// Stefan (W/m2/K4)
#define PL_ST 5.670400e-8
// Molar Mass Constant
#define PL_M0 1.99264654e-26;

#define PL_SEC_PER_DAY (3600.0 * 24.0)

#include "object.h"
#include "orbit.h"
#include "mass.h"

// We cannot cope with the precision issues without subdividing the universe
// If we take 1.0 Tm side boxes, we can maintain decent double precision units and still
// use metres. For now, we use 16 bit integers to identify the box, which means that the
// total volume available is something like +/- 200 au on each side
#define PL_SEGMENT_LEN 1024.0f
#define PL_SEGMENT_LEN64 1024.0


// Unit conversion macros

void plInit(void);
static inline double
plMetresToAu(double m)
{
  return m / 149598000000.0;
}

static inline PLdouble3
plMetresToAu_v(PLdouble3 m)
{
  return vd3_s_div(m, 149598000000.0);
}


static inline double
plAuToMetres(double au)
{
  return au * 149598000000.0;
}

static inline PLdouble3
plAuToMetres_v(PLdouble3 au)
{
  return vd3_s_mul(au, 149598000000.0);
}


static inline double
plMetresToPc(double m)
{
  return m / 3.08e16;
}

static inline double
plPcToMetres(double pc)
{
  return pc * 3.08e16;
}

static inline PLdouble3
plMetresToPc_v(PLdouble3 m)
{
  return vd3_s_div(m, 3.08e16);
}

static inline PLdouble3
plPcToMetres_v(PLdouble3 pc)
{
  return vd3_s_mul(pc, 3.08e16);
}



static inline double
plPaToBar(double p)
{
  return p / 100000.0;
}

static inline double
plBarToPa(double p)
{
  return p * 100000.0;
}


static inline double
plCToK(double t)
{
  return t + 273.16;
}

static inline double
plKToC(double t)
{
  return t - 273.16;
}


#endif /* end of include guard: PHYSICS_H_MW8CHG7V */
