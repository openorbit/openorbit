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
    created by the Initial Developer are Copyright (C) 2006 the
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

#include "dynamics.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <err.h>

#include <gencds/object-manager.h>
#include <vmath/vmath.h>
#include <ode/ode.h>



static om_ctxt_t *_ctxt; // Context pointer


struct _ph_obj_t {
    char *name;
    dBodyID id;
};

typedef struct _ph_obj_node_t {
    ph_obj_t *obj;
    struct _ph_obj_node_t *next;
} ph_obj_node_t;

struct _ph_sys_t {
    dWorldID world;
    
    ph_obj_node_t *obj;
    
    size_t subsys_count;
    ph_sys_t* subsys[];
};



ph_obj_t*
new_body(ph_sys_t *sys, const char *name)
{
    assert(sys != NULL);
    ph_obj_t *obj = malloc(sizeof(ph_obj_t));
    ph_obj_node_t *node = malloc(sizeof(ph_obj_node_t));
    if (obj == NULL || node == NULL) {
        free(obj);
        free(node);
        
        return NULL;
    }
    
    obj->name = strdup(name);
    
    node->obj = obj;
    obj->id = dBodyCreate(sys->world);
    
    node->next = sys->obj;
    sys->obj = node;
    
    return obj;
}

ph_sys_t*
new_sys(ph_sys_t *parent, size_t subsys_count)
{
    ph_sys_t *sys = malloc(sizeof(ph_sys_t) + subsys_count * sizeof(ph_sys_t*));

    memset(sys->subsys, 0, subsys_count*sizeof(ph_sys_t*));

    sys->world = dWorldCreate();
    sys->subsys_count = subsys_count;
    
    if (parent) {
        for (int i = 0 ; i < parent->subsys_count ; i ++) {
            if (parent->subsys[i] == NULL) {
                parent->subsys[i] = sys;
                return sys;
            }
        }
        
        // we get here if the parent is full
        dWorldDestroy(sys->world);
        free(sys);
        return NULL;
    }
    return sys; 
}



void*
ph_creat_obj(void)
{
    ph_obj_t *obj = malloc(sizeof(ph_obj_t));
    memset(obj, 0, sizeof(obj));
    return obj;
}

void
ph_delete_obj(void *obj)
{
    free(obj);
}

static scalar_t
obj_get_m(void *obj)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    dMass mass;
    dBodyGetMass(o->id, &mass);
    return (scalar_t)mass.mass;
}

static void
obj_set_m(void *obj, scalar_t m)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    dMass mass;
    dBodyGetMass(o->id, &mass);
    mass.mass = (dReal)m;
    dBodySetMass(o->id, &mass);
}

static void
obj_get_I(void *obj, matrix_t *I)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    dMass mass;
    dBodyGetMass(o->id, &mass);

//    *I 
}

static void
obj_set_I(void *obj, matrix_t *I)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    dMass mass;
    dBodyGetMass(o->id, &mass);
    
    mass.I[0] = I->a[0][0];
    mass.I[1] = I->a[0][1];
    mass.I[2] = I->a[0][2];
    
    mass.I[3] = I->a[1][0];
    mass.I[4] = I->a[1][1];
    mass.I[5] = I->a[1][2];
    
    mass.I[6] = I->a[2][0];
    mass.I[7] = I->a[2][1];
    mass.I[8] = I->a[2][2];
    
    dBodySetMass(o->id, &mass);
}

static vector_t
obj_get_r(void *obj)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    const dReal *p = dBodyGetPosition(o->id);
    
    vector_t v = {.a = {p[0],p[1],p[2],0.0f}};
    return v;
}

static void
obj_set_r(void *obj, vector_t r)
{
    ph_obj_t *o = (ph_obj_t*)obj;

    dBodySetPosition(o->id, r.x, r.y, r.z);
}




