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
#include "physics.h"
#include "geo/geo.h"
#include "rendering/scenegraph.h"


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



struct PLorbsys {
    dWorldID world;
    char *name;
    
    struct {
      double dist;
      double distInv;
      double mass;
      double massInv;
    } scale;

    struct {
      struct {
        double m;
        double orbitalPeriod;
        double rotationPeriod;
        v4f_t pos;
        v4f_t rot;
      } param;
      struct {
        double G; //!< Gravitational constant (6.67428e-11)
      } k;
    } phys;

    struct PLorbsys *parent; // parent
    unsigned level;
    OOscene *scene; //!< Link to scene graph scene corresponding to this system

    OOellipse *orbit;

    OOobjvector sats; //!< Natural satellites, i.e. other orbital systems
    OOobjvector objs; //!< Synthetic satellites, i.e. stuff that is handled by ODE
};


PLorbsys* ooOrbitNewSys(const char *name, OOscene *scene,
                        double m, double orbitPeriod, double rotPeriod,
                        double semiMaj, double semiMin);
PLobject*
ooOrbitNewObj(PLorbsys *sys, const char *name,
              OOdrawable *drawable,
              double m,
              double x,   double y,   double z,
              double vx,  double vy,  double vz,
              double qx,  double qy,  double qz,  double qw,
              double rqx, double rqy, double rqz, double rqw);

/*!
   Searches the system graph for a system with the given name. The name must be
   a search path.

   e.g. ooOrbitGetSys(solsys, "sol/earth/luna") would return the luna system.
 */
PLorbsys* ooOrbitGetSys(const PLorbsys *root,  const char *name);

void ooOrbitAddChildSys(PLorbsys * restrict sys, PLorbsys * restrict child);

void ooOrbitSetScale(PLorbsys *sys, double ms, double ds);
void ooOrbitSetConstant(PLorbsys *sys, const char *key, double k);
void ooOrbitSetScene(PLorbsys *sys, OOscene *scene);

void ooOrbitStep(PLorbsys *sys, double stepsize);
void ooOrbitClear(PLorbsys *sys);

/*!
   Loads an hrml description of a solar system and builds a solar system graph
   it also connects the physics system to the graphics system.

   This function does not belong in the physics system, but will be here for
   now beeing.
 */
PLorbsys* ooOrbitLoad(OOscenegraph *sg, const char *file);

#endif /* ! _ORBIT_H_ */
