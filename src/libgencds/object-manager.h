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


/*! \file object-manager.h
    \brief Public interface for the Open Orbit object system. A runtime system
        storing meta data about data structures, allowing for object
        introspection.
    
    The Open Orbit object system is one of the most important element in the
    application. All plug-in interfaces utilises this. It is infact impossible
    to write plugins without grasping the basics of the object manger.
    
    The object manager is there in order to allow introspection and a farly
    simple way to generate scripting interfaces during runtime (see the Python
    backend). There are a few concepts that one need to grasp when dealing with
    the system.
    
    A class is a description of an object, how the objects are created and how
    they are destroyed, what properties an object have and what interfaces it
    conforms to. Interfaces points to the set of methods that an object is
    guaranteed to conform to, this set of methods must conform to a certain
    interface specification called a meta interface.
    
    An object is a concrete instance of what is described by the class. The
    object will have properties. Properties are the data that an object posess.
    
    A meta interface is a global description of what methods an interface must
    implement.
    
    In order to put this in a C-perspective here: an object is a concrete
    representation of some struct. The properties are the regular data members
    in it and interfaces are structs of function pointers.
    
    Note that there are two kinds of functions in the object manager. Firstly,
    those that returns meta objects, that describe the concrete objects.
    Secondly there are functions that return and set concrete data. In order
    to be a bit more clear about this, there are functions that return both
    the meta representation of say an interface, i.e. an om_iface_t pointer
    and those that return the concrete interface representation, e.g.
    &iface_struct.
    
    There are limitations on what kind of data that an object can contain. At
    present, the only allowed data members are integers, floats and objects.
    The objects are always attached as pointers. These types can be stored in
    arrays both static sized arrays, and dynamically allocated ones (in the
    later case, a length property for that array must also be registered).
    
    If one want to add a struct as an object member, this is possible by
    registering the member variables one by one. It is not possible to add
    arrays of structs (in any simple way).
 */

#ifndef OBJECT_MANAGER__H__
#define OBJECT_MANAGER__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#include "hashtable.h"
#include "list.h"

typedef void* (*om_object_constructor_f)(void);
typedef void (*om_object_destructor_f)(void*);

typedef struct _om_ctxt_t om_ctxt_t;

typedef struct _om_class_t om_class_t;
typedef struct _om_object_t om_object_t;
typedef struct _om_prop_t om_prop_t;
typedef struct _om_meta_iface_t om_meta_iface_t;
typedef struct _om_iface_t om_iface_t;

/*!
 * Property type code
 *
 * The om_prop_type is a unique typecode, representing different kinds of
 * standard C-types. The type code is 16 bit long, and the 8 LSb are used
 * for the master type, and the 8 MSb are used to specialise the type.
 * 
 * The supported types include all standard int and float types, and the
 * fixed size (8, 16, 32 and 64) int types defined in stdint.h. Note that it is
 * also possible to specify a poperty as an object reference, and arrays of the
 * base types.
 *
 * Some type codes are illegal, it is for example not allowed to define a type
 * as an unsigned double, or to define a reference to an object, since this is
 * implicit in the object type.
 *
 * Example: unsigned int: OM_UNSIGNED | OM_INT
 * Example: uint16_t: OM_UNSIGNED | OM_INT16
 * Example: char[]: OM_ARRAY | OM_CHAR
 */
typedef uint16_t om_prop_type_t;

/* Simple types */
#define OM_CHAR     (0)
#define OM_SHORT    (1)
#define OM_INT      (2)
#define OM_LONG     (3)
#define OM_LONGLONG (4)
#define OM_INT8     (5)
#define OM_INT16    (6)
#define OM_INT32    (7)
#define OM_INT64    (8)


#define OM_FLOAT    (16)
#define OM_DOUBLE   (17)
#define OM_QUAD     (18) // TODO: Support quad precision floats

#define OM_OBJECT   (32)


/* Type specialisation */
#define OM_ARRAY    (1 << 8)
#define OM_REF      (1 << 9) // NOTE: Not supported at the moment
#define OM_UNSIGNED (1 << 10)  

/* Type simplivication */
#define OM_STRING   (OM_CHAR | OM_ARRAY)

int
om_save_ctxt(om_ctxt_t *ctxt); // save to memory

int
om_archive_ctxt(om_ctxt_t *ctxt); // save to disk

int
om_restore_ctxt(om_ctxt_t *ctxt); // restore ctxt

