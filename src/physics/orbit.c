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

#include "orbit.h"
#include <ode/ode.h>
#include <assert.h>


#include <vmath/vmath.h>

#include "log.h"





static OOorbsys *gOrb_root_system;

void
ooOrbitSetRoot(OOorbsys *sys)
{
    ooLogTrace("root system %s added\n", sys->name);
    gOrb_root_system = sys;
}

OOorbsys*
ooOrbitNewSys(const char *name, float radius, float w0)
{
    OOorbsys *sys = malloc(sizeof(OOorbsys));
    if (sys == NULL) goto malloc_failed;
    
    sys->world = dWorldCreate();
    sys->name = strdup(name);
    if (sys->name == NULL) goto malloc_failed;

    sys->parent = NULL;
    sys->child = NULL;
    sys->next = NULL;
    sys->obj = NULL;
    sys->id = 0;
    
    sys->k.G = 6.67428e-11;
    
    return sys;
    
malloc_failed:
    // we get here if we run out of memory
    if (sys) {
        dWorldDestroy(sys->world);
        free(sys->name);        
        free(sys);
    }
    ooLogFatal("failed malloc when adding %s system\n", name);
}

OOorbobj*
ooOrbitAddObj(OOorbsys *sys, const char *name, float radius, float w0, float m)
{
    assert(sys != NULL);
    OOorbobj *obj = malloc(sizeof(OOorbobj));
    if (obj == NULL) {
        free(obj);
        return NULL;
    }
    
    obj->name = strdup(name);
    obj->id = dBodyCreate(sys->world);

    // Insert as first object
    obj->next = sys->obj;
    sys->obj = obj;
        
    // Add mass to parent systems, note that this assumes that the orbital mass
    // is in the same unit
    obj->m = m;
    OOorbsys *sp = sys;
    while (sp) {
        sp->m += m;
        sp = sp->parent;
    }
    
    return obj;
}


void
ooOrbitAddChildSys(OOorbsys * restrict parent, OOorbsys * restrict child)
{        
    child->id = dBodyCreate(parent->world);
    child->next = parent->child;
    parent->child = child;
}
#if 0

void
orbit_init_obj(OOorbobj *obj)
{
    
}
// Initalises the necissary physical parameters for the simulation system
void
orbit_init_sys(OOorbsys *sys)
{
    orb_obj_node_t *onode = sys->obj;
    while (onode) {
        orbit_init_obj(onode->obj);
        onode = onode->next;
    }
    
    orb_sys_node_t *snode = sys->subsys;
    while (snode) {
        orbit_init_sys(snode->sys);
        snode = snode->next;
    }

    
}
#endif
void
ooOrbitClear(OOorbsys *sys)
{
    if (sys == NULL) return;
    
    if (sys->parent != NULL) {
        dBodySetForce(sys->id, 0.0f, 0.0f, 0.0f);
        dBodySetTorque(sys->id, 0.0f, 0.0f, 0.0f);
    }
    OOorbobj *obj = sys->obj;
    while (obj) {
        dBodySetForce(obj->id, 0.0f, 0.0f, 0.0f);
        dBodySetTorque(obj->id, 0.0f, 0.0f, 0.0f);

        obj = obj->next;
    }
    
    ooOrbitClear(sys->child); // children
    ooOrbitClear(sys->next); // siblings
}

#define P_G 6.67428e-11 
void
ooOrbitStep(OOorbsys *sys, float stepsize)
{
    if (sys == NULL) return;
    
    // Solves mutual force equations for all objects in the system, and then solves them
    // for each subsystem
    OOorbobj *onode0 = sys->obj;
    while (onode0) {
        const dReal *ode_o0p = dBodyGetPosition (onode0->id);
        vector_t o0p = v_set(ode_o0p[0], ode_o0p[1], ode_o0p[2], 0.0f);
        OOorbobj *onode1 = onode0->next;
        while (onode1) {
            const dReal *ode_o1p = dBodyGetPosition (onode1->id);
            vector_t o1p = v_set(ode_o1p[0], ode_o1p[1], ode_o1p[2], 0.0f);            
            vector_t dist = v_sub(o1p, o0p);

            scalar_t r12 = v_abs(dist);
            r12 = r12 * r12;
            vector_t f12 = v_s_mul(v_normalise(dist),
                                  -sys->k.G*onode0->m * onode1->m / r12);
            vector_t f21 = v_neg(f12);
            
            dBodyAddForce(onode0->id, f12.x, f12.y, f12.z);
            dBodyAddForce(onode1->id, f21.x, f21.y, f21.z);

            onode1 = onode1->next;
        }
        
        // all system children and objects
        OOorbsys *snode = sys->child;
        while (snode) {
            const dReal *ode_s1p = dBodyGetPosition (snode->id);
            vector_t s1p = v_set(ode_s1p[0], ode_s1p[1], ode_s1p[2], 0.0f);            
            vector_t dist = v_sub(s1p, o0p);

            scalar_t r12 = v_abs(dist);
            r12 = r12 * r12;
            vector_t f12 = v_s_mul(v_normalise(dist),
                                   -sys->k.G*onode0->m * snode->m / r12);
            vector_t f21 = v_neg(f12);
            
            dBodyAddForce(onode0->id, f21.x, f21.y, f21.z);
            dBodyAddForce(snode->id, f12.x, f12.y, f12.z);

            snode = snode->next;
        }
                
        onode0 = onode0->next;
    }
    
    dWorldStep(sys->world, stepsize);
    
    // all siblings
    ooOrbitStep(sys->next, stepsize);

    // handle children    
    ooOrbitStep(sys->child, stepsize);
}

void
ooOrbitSetConstant(OOorbsys *sys, const char *key, float k)
{
    assert(sys != NULL && "sys is null");
    assert(key != NULL && "key is null");
    
    if (!strcmp(key, "G")) {
        sys->k.G = k;
    }
}
