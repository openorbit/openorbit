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

struct _orb_obj_t {
    char *name;
    dBodyID id;
    vector_t pos;
    scalar_t m;
};

typedef struct _orb_obj_node_t {
    orb_obj_t *obj;
    struct _orb_obj_node_t *next;
} orb_obj_node_t;


typedef struct _orb_sys_node_t {
    orb_sys_t *sys;
    struct _orb_sys_node_t *next;
} orb_sys_node_t;

struct _orb_sys_t {
    dBodyID id; // in parent system this system has an object id
    dWorldID world;
    char *name;
    orb_sys_t *parent;
    
    vector_t pos;
    float m; // sum of subsystem masses
    
    orb_obj_node_t *obj;
    orb_sys_node_t *subsys;
};

orb_sys_t *gOrb_root_system;

orb_sys_t*
orbit_add_sys(  orb_sys_t *parent, const char *name, float radius, float w0)
{
    orb_sys_t *sys = malloc(sizeof(orb_sys_t));
    if (sys == NULL) goto malloc_failed;
    
    sys->world = dWorldCreate();
    sys->name = strdup(name);
    if (sys->name == NULL) goto malloc_failed;

    sys->parent = parent;
    if (parent) {
        orb_sys_node_t *subsysnode = malloc(sizeof(orb_sys_node_t));
        if (subsysnode == NULL) goto malloc_failed;
        
        subsysnode->next = parent->subsys;
        subsysnode->sys = sys;
        parent->subsys = subsysnode;
        sys->id = dBodyCreate(parent->world);
    } else {
        printf("root system %s added\n", name);
        // This is the root system
        gOrb_root_system = sys;
    }
    
    return sys;
    
malloc_failed:
    // we get here if we run out of memory
    if (sys) {
        dWorldDestroy(sys->world);
        free(sys->name);        
        free(sys);
    }
    return NULL;
}

orb_obj_t*
orbit_add_obj(orb_sys_t *sys, const char *name, float radius, float w0, float m)
{
    assert(sys != NULL);
    orb_obj_t *obj = malloc(sizeof(orb_obj_t));
    orb_obj_node_t *node = malloc(sizeof(orb_obj_node_t));
    if (obj == NULL || node == NULL) {
        free(obj);
        free(node);

        return NULL;
    }
    
    obj->name = strdup(name);
    
    node->obj = obj;
    obj->id = dBodyCreate(sys->world);

    // Insert as first object
    node->next = sys->obj;
    sys->obj = node;
        
    // Add mass to parent systems, note that this assumes that the orbital mass
    // is in the same unit
    obj->m = m;
    orb_sys_t *sp = sys;
    while (sp) {
        sp->m += m;
        sp = sp->parent;
    }
    
    return obj;
}


void
orbit_init_obj(orb_obj_t *obj)
{
    
}
// Initalises the necissary physical parameters for the simulation system
void
orbit_init_sys(orb_sys_t *sys)
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

void
orbit_clear(orb_sys_t *sys)
{
    dBodySetForce(sys->id, 0.0f, 0.0f, 0.0f);
    dBodySetTorque(sys->id, 0.0f, 0.0f, 0.0f);
    
    orb_obj_node_t *onode = sys->obj;
    while (onode) {
        dBodySetForce(onode->obj->id, 0.0f, 0.0f, 0.0f);
        dBodySetTorque(onode->obj->id, 0.0f, 0.0f, 0.0f);

        onode = onode->next;
    }
    
    orb_sys_node_t *snode = sys->subsys;
    while (snode) {
        orbit_clear(snode->sys);
        snode = snode->next;
    }
}

#define P_G 6.67428e-11 
void
orbit_step(orb_sys_t *sys, float stepsize)
{
    // Solves mutual force equations for all objects in the system, and then solves them
    // for each subsystem
    orb_obj_node_t *onode0 = sys->obj;
    while (onode0) {
        const dReal *ode_o0p = dBodyGetPosition (onode0->obj->id);
        vector_t o0p = v_set(ode_o0p[0], ode_o0p[1], ode_o0p[2], 0.0f);
        orb_obj_node_t *onode1 = onode0->next;
        while (onode1) {
            const dReal *ode_o1p = dBodyGetPosition (onode1->obj->id);
            vector_t o1p = v_set(ode_o1p[0], ode_o1p[1], ode_o1p[2], 0.0f);            
            vector_t dist = v_sub(o1p, o0p);

            scalar_t r12 = v_abs(dist);
            r12 = r12 * r12;
            vector_t f12 = v_s_mul(v_normalise(dist),
                                -P_G*onode0->obj->m * onode1->obj->m / r12);
            vector_t f21 = v_neg(f12);
            
            dBodyAddForce(onode0->obj->id, f12.x, f12.y, f12.z);
            dBodyAddForce(onode1->obj->id, f21.x, f21.y, f21.z);

            onode1 = onode1->next;
        }
        
        orb_sys_node_t *snode = sys->subsys;
        while (snode) {
            const dReal *ode_s1p = dBodyGetPosition (snode->sys->id);
            vector_t s1p = v_set(ode_s1p[0], ode_s1p[1], ode_s1p[2], 0.0f);            
            vector_t dist = v_sub(s1p, o0p);

            scalar_t r12 = v_abs(dist);
            r12 = r12 * r12;
            vector_t f12 = v_s_mul(v_normalise(dist),
                                -P_G*onode0->obj->m * snode->sys->m / r12);
            vector_t f21 = v_neg(f12);
            
            dBodyAddForce(onode0->obj->id, f21.x, f21.y, f21.z);
            dBodyAddForce(snode->sys->id, f12.x, f12.y, f12.z);

            snode = snode->next;
        }
                
        onode0 = onode0->next;
    }
    
    dWorldStep(sys->world, stepsize);
    
    orb_sys_node_t *snode = sys->subsys;
    while (snode) {
        orbit_step(snode->sys, stepsize);
        snode = snode->next;
    }
}

