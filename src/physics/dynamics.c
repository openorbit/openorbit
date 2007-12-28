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

#include "math/constants.h"
#include "math/linalg.h"
#include "math/quaternions.h"


void
ph_init(om_ctxt_t *ctxt)
{
    om_class_t *obj_cls = om_new_proxy_class(ctxt, "ph_obj");
    //bool is_valid;
    //bool is_enabled;
    
    om_reg_prop(obj_cls, "valid", OM_BOOL, offsetof(ph_obj_t, is_valid));
    om_reg_prop(obj_cls, "enabled", OM_BOOL, offsetof(ph_obj_t, is_enabled));
    
    om_reg_prop(obj_cls, "m", OM_FLOAT, offsetof(ph_obj_t, m));
    om_reg_prop(obj_cls, "i", OM_FLOAT, offsetof(ph_obj_t, i));
    om_reg_static_array_prop(obj_cls, "r", OM_FLOAT, offsetof(ph_obj_t, r), 4);
    om_reg_static_array_prop(obj_cls, "v", OM_FLOAT, offsetof(ph_obj_t, v), 4);
    om_reg_static_array_prop(obj_cls, "f_acc", OM_FLOAT, offsetof(ph_obj_t, f_acc), 4);
    om_reg_static_array_prop(obj_cls, "t_acc", OM_FLOAT, offsetof(ph_obj_t, t_acc), 4);
    om_reg_static_array_prop(obj_cls, "q", OM_FLOAT, offsetof(ph_obj_t, q), 4);
    om_reg_static_array_prop(obj_cls, "w", OM_FLOAT, offsetof(ph_obj_t, w), 4);
    
}

ph_sys_t*
ph_new_system(ph_sys_t *parent, size_t obj_count)
{
    ph_sys_t *sys;
    
    if (parent && (parent->child_alen <= parent->child_count)) {
        warnx("ph: sys full");
        return NULL;
    }
    
    sys = malloc(sizeof(ph_sys_t) + sizeof(ph_obj_t)*obj_count);
    
    if (sys == NULL) {
        return NULL;
    }
    
    memset(sys, 0, sizeof(ph_sys_t) + sizeof(ph_obj_t)*obj_count);
    
    sys->free_obj_list = list_new();
    
    if (sys->free_obj_list == NULL) {
        free(sys);
        return NULL;
    }
    
    for (int i = 0 ; i < obj_count ; i ++) {
        list_append(sys->free_obj_list, &sys->obj[i]);
    }
    
    sys->alloc_size = obj_count;
    
    if (parent) {
        parent->children[parent->child_count] = sys;
        parent->child_count ++;
    }
    
    return sys;
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

    vector_t cm = {.s.x = 0.0, .s.y = 0.0, .s.z = 0.0, .s.w = 0.0};
    vector_t cm_tmp;
    scalar_t m = S_CONST(0.0);
    vector_t g; // gravity
    // for all subsystems
    for (int i = 0 ; i < sys->child_count ; i ++) {
        ph_apply_gravity(sys->children[i], g);
        ph_step(sys->children[i], step);
    }
    
    // for all enabled objects update their positions, sum together their
    // masses, and their average cm
    for (int i = 0 ; i < sys->alloc_size ; i ++) {
        vector_t a, at, v, vt, r, f;
        
        if (sys->obj[i].is_enabled) {
            // comp total force and acceleration
            V_ADD(f, sys->g, sys->obj[i].f_acc);
            V_S_DIV(a, f, sys->obj[i].m);
            
            // comp new velocity
            V_S_MUL(at, a, step);
            V_ADD(v, sys->obj[i].v, at);
            
            // comp new position
            V_S_MUL(vt, sys->obj[i].v, step);
            V_ADD(r, sys->obj[i].r, vt);
            
            // apply changes
            V_CPY(sys->obj[i].r, r);
            V_CPY(sys->obj[i].v, v);
            
            // rotate object, we should probably optimise this
            vector_t delta_w;
            V_S_DIV(delta_w, sys->obj[i].t_acc, sys->obj[i].i);
            quaternion_t qr0, qr1, q0, q1, q2;
            Q_ROT_X(q0, delta_w.s.x);
            Q_ROT_Y(q1, delta_w.s.y);
            Q_ROT_Z(q2, delta_w.s.z);
            Q_MUL(qr0, q0, q1);
            Q_MUL(qr1, qr0, q2);
            Q_MUL(qr0, sys->obj[i].w, qr1);
            V_CPY(sys->obj[i].w, qr0);
            
            m += sys->obj[i].m;
            V_S_MUL(cm_tmp, sys->obj[i].r, sys->obj[i].m);
            V_ADD(cm, cm, cm_tmp);
        }
    }
    
    V_CPY(sys->r, cm);
    sys->m = m;
}


void
ph_apply_gravity(ph_sys_t *sys, vector_t g)
{
    assert(sys != NULL);
    
    V_ADD(sys->g, sys->g, g);    
}

void
ph_apply_force(ph_obj_t *obj, vector_t f)
{
    assert(obj != NULL);
    
    V_ADD(obj->f_acc, obj->f_acc, f);
}

void
ph_apply_force_at_pos(ph_obj_t *obj, vector_t pos, vector_t f)
{
    assert(obj != NULL);
    
    vector_t t, r;
    
    /* Compute torque (wrt the cm).
        NOTE: This is not entirely correct as it violates energy conversation
        principles. */
    V_SUB(r, pos, obj->r);    
    V_CROSS(t, r, f);
    
    V_ADD(obj->f_acc, obj->f_acc, f); // force
    V_ADD(obj->t_acc, obj->t_acc, t); // torque
}


void
ph_apply_force_relative(ph_obj_t *obj, vector_t pos, vector_t f)
{
    assert(obj != NULL);
    
    // Need to rotate f and pos before applying
    vector_t pt, ft, t;
    matrix_t rm;
    
    Q_M_CONVERT(rm, obj->q);
    
    M_V_MUL(pt, rm, pos);
    M_V_MUL(ft, rm, f);
    
    // Torque
    V_CROSS(t, pt, ft);
    
    V_ADD(obj->f_acc, obj->f_acc, ft); // force
    V_ADD(obj->t_acc, obj->t_acc, t); // torque

}


void
ph_migrate_object(ph_sys_t *dst_sys, ph_sys_t *src_sys, ph_obj_t *obj)
{
    assert(dst_sys != NULL);
    assert(src_sys != NULL);
    assert(obj != NULL);
    
    ph_obj_t *dst_obj;
    
    dst_obj = (ph_obj_t*) list_remove_first(dst_sys->free_obj_list);
    
    if (dst_obj) {
        unsigned obj_id = (unsigned)(obj - offsetof(ph_sys_t, obj))
                          / (unsigned)sizeof(ph_obj_t);
        (*dst_obj) = *obj;
        obj->is_valid = false;
        
        list_insert(src_sys->free_obj_list, obj);
    } else {
        fprintf(stderr, "No space in destination system, cannot migrate object");
    }
    
}
