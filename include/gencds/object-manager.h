/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
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
    
    In normal circumstances, memory management is carried out by the om_
    functions, however in some cases this is not desirable (e.g. out of
    performance reasons, or that an existing code base cannot be moved to use
    the object manager). For these situations one can use proxy objects, these
    are instansiated from proxy classes and allows the user full introspection
    of the object, but not to allocate instances of it. One example would be
    a simulation where structs get packed into arrays in order to better utilise
    the CPUs cache, it would in this case be possible to register a proxy class
    and supply the object pointers manualy.
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
 * base types. Further the libgencds datastructures list_t, hashtable_t and
 * heap_t are also supported.
 *
 * Some type codes are illegal, it is for example not allowed to define a type
 * as an unsigned double, or to define a reference to an object, since this is
 * implicit in the object type.
 *
 * Example: unsigned int: OM_UNSIGNED | OM_INT
 * Example: uint16_t: OM_UNSIGNED | OM_INT16
 * Example: char[]: OM_ARRAY | OM_CHAR
 * 
 * If you make derrived types (whose basetype may depend on preprocessor
 * definitions), it is recommended that you define additional typecodes based
 * on the one below. For example, assume we have a scalar_t type defined in the
 * module foo as either a float or a double, you would define another typcode
 * called FOO_OM_SCALAR that is equal to either OM_FLOAT OR OM_DOUBLE.
 */
typedef uint16_t om_prop_type_t;
struct typecode {
    unsigned proxy:1;
    unsigned array:2;
    unsigned ref:1;
    unsigned base:8;
};