int
om_load_ctxt(om_ctxt_t *ctxt); // restore ctxt from disk

om_ctxt_t*
om_new_ctxt(void); // create new context

void
om_delete_ctxt(om_ctxt_t *ctxt); // Deletes a context and all the objects


om_class_t*
om_new_class(om_ctxt_t *ctxt, const char *class_name,
             om_object_constructor_f, om_object_destructor_f, size_t size);
void om_delete_class(om_class_t *cls);

om_object_t*
om_new_object(om_ctxt_t *ctxt, const char *class_name,
              const char *object_name);
void om_delete_object(om_object_t *obj);


om_meta_iface_t*
om_new_meta_interface(om_ctxt_t *ctxt, const char *iface_name);

void om_delete_meta_interface(om_meta_iface_t *iface);


int
om_reg_method(om_meta_iface_t *meta_iface, const char *method_name,
              ptrdiff_t offset);

om_prop_t*
om_reg_prop(om_class_t *class_object, const char *name,
            om_prop_type_t type, ptrdiff_t offset);

om_prop_t*
om_reg_static_array_prop(om_class_t *class_object, const char *name,
                         om_prop_type_t type, ptrdiff_t offset, size_t length);
            

int
om_reg_iface(om_class_t *class_object, const char *name, void *iface_addr);

const char*
om_get_object_name(const om_object_t *obj);
void*
om_get_object_data(const om_object_t *obj);


om_object_t*
om_get_object_from_ptr(const om_ctxt_t *ctxt, void *address);

bool
om_object_is_class(const om_object_t *obj,
                   const char *class_name);


bool
om_conforms_to_iface(const om_object_t *obj,
                     const char *iface);

om_class_t*
om_get_class(const om_ctxt_t *ctxt, const char *class_name);

om_object_t*
om_get_object(const om_ctxt_t *ctxt, const char *object_name);

om_meta_iface_t*
om_get_meta_iface(const om_ctxt_t *ctxt, const char *iface_name);

om_iface_t*
om_get_iface(const om_class_t *cls, const char *iface_name);

om_prop_t*
om_get_prop(const om_class_t *cls, const char *prop_name);

om_class_t*
om_get_object_class(const om_object_t *obj);

void*
om_get_concrete_obj(const om_ctxt_t *ctxt, const char *object_name);

void*
om_get_concrete_method(const om_class_t *cls, const char *iface_name,
                       const char *method_name);

void*
om_get_concrete_iface(const om_class_t *obj, const char *iface_name);

void*
om_get_concrete_prop(const om_object_t *obj, const char *prop_name);

#define OM_ACCESSOR_PAIR_DEF(T, N) \
    T om_get_ ## N ## _prop(const om_object_t *obj, const char *prop_name);    \
    void om_set_ ## N ## _prop(om_object_t *obj, const char *prop_name, T val);\
    T om_get_ ## N ## _idx_prop(const om_object_t *obj, const char *prop_name, unsigned int idx);    \
    void om_set_ ## N ## _idx_prop(om_object_t *obj, const char *prop_name, unsigned int idx, T val);
    
OM_ACCESSOR_PAIR_DEF(char, char);

OM_ACCESSOR_PAIR_DEF(short, short);
OM_ACCESSOR_PAIR_DEF(unsigned short, ushort);

OM_ACCESSOR_PAIR_DEF(int, int);
OM_ACCESSOR_PAIR_DEF(unsigned int, uint);

OM_ACCESSOR_PAIR_DEF(long, long);
OM_ACCESSOR_PAIR_DEF(unsigned long, ulong);

OM_ACCESSOR_PAIR_DEF(float, float);
OM_ACCESSOR_PAIR_DEF(double, double);

OM_ACCESSOR_PAIR_DEF(int8_t, int8);
OM_ACCESSOR_PAIR_DEF(uint8_t, uint8);

OM_ACCESSOR_PAIR_DEF(int16_t, int16);
OM_ACCESSOR_PAIR_DEF(uint16_t, uint16);

OM_ACCESSOR_PAIR_DEF(int32_t, int32);
OM_ACCESSOR_PAIR_DEF(uint32_t, uint32);

OM_ACCESSOR_PAIR_DEF(int64_t, int64);
OM_ACCESSOR_PAIR_DEF(uint64_t, uint64);

OM_ACCESSOR_PAIR_DEF(void*, object);


#ifdef __cplusplus
}
#endif
    
#endif
