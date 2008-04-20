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

/*  
    We will use a number of systems containing objects for the simulation
    engine. Each system will have a fixed list of objects on which we will
    iterate our solver. We will iterate over all objects in the system. Though
    some of the objects can be disabled. A system can have one gravitational
    centre and external gravitational sources (this is effectivelly a PIC
    solver).
 */ 

#ifndef _DYNAMICS_H_
#define _DYNAMICS_H_



#include <stdbool.h>

#include <gencds/object-manager.h>
#include <gencds/list.h>
#include <vmath/vmath.h>

#define PH_G 6.6732e-11 //Nm^2 kg^-2

// invariant: is_valid == false && is_enabled == true
typedef struct {
    bool is_valid;
    bool is_enabled;
    scalar_t m;       //!< mass of object in kg
    vector_t r;       //!< position of the object in the local reference frame
    vector_t v;       //!< velocity of the object in m/s
    vector_t f_acc;   //!< force accumulator in N
    vector_t t_acc;   //!< torque accumulator in Nm
    quaternion_t q;   //!< rotation quat
    quaternion_t w;   //!< rotational speed quat
    matrix_t I;       //!< Inertial tensor
    matrix_t I_rep;   //!< Inverse of inertial tensor
} ph_obj_t;


/*! A system of several objects

    Note that each system has an object seen as the main system object that is
    generating gravity and other nice things.
*/
typedef struct _ph_sys_t {
    vector_t r; //!< position of the system in parent world coords
    scalar_t radius; //!< Distance to orbiting point in parent units
    quaternion_t w; //!< Rotational quaternion with respect to starting point
    quaternion_t p; //!< Preccession quaternion
    
    scalar_t m; //!< mass of system in kg
    
    size_t child_alen;
    size_t child_count;
    struct _ph_sys_t **children;
    
    list_t *free_obj_list; //!< for quickly inserting objects
    size_t alloc_size; //!< size of obj array (in no of objects)
    
    ph_obj_t **obj; //! List of current objects in the system

} ph_sys_t;



/*! Creates a new (almost) isolated physics system
    
    The new_sys function, allocates a new physics system with a maximum object
    count of obj_count objects.
    
    \param parent The parent system where the created system is to be inserted.
        If the param is NULL, then a root system is created.
    \param obj_count The initial size of the object array in the system.
    \return The new physics system.
 */
ph_sys_t* ph_new_sys(ph_sys_t *parent, size_t obj_count);

/*! Creates a new object in the given system
  
  \param sys The system in which the object is placed initially.
  \param name The name of the object for the object manager.
  \return The new object
*/
ph_obj_t* ph_new_obj(ph_sys_t *sys, const char *name);

/*! Does a time step in the simulation of the given physics system
    
    \param sys 
    \param step The time in seconds, should be sufficiently small.
 */
void ph_step(ph_sys_t *sys, scalar_t step);

/*! Applies an external global force on the entire system

    Note, this does not apply the gravity on any individual objects, only the
    system (i.e. for simulating orbits).
    
    \param sys
    \param g Gravity vector in Nm
 */
void ph_apply_gravity(ph_sys_t *sys, vector_t g);
void ph_apply_force(ph_obj_t *obj, vector_t f);
void ph_apply_force_at_pos(ph_obj_t *obj, vector_t pos, vector_t f);
void ph_apply_force_relative(ph_obj_t *obj, vector_t pos, vector_t f);

/*!
    Moves an object from one system to another
    
    The migrate_object function performs the migration of on object from one
    system to another. This behaviour is most useful when the simulation
    environment consist of several (more or less) isolated systems that need to
    be interacting in one way or the other.
    
    Note that an object in one system cannot directly interact with objects
    from other systems. This means that all closly coupled objects must be in
    the same system. System can still interact however, as they yeald a mass and
    a center of mass. This mean that gravity can be simulatend with a pic-method
    using several systems. In that case objects must be able to migrate to other
    systems. It can be expected that object migration is rare, and thus the
    system is optimised to keep in-system calculations fast at the expense of
    migration cost.
    
    Migration is basically the feat of copying a pointer and translating the
    object coordinates to the target systems local coordinate space.
    
    \param dst_sys
    \param src_sys
    \param obj
*/
void ph_migrate_object(ph_sys_t *dst_sys, ph_sys_t *src_sys, ph_obj_t *obj);

/*!
    Initialise the physics subsystem
    
    Initialises the physics system by creating the relevant classes in the
    object manager. It will for example insert the ph_obj_t class in the om.
    
    \param ctxt Tho object manager context where the classes are to be created.
 */ 
void ph_init(om_ctxt_t *ctxt);
#endif /* ! _DYNAMICS_H_ */
