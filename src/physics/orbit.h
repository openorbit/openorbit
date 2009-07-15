/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

/*
    Large orbital objects
*/
#ifndef _ORBIT_H_
#define _ORBIT_H_

#include <ode/ode.h>
#include <openorbit/openorbit.h>
#include <vmath/vmath.h>

#include "geo/geo.h"
#include "rendering/scenegraph.h"

typedef struct OOorbsys OOorbsys;

static inline v4f_t
ode2v4(const dReal *vec)
{
  return v4f_make(vec[0], vec[1], vec[2], vec[3]);
}

static inline v4f_t
ode2v3(const dReal *vec)
{
  return v4f_make(vec[0], vec[1], vec[2], 1.0f);
}


typedef struct OOorbobj {
    char *name;
    dBodyID id;
    float m;
    OOdrawable *drawable; //!< Link to scenegraph drawable object representing this
                          //!< object.

    OOorbsys *sys;
} OOorbobj;

struct OOorbsys {
    dWorldID world;
    char *name;
    
    struct {
      float dist;
      float distInv;
      float mass;
      float massInv;
    } scale;

    struct {
      struct {
        float m;
        float orbitalPeriod;
        float rotationPeriod;
        v4f_t pos;
        v4f_t rot;
      } param;
      struct {
        float G; //!< Gravitational constant (6.67428e-11)
      } k;
    } phys;

    struct OOorbsys *parent; // parent
    unsigned level;
    OOscene *scene; //!< Link to scene graph scene corresponding to this system

    OOellipse *orbit;

    OOobjvector sats; //!< Natural satellites, i.e. other orbital systems
    OOobjvector objs; //!< Synthetic satellites, i.e. stuff that is handled by ODE
};


OOorbsys* ooOrbitNewSys(const char *name, OOscene *scene,
                        float m, float orbitPeriod, float rotPeriod,
                        float semiMaj, float semiMin);
OOorbobj*
ooOrbitNewObj(OOorbsys *sys, const char *name,
              OOdrawable *drawable,
              float m,
              float x, float y, float z,
              float vx, float vy, float vz,
              float qx, float qy, float qz, float qw,
              float rqx, float rqy, float rqz, float rqw);

/*!
   Searches the system graph for a system with the given name. The name must be
   a search path.

   e.g. ooOrbitGetSys(solsys, "sol/earth/luna") would return the luna system.
 */
OOorbsys* ooOrbitGetSys(const OOorbsys *root,  const char *name);

void ooOrbitAddChildSys(OOorbsys * restrict sys, OOorbsys * restrict child);

void ooOrbitSetScale(OOorbsys *sys, float ms, float ds);
void ooOrbitSetConstant(OOorbsys *sys, const char *key, float k);
void ooOrbitSetScene(OOorbsys *sys, OOscene *scene);

void ooOrbitStep(OOorbsys *sys, float stepsize);
void ooOrbitClear(OOorbsys *sys);

/*!
   Loads an hrml description of a solar system and builds a solar system graph
   it also connects the physics system to the graphics system.

   This function does not belong in the physics system, but will be here for
   now beeing.
 */
OOorbsys* ooOrbitLoad(OOscenegraph *sg, const char *file);

#endif /* ! _ORBIT_H_ */
