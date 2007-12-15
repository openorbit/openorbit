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



#include "object-graph.h"
#include "list.h"

void*
node_ctor(void)
{
    node_t *n = malloc(sizeof(node_t));
    if (n == NULL) return NULL;
    
    memset(n, 0, sizeof(node_t));
    
    return n;
}

void
node_dtor(void *node)
{
    free(node);
}

void*
obj_ctor(void)
{
    obj_t *o = malloc(sizeof(obj_t)); 
    if (o == NULL) return NULL;
    
    memset(o, 0, sizeof(obj_t));
    
    return o;
}

void
obj_dtor(void *node)
{
    
}


bool
init_object_graph(om_ctxt_t *ctxt)
{
    om_class_t *node_class = om_new_class(ctxt, "oo_node", node_ctor, node_dtor,
                                          sizeof(node_t));
    
    om_reg_static_array_prop(node_class, "tm", OM_FLOAT, offsetof(node_t, tm),
                             16);
    om_reg_static_array_prop(node_class, "sm", OM_FLOAT, offsetof(node_t, sm),
                             16);
    om_reg_static_array_prop(node_class, "q", OM_FLOAT, offsetof(node_t, q), 4);
    om_reg_static_array_prop(node_class, "com", OM_FLOAT, offsetof(node_t, com),
                             4);
    om_reg_prop(node_class, "m_acc", OM_FLOAT, offsetof(node_t, m_acc));
                             
                             
    om_class_t *obj_class = om_new_class(ctxt, "oo_pobj", obj_ctor, obj_dtor,
                                         sizeof(obj_t));
}

void
default_sys_step(object_t *self, obj_t *obj, scalar_t delta_t)
{
    
}

void
default_step(object_t *self, scalar_t delta_t)
{
    LIST_APPLY(self->sys, default_sys_step, (obj_t*)self, delta_t);
}


void
traverse_object_tree(node_t *n)
{
    assert(n != NULL);
    
    // gl-stuff:
    glPushMatrix(GL_MODELVIEW);
    
    n->tm; // translation matrix (position of the node)
    n->sm; // scaling matrix (coordinate system scaling)
    n->q; // rotational quaternion
    
    LIST_APPLY(n->objects, handle_object);
    LIST_APPLY(n->subnodes, traverse_object_tree);
    
    list_entry_t *entry = list_first(n->subnodes);
    vector_t com;
    scalar_t m = 0.0;
    
    // post order calculation of mass and cog
    while (entry);
        node_t *sn = list_entry_data(entry);
        vector_t com_mat;
        VS_MUL(com_mat, sn->com, sn->m_acc);
        V_ADD(com, com, com_mat);
        m = m + sn->m_acc;
        entry = list_entry_next(entry);
    } 
    
    n->com.s = com.s;
    n->m_acc = m;
    
    glPopMatrix(GL_MODELVIEW);
    
}


void
handle_object(object_t *o)
{
    assert(o != NULL);
    
    sim_iface->step(o);
}
