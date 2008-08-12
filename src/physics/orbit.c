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

struct _orb_obj_t {
    char *name;
    dBodyID id;
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
    dWorldID world;
    char *name;
    float m; // sum of subsystem masses
    
    orb_obj_node_t *obj;
    orb_sys_node_t *subsys;
};

static orb_sys_t *gOrb_root_system;

orb_sys_t*
orbit_add_sys(  orb_sys_t *parent, size_t subsys_count,
                const char *name, float radius, float w0, float inc)
{
    orb_sys_t *sys = malloc(sizeof(orb_sys_t) + subsys_count * sizeof(orb_sys_t*));

    memset(sys->subsys, 0, subsys_count*sizeof(orb_sys_t*));
    
    sys->world = dWorldCreate();
    sys->subsys_count = subsys_count;
    sys->name = strdup(name);

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
    } else {
        // This is the root system
        gOrb_root_system = sys;
    }
    
    return sys;
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

    node->next = sys->obj;
    sys->obj = node;
        
    // Add mass to parent systems, note that this assumes that the orbital mass is in the
    // same unit
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

