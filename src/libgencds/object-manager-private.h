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
        //list_t *class_list; /*!< Iterable list of all classes */

        hashtable_t *object_dict; /*!< Dictionary of all objects */    
        /*! Dictionary of all objects, indexed by object pointers */
        hashtable_t *reverse_object_dict;    
        //list_t *object_list; /*!< Iterable list of all objects */

        hashtable_t *interface_dict; /*!< Dictionary of all meta interfaces */
        list_t *interface_list; /*!< Iterable list of all meta interfaces */
    };


    struct _om_class_t {
        char *class_name; /*!< Name of the class, usually not used here */ 
        om_ctxt_t *ctxt; /*!< A pointer to the class' context */

        size_t object_size; /*!< Size of an instance of this class in bytes */

        hashtable_t *property_dict; /*!< A list of all properties */
        //list_t *property_list;

        /*! All interfaces that the class implements to */
        hashtable_t *interface_dict;
        //list_t *interface_list; // TODO: Verify that we need this

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
