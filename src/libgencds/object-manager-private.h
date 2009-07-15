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

#ifndef OBJECT_MANAGER_PRIVATE__H__
#define OBJECT_MANAGER_PRIVATE__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>




    /*! Context of classes, meta interfaces and instanciated objects
     * 
     * Om_context_t contains the state of a number of objects, classes and meta
     * interfaces. It is not intended for direct use, and should only be accessed
     * by the functions in this module.
     * 
     * Note that classes, objects and interfaces are stored in both lists and
     * dictionarys in order to provide fast individual access plus an iteratable
     * access option.
     * */
    struct _om_ctxt_t {
        hashtable_t *class_dict; /*!< Dictionary of all class objects */

        hashtable_t *object_dict; /*!< Dictionary of all objects */    
        /*! Dictionary of all objects, indexed by object pointers */
        hashtable_t *reverse_object_dict;    

        hashtable_t *interface_dict; /*!< Dictionary of all meta interfaces */
        list_t *interface_list; /*!< Iterable list of all meta interfaces */
    };


    struct _om_class_t {
        char *class_name; /*!< Name of the class, usually not used here */ 
        om_ctxt_t *ctxt; /*!< A pointer to the class' context */

        size_t object_size; /*!< Size of an instance of this class in bytes */

        hashtable_t *property_dict; /*!< A list of all properties */

        /*! All interfaces that the class implements to */
        hashtable_t *interface_dict;

        om_object_constructor_f create; /*!< Object constructor */
        om_object_destructor_f destroy; /*!< Object destructor */
        
        bool is_proxy; /*!< Whether the class is a proxy class (i.e. unmanaged) */
    };


    struct _om_object_t {
        om_class_t *cls; //!< Class pointer

        char *name; //!< Name of object
        void *data; //!< The object itself
    };


    struct _om_prop_t {
        char *name;

        om_prop_type_t type_code;

        union {
            struct {
                size_t length;
            } static_array;
            struct {
                struct _om_prop_t *length;
            } dynamic_array;
        } info;

        void *get, *set, *rd, *wr; // overloaded accessors
        ptrdiff_t offset; // offset from struct start
    };


    struct _om_meta_iface_t {
        char *name;
        hashtable_t *methods;
    };


    struct _om_iface_t {
        om_meta_iface_t *meta_interface;
        void *address;
    };

#ifdef __cplusplus
}
#endif

#endif /*! OBJECT_MANAGER_PRIVATE__H__ */
