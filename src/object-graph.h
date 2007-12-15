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


 
 
#ifndef _OBJECT_GRAPH_H_
#define _OBJECT_GRAPH_H_

#include <stdbool.h>

#include "object-manager.h"
#include "list.h"
#include "physics/dynamics.h"

// OM-INTERFACE BEGIN
typedef struct {
    void (*step)(object_t *self, scalar_t delta_t);
    void (*paint)(object_t *self);
} sim_obj_iface;
// OM-INTERFACE END

// note that an obj_t should conform to the sim_obj interface
typedef struct {
    dynamics_t dyn; // dynamics information on object
    list_t *sys; // list of systems in the object that respond to sys_sim iface
} obj_t;


typedef struct {
    matrix_t tm; // translation matrix (position of the node)
    matrix_t sm; // scaling matrix (coordinate system scaling)
    quaternion_t q; // rotational quaternion
    vector_t com; // center of mass in parent coordinates
    scalar_t m_acc; // mass accumulator for gravity calculations (kg) 

    list_t *subnodes; // list of subnodes
    list_t *objects; // list of objects in this node
} node_t;

bool init_object_graph(om_ctxt_t *ctxt);
void default_step(object_t *self, scalar_t delta_t);

void traverse_object_tree(node_t *n);
void handle_object(object_t *n);


#endif /* _OBJECT_GRAPH_H_ */
