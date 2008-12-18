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
#include <vmath/vmath.h>


typedef struct __OOorbobj {
    char *name;
    dBodyID id;
    vector_t pos;
    scalar_t m;
    struct __OOorbobj *next;
} OOorbobj;


typedef struct _OOorbsys {
    dBodyID id; // in parent system this system has an object id
    dWorldID world;
    char *name;
    
    float scale; //!< Scale of system in spatial dimentions (1.0 = m)
    struct {
        float G; //!< Gravitational constant (6.67428e-11)
    } k;
    
    vector_t pos;
    float m; // sum of subsystem masses
    
    OOorbobj *obj;
    
    struct _OOorbsys *parent; // parent
    struct _OOorbsys *next; // siblings
    struct _OOorbsys *child; // children
} OOorbsys;

void ooOrbitSetRoot(OOorbsys *sys);

OOorbsys* ooOrbitNewSys(const char *name, float radius, float w0);
OOorbobj* ooOrbitAddObj(OOorbsys *sys, const char *name, float radius, float w0, float m);
void ooOrbitAddChildSys(OOorbsys * restrict sys, OOorbsys * restrict child);
void ooOrbitSetConstant(OOorbsys *sys, const char *key, float k);

void ooOrbitStep(OOorbsys *sys, float stepsize);
void ooOrbitClear(OOorbsys *sys);

#endif /* ! _ORBIT_H_ */