#define GET_SET_TYPES(NAME, TYPE)                                   \
    typedef void (*om_set_##NAME##_f)(void*, TYPE);                 \
    typedef TYPE (*om_get_##NAME##_f)(void*);                       \
    typedef void (*om_idx_set_##NAME##_f)(void*, unsigned, TYPE);   \
    typedef TYPE (*om_idx_get_##NAME##_f)(void*, unsigned);
    
// Standard discrete C types
GET_SET_TYPES(char, char)
GET_SET_TYPES(bool, bool)
GET_SET_TYPES(str, char*)
GET_SET_TYPES(int, int)
GET_SET_TYPES(uint, unsigned int)
GET_SET_TYPES(short, short)
GET_SET_TYPES(ushort, unsigned short)
GET_SET_TYPES(long, long)
GET_SET_TYPES(ulong, unsigned long)

// Generic pointer
GET_SET_TYPES(object, void*)

// Stdint fixed length types
GET_SET_TYPES(uint8, uint8_t)
GET_SET_TYPES(uint16, uint16_t)
GET_SET_TYPES(uint32, uint32_t)
GET_SET_TYPES(uint64, uint64_t)
GET_SET_TYPES(int8, int8_t)
GET_SET_TYPES(int16, int16_t)
GET_SET_TYPES(int32, int32_t)
GET_SET_TYPES(int64, int64_t)

// FP types
GET_SET_TYPES(float, float)
GET_SET_TYPES(double, double)


/* Simple types */
/* integral types */
#define OM_CHAR         (0)
#define OM_SHORT        (1)
#define OM_INT          (2)
#define OM_LONG         (3)
#define OM_LONGLONG     (4)
#define OM_INT8         (5)
#define OM_INT16        (6)
#define OM_INT32        (7)
#define OM_INT64        (8)
#define OM_BOOL         (9)

/* real types */
#define OM_FLOAT        (16)
#define OM_DOUBLE       (17)
#define OM_QUAD         (18) // TODO: Support quad precision floats

/* om special types */
#define OM_OBJECT       (32)
#define OM_HASHTABLE    (33) 
#define OM_HEAP         (34)
#define OM_LIST         (35)


/* Type specialisation */
#define OM_ARRAY    (1 << 8)
#define OM_REF      (1 << 9)
#define OM_UNSIGNED (1 << 10)  
#define OM_PROXY    (1 << 11) // TODO: shoud classes or objects be proxies

/* Type simplification */
#define OM_STRING   (OM_CHAR | OM_ARRAY)

/*! Saves all objects in the context to memory 
 
    When saving the context all the objects are serialised and placed in a
    compacted in memory database. This database only keep the object data, all
    other data (class and meta information) is ignored and assumed to be static.
    
    \param ctxt The context to save
    \return The serialised data of the context.
 */
int om_save_ctxt(om_ctxt_t *ctxt); // save to memory
int om_archive_ctxt(om_ctxt_t *ctxt, const char *fname); // save to disk
int om_restore_ctxt(om_ctxt_t *ctxt); // restore ctxt
int om_load_ctxt(om_ctxt_t *ctxt, const char *fname); // restore ctxt from disk

om_ctxt_t* om_new_ctxt(void); // create new context

void om_delete_ctxt(om_ctxt_t *ctxt); // Deletes a context and all the objects

/*! Om_new_class creates a new class object in the given context
 *  
 *  \param ctxt The object manager context to create the class in
 *  \param class_name The name of the class to be created
 *  \param constr The object constructor function
 *  \param destr The object destructor function (may be NULL)
 *  \param size The size of the C-struct representing the object. 
 */
om_class_t* om_new_class(om_ctxt_t *ctxt, const char *class_name,
                         om_object_constructor_f constr,
                         om_object_destructor_f destr,
                         size_t size);

om_class_t* om_new_proxy_class(om_ctxt_t *ctxt, const char *class_name);

                         
void om_delete_class(om_class_t *cls);

om_object_t* om_new_object(om_ctxt_t *ctxt, const char *class_name,
                           const char *object_name);
void om_delete_object(om_object_t *obj);

/*!
    Om_insert_proxy_obj will insert the object pointer so that it is available
    from the object manager for introspection.
    
    If the named object already exist and it is a proxy object, the proxy
    object will be replaced.
    
    \param ctxt The object manager context
    \param class_name The name of the class to which the object is supposed to
        belong.
    \param object_name The name of the new proxy object, if the name already
        exist and the existing object is a proxy object of the same class, the
        existing object will be replaced.
    \param obj_addr The address of the new proxy object.
*/
om_object_t* om_insert_proxy_obj(om_ctxt_t *ctxt, const char *class_name,
                                 const char *object_name, void *obj_addr);



om_meta_iface_t* om_new_meta_interface(om_ctxt_t *ctxt, const char *iface_name);
void om_delete_meta_interface(om_meta_iface_t *iface);


int om_reg_method(om_meta_iface_t *meta_iface, const char *method_name,
    const char *convention, ptrdiff_t offset);


om_prop_t* om_reg_prop(om_class_t *class_object, const char *name,
                       om_prop_type_t type, ptrdiff_t offset);

om_prop_t* om_reg_overloaded_prop(om_class_t *class_object, const char *name,
                                  om_prop_type_t type, ptrdiff_t offset,
                                  void *getter, void *setter, void *reader, void *writer);

om_prop_t* om_reg_static_array_prop(om_class_t *class_object, const char *name,
                                    om_prop_type_t type, ptrdiff_t offset,
                                    size_t length);

om_prop_t* om_reg_overloaded_static_array_prop(om_class_t *class_object, const char *name,
                                               om_prop_type_t type, ptrdiff_t offset,
                                               size_t length,
                                               void *getter, void *setter,
                                               void *reader, void *writer);


/*! Om_reg_dynamic_array_prop registers a dynamically allocated array property.
 *  A dynamic array property requires that the length of the array is tracked,
 * hence, the need to specify the property name of the length tracking variable.
 * \param class_object
 * \param name
 * \param type
 * \param offset
 * \param length_prop The name of the property containing the length of the
 *        array. This property must already be registered.
 */
om_prop_t*
om_reg_dynamic_array_prop(om_class_t *class_object, const char *name,
                          om_prop_type_t type, ptrdiff_t offset,
                          char *length_prop);

om_prop_t*
om_reg_overloaded_dynamic_array_prop(om_class_t *class_object, const char *name,
                                     om_prop_type_t type, ptrdiff_t offset,
                                     char *length_prop,
                                     void *getter, void *setter,
                                     void *reader, void *writer);

int om_reg_iface(om_class_t *class_object, const char *name, void *iface_addr);

const char* om_get_object_name(const om_object_t *obj);
void* om_get_object_data(const om_object_t *obj);


om_object_t* om_get_object_from_ptr(const om_ctxt_t *ctxt, void *address);

bool om_object_is_class(const om_object_t *obj, const char *class_name);
bool om_conforms_to_iface(const om_object_t *obj, const char *iface);

om_class_t* om_get_class(const om_ctxt_t *ctxt, const char *class_name);
om_object_t* om_get_object(const om_ctxt_t *ctxt, const char *object_name);
om_meta_iface_t* om_get_meta_iface(const om_ctxt_t *ctxt,
                                   const char *iface_name);

om_iface_t* om_get_iface(const om_class_t *cls, const char *iface_name);
om_prop_t* om_get_prop(const om_class_t *cls, const char *prop_name);

om_class_t* om_get_object_class(const om_object_t *obj);

void* om_get_concrete_obj(const om_ctxt_t *ctxt, const char *object_name);
void* om_get_concrete_method(const om_class_t *cls, const char *iface_name,
                             const char *method_name);
void* om_get_concrete_iface(const om_class_t *obj, const char *iface_name);
void* om_get_concrete_prop(const om_object_t *obj, const char *prop_name);

#define OM_ACCESSOR_PAIR_DEF(T, N) \
    T om_get_ ## N ## _prop(const om_object_t *obj, const char *prop_name);    \
    void om_set_ ## N ## _prop(om_object_t *obj, const char *prop_name, T val);\
    T om_get_ ## N ## _idx_prop(const om_object_t *obj, const char *prop_name, unsigned int idx);    \
    void om_set_ ## N ## _idx_prop(om_object_t *obj, const char *prop_name, unsigned int idx, T val);
    
OM_ACCESSOR_PAIR_DEF(bool, bool);

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


// Object manager predefined interfaces
typedef struct {
    void (*save)(void*);
    void (*restore)(void*);
} om_serialisation_if;
#define OM_SERIALISATION_IF_KEY "om.serialisation"


#ifdef __cplusplus
}
#endif
    
#endif