void
ph_init(om_ctxt_t *ctxt)
{
    _ctxt = ctxt;
    om_class_t *obj_cls = om_new_class(ctxt, "ph_obj",
                                       ph_creat_obj, ph_delete_obj,
                                       sizeof(ph_obj_t));
                                       
    om_reg_overloaded_prop(obj_cls, "m", OM_FLOAT, 0, obj_get_m, obj_set_m,
                           NULL, NULL);
    //om_reg_prop(obj_cls, "i", OM_FLOAT, offsetof(ph_obj_t, i));
    om_reg_static_array_prop(obj_cls, "r", OM_FLOAT, 0, 4);
    om_reg_static_array_prop(obj_cls, "v", OM_FLOAT, 0, 4);
    om_reg_static_array_prop(obj_cls, "f_acc", OM_FLOAT,
                             0, 4);
    om_reg_static_array_prop(obj_cls, "t_acc", OM_FLOAT,
                             0, 4);
    om_reg_static_array_prop(obj_cls, "q", OM_FLOAT, 0, 4);
    om_reg_static_array_prop(obj_cls, "w", OM_FLOAT, 0, 4);
    om_reg_static_array_prop(obj_cls, "I", OM_FLOAT, 0, 16);
    
    
    om_class_t *sys_cls = om_new_proxy_class(ctxt, "ph_sys");
   // om_reg_prop(sys_cls, "m", OM_FLOAT, offsetof(ph_sys_t, m));
  //  om_reg_static_array_prop(obj_cls, "r", OM_FLOAT, offsetof(ph_sys_t, r), 4);
   // om_reg_static_array_prop(obj_cls, "g", OM_FLOAT, offsetof(ph_sys_t, g), 4);
}


ph_obj_t*
ph_new_obj(ph_sys_t *sys, const char *name)
{
    ph_obj_t *obj = (ph_obj_t*) om_new_object(_ctxt, "ph_obj", name);
    //ph_insert_obj(sys, obj);
    
    return obj;
}

// Euler step
// v(t+dt) = v(t) + a(t)dt
// r(t+dt) = r(t) + v(t)dt
// Iw = N
void
ph_step(ph_sys_t *sys, scalar_t step)
{
    assert(sys != NULL);
    assert(step > S_CONST(0.0));
    
    for (int i = 0 ; i < sys->subsys_count ; i ++) {
        if (sys->subsys[i] == NULL) break;
        ph_step(sys->subsys[i], step);
    }
    
    dWorldStep(sys->world, step);
}


void
ph_apply_gravity(ph_sys_t *sys, vector_t g)
{
    assert(sys != NULL);
    
//    V_ADD(sys->g, sys->g, g);    
}

void
ph_apply_force(ph_obj_t *obj, vector_t f)
{
    assert(obj != NULL);
    
    dBodyAddForce(obj->id, f.x, f.y, f.z);
}

void
ph_apply_force_at_pos(ph_obj_t *obj, vector_t pos, vector_t f)
{
    assert(obj != NULL);
        
    dBodyAddRelForceAtPos(obj->id, f.x, f.y, f.z, pos.x, pos.y, pos.z);
}


void
ph_apply_force_relative(ph_obj_t *obj, vector_t f)
{
    assert(obj != NULL);

    dBodyAddRelForce(obj->id, f.x, f.y, f.z);
}


void
ph_migrate_object(ph_sys_t *dst_sys, ph_sys_t *src_sys, ph_obj_t *obj)
{
    assert(dst_sys != NULL);
    assert(src_sys != NULL);
    assert(obj != NULL);    
}


void
ph_set_mass(ph_obj_t *obj, scalar_t m) {
    assert(obj != NULL);
    //obj->m = m;
}

void
ph_reduce_mass(ph_obj_t *obj, scalar_t dm)
{
    assert(obj != NULL);
    
    //obj->m -= dm;
}

bool
ph_reduce_mass_min(ph_obj_t *obj, scalar_t dm, scalar_t min)
{
    assert(obj != NULL);
    
    //if (obj->m - dm < min) {
    //    obj->m = min;
    //    return false;
    //}
    //obj->m -= dm;
    return true;
}

void
ph_increase_mass(ph_obj_t *obj, scalar_t dm)
{
    assert(obj != NULL);
    
    //obj->m += dm;
}

void
ph_set_inertial_tensor(ph_obj_t *obj, matrix_t *new_I)
{
    assert(obj != NULL);
    assert(new_I != NULL);
    
    
    //obj->I = *new_I; // set the tensor itself
    //obj->I_rep = m_inv(new_I); // set the inverse of the tensor as well
}
