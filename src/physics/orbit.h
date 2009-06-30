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
    created by the Initial Developer are Copyright (C) 2008 the
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
} OOorbobj;

typedef struct OOorbsys {
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
        float period;
        v4f_t pos;
        v4f_t rot;
      } param;
      struct {
        float G; //!< Gravitational constant (6.67428e-11)
      } k;
    } phys;
        
    struct OOorbsys *parent; // parent
    OOscene *scene; //!< Link to scene graph scene corresponding to this system

    OOellipse *orbit;

    OOobjvector sats; //!< Natural satellites, i.e. other orbital systems
    OOobjvector objs; //!< Synthetic satellites, i.e. stuff that is handled by ODE
} OOorbsys;


OOorbsys* ooOrbitNewSys(const char *name, OOscene *scene,
                        float m, float period,
                        float semiMaj, float semiMin);
OOorbobj*
ooOrbitNewObj(OOorbsys *sys, const char *name, float m,
              float x, float y, float z,
              float vx, float vy, float vz,
              float qx, float qy, float qz, float qw,
              float rqx, float rqy, float rqz, float rqw);


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
