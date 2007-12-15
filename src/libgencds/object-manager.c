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


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "object-manager.h"
#include "object-manager-private.h"

#include "hashtable.h"
#include "list.h"

#define IS_POWER_OF_2(val) ((!((val) & ((val)-1))) && (val))



static void om_delete_prop_obj(om_prop_t *prop);
static void om_delete_iface_obj(om_iface_t *iface);

om_ctxt_t
*om_new_ctxt(void)
{
    om_ctxt_t *ctxt = malloc(sizeof(om_ctxt_t));
    if (!ctxt) {
        return NULL;
    }
    
    ctxt->class_dict = hashtable_new_with_str_keys(512);
    ctxt->class_list = list_new();
    ctxt->object_dict = hashtable_new_with_str_keys(1024);
    ctxt->object_list = list_new();
    ctxt->reverse_object_dict = hashtable_new_with_ptr_keys(1024);

    ctxt->interface_dict = hashtable_new_with_str_keys(1024);
    ctxt->interface_list = list_new();
    
    if ((!ctxt->class_dict) || (!ctxt->object_dict)
    || (!ctxt->interface_dict) || (!ctxt->class_list)
    || (!ctxt->object_list) || (!ctxt->interface_list)) {
        hashtable_delete(ctxt->class_dict);
        hashtable_delete(ctxt->object_dict);
        hashtable_delete(ctxt->reverse_object_dict);
        hashtable_delete(ctxt->interface_dict);
        
        list_delete(ctxt->class_list);
        list_delete(ctxt->object_list);
        list_delete(ctxt->interface_list);
        
        free(ctxt);
        return NULL;
    }
    
    return ctxt;
}

void
om_delete_ctxt(om_ctxt_t *ctxt)
{
    assert(ctxt != NULL);
    
    LIST_APPLY(ctxt->object_list, om_delete_object);
    LIST_APPLY(ctxt->class_list, om_delete_class);
    LIST_APPLY(ctxt->interface_list, om_delete_meta_interface);
    
    // Delete the top level data
    hashtable_delete(ctxt->class_dict);
    hashtable_delete(ctxt->object_dict);
    hashtable_delete(ctxt->interface_dict);
    hashtable_delete(ctxt->reverse_object_dict);

    
    list_delete(ctxt->class_list);
    list_delete(ctxt->object_list);
    list_delete(ctxt->interface_list);
    
    free(ctxt);
}
 
 
int
om_save_ctxt(om_ctxt_t *ctxt)
{
    return 1;
}

static void
_write_object(om_object_t *obj, FILE *fp)
{
    fprintf(fp, "%s : %s = {\n", obj->name, obj->cls->class_name);
    
    
    
    fprintf(fp, "}\n");
}

int
om_archive_ctxt(om_ctxt_t *ctxt, const char *fname)
{
    assert(fname != NULL);
    
    FILE *fp = fopen(fname, "r");
    
    if ( fp == NULL ) return 1;
    
    // For each object, write object to file
    LIST_APPLY(ctxt->object_list, _write_object, fp);
    return 1;
}
int
om_restore_ctxt(om_ctxt_t *ctxt)
{
    return 1;
}
int
om_load_ctxt(om_ctxt_t *ctxt)
{
    return 1;   
}


om_class_t*
om_new_class(om_ctxt_t *ctxt, const char *class_name,
             om_object_constructor_f constr,
             om_object_destructor_f dest, size_t size)
{
    /* Ensure preconditions */
    assert(ctxt != NULL);
    assert(class_name != NULL);
    assert(constr != NULL);
    assert(dest != NULL);
    assert(size > 0);
    
    om_class_t *class_obj = malloc(sizeof(om_class_t));
    if (! class_obj) {
        return NULL;
    }
    
    class_obj->class_name = strdup(class_name);
    class_obj->ctxt = ctxt;
    class_obj->create = constr;
    class_obj->destroy = dest;
    class_obj->object_size = size;
    
    class_obj->property_dict = hashtable_new_with_str_keys(32);
    class_obj->interface_dict = hashtable_new_with_str_keys(32);
    class_obj->property_list = list_new();
    class_obj->interface_list = list_new();
    
    if (!(class_obj->property_dict) || !(class_obj->interface_dict)
    || !(class_obj->property_list) || !(class_obj->interface_list)) {
        hashtable_delete(class_obj->property_dict);
        hashtable_delete(class_obj->interface_dict);
        list_delete(class_obj->property_list);
        list_delete(class_obj->interface_list);
    }
    
    hashtable_insert(ctxt->class_dict, class_name, class_obj);
    list_insert(ctxt->class_list, class_obj);
    return class_obj;
}

void
om_delete_class(om_class_t *cls)
{
    assert(cls != NULL);
    
    LIST_APPLY(cls->interface_list, om_delete_iface_obj);
    LIST_APPLY(cls->property_list, om_delete_prop_obj);
    list_delete(cls->interface_list);
    list_delete(cls->property_list);
    
    hashtable_delete(cls->interface_dict);
    hashtable_delete(cls->property_dict);
    
    free(cls->class_name);
    free(cls);
}

om_object_t*
om_new_object(om_ctxt_t *ctxt, const char *class_name,
              const char *object_name)
{
    assert(ctxt != NULL);
    assert(class_name != NULL);
    assert(object_name != NULL);
        
    om_class_t *class_object = om_get_class(ctxt, class_name);
    if (! class_object) return NULL;
    
    om_object_t *obj = malloc(sizeof(om_object_t));
    
    obj->cls = class_object;
    obj->name = strdup(object_name);
    obj->data = class_object->create();

    
    hashtable_insert(ctxt->object_dict, object_name, obj);   
    list_append(ctxt->object_list, obj);   
    obj->list_entry = list_last(ctxt->object_list);    
    hashtable_insert(ctxt->reverse_object_dict, obj->data, obj);   
    
    
    return obj;
}

void
om_delete_object(om_object_t *obj)
{
    assert(obj != NULL);
    
    hashtable_remove(obj->cls->ctxt->object_dict, obj->name);
    hashtable_remove(obj->cls->ctxt->reverse_object_dict, obj->data);
    list_remove_entry(obj->cls->ctxt->object_list, obj->list_entry);
    obj->list_entry = NULL;
    
    // Delete object
    obj->cls->destroy(obj->data);
    
    free(obj->name);
    free(obj);
}

om_meta_iface_t*
om_new_meta_interface(om_ctxt_t *ctxt, const char *iface_name)
{
    assert(ctxt != NULL);
    assert(iface_name != NULL);
    
    om_meta_iface_t *mi = malloc(sizeof(om_meta_iface_t));
    
    if (mi == NULL) return NULL;
    
    mi->methods = hashtable_new_with_str_keys(32);
    if (mi->methods == NULL) {
        free(mi);
        return NULL;
    }
    
    mi->name = strdup(iface_name);
    
    hashtable_insert(ctxt->interface_dict, iface_name, mi);
    list_append(ctxt->interface_list, mi);

    return mi;
}

// TODO: Delete from list
void
om_delete_meta_interface(om_meta_iface_t *iface)
{
    assert(iface != NULL);
    
    hashtable_delete(iface->methods);
    free(iface->name);
    free(iface);
}

int
om_reg_method(om_meta_iface_t *meta_iface, const char *method_name,
              ptrdiff_t offset)
{
    assert(meta_iface != NULL);
    assert(method_name != NULL);
    
    hashtable_insert(meta_iface->methods, method_name, (void*)offset);
}

/* TODO: Support complex types (arrays, objects, refs, et.c.) */
om_prop_t*
om_reg_prop(om_class_t *class_object, const char *name,
            om_prop_type_t type, ptrdiff_t offset)
{
    assert(class_object != NULL);
    assert(name != NULL);
    assert(offset >= 0);
    
    assert(IS_POWER_OF_2(type & 0x00ff)); // One and only one base type
    
    // Currently allowed base types
    assert(type & (OM_CHAR|OM_SHORT|OM_INT|OM_FLOAT|OM_DOUBLE|OM_OBJECT));
    

    om_prop_t *prop = malloc(sizeof(om_prop_t));
    prop->name = strdup(name);
    
    prop->type_code = type;
    prop->offset = offset;
    
    hashtable_insert(class_object->property_dict, name, prop);
    list_append(class_object->property_list, prop);
    
    return prop;
}

om_prop_t*
om_reg_static_array_prop(om_class_t *class_object, const char *name,
                         om_prop_type_t type, ptrdiff_t offset, size_t length)
{
    om_prop_t *prop = om_reg_prop(class_object, name, OM_ARRAY|type, offset);
    prop->info.static_array.length = length;
    
    return prop;
}

om_prop_t*
om_reg_dynamic_array_prop(om_class_t *class_object, const char *name,
                          om_prop_type_t type, ptrdiff_t offset,
                          char *length_prop)
{
    om_prop_t *prop = om_reg_prop(class_object, name, OM_REF|OM_ARRAY|type,
                                  offset);
    assert(prop != NULL);
    
    prop->info.dynamic_array.length = om_get_prop(class_object, length_prop);
    assert(prop->info.dynamic_array.length != NULL);
    
    return prop;
}


int
om_reg_iface(om_class_t *class_object, const char *name, void *iface_addr)
{
    assert(class_object != NULL);
    assert(name != NULL);
    assert(iface_addr != NULL);
    
    /* Verify that the interface actually have a meaning */
    om_meta_iface_t *meta_iface =
        om_get_meta_iface(class_object->ctxt, name); 
    
    if (meta_iface == NULL) return -1;
    
    om_iface_t *iface = malloc(sizeof(om_iface_t));

    iface->meta_interface = meta_iface;
    iface->address = iface_addr;
    
    hashtable_insert(class_object->interface_dict, name, iface);
}

const char*
om_get_object_name(const om_object_t *obj)
{
    assert(obj != NULL);
    return obj->name;
}

void*
om_get_object_data(const om_object_t *obj)
{
    assert(obj != NULL);
    return obj->data;
}


bool
om_object_is_class(const om_object_t *obj,
                   const char *class_name)
{
    if (strcmp(obj->cls->class_name, class_name)) {
        return false;
    }
    
    return true;
}

om_object_t*
om_get_object_from_ptr(const om_ctxt_t *ctxt, void *address)
{
    om_object_t *obj = hashtable_lookup(ctxt->reverse_object_dict, address);

    return obj;
}


bool
om_conforms_to_iface(const om_object_t *obj, const char *iface_name)
{
    assert(obj != NULL);
    assert(iface_name != NULL);
    
    om_iface_t *iface = om_get_iface(obj->cls, iface_name);
    
    if (iface == NULL) {
        return false;
    }
    
    return true;
}

om_class_t*
om_get_class(const om_ctxt_t *ctxt, const char *class_name)
{
    assert(ctxt != NULL);
    assert(class_name != NULL);
    
    return hashtable_lookup(ctxt->class_dict, class_name);
}

om_object_t*
om_get_object(const om_ctxt_t *ctxt, const char *object_name)
{
    assert(ctxt != NULL);
    assert(object_name != NULL);
    
    return hashtable_lookup(ctxt->object_dict, object_name);
}

om_meta_iface_t*
om_get_meta_iface(const om_ctxt_t *ctxt, const char *iface_name)
{
    assert(ctxt != NULL);
    assert(iface_name != NULL);
    
    return hashtable_lookup(ctxt->interface_dict, iface_name);
}


om_iface_t*
om_get_iface(const om_class_t *cls, const char *iface_name)
{
    assert(cls != NULL);
    assert(iface_name != NULL);

    return hashtable_lookup(cls->interface_dict, iface_name);
}

om_prop_t*
om_get_prop(const om_class_t *cls, const char *prop_name)
{
    assert(cls != NULL);
    assert(prop_name != NULL);
    
    return hashtable_lookup(cls->property_dict, prop_name);
}

void*
om_get_concrete_obj(const om_ctxt_t *ctxt, const char *object_name)
{
    assert(ctxt != NULL);
    assert(object_name != NULL);
    
    om_object_t *obj = om_get_object(ctxt, object_name);
    
    if (obj == NULL) return NULL;
    
    return obj->data;
}

void*
om_get_concrete_iface(const om_class_t *cls, const char *iface_name)
{
    assert(cls != NULL);
    assert(iface_name != NULL);

    om_iface_t *iface = om_get_iface(cls, iface_name);
    
    return iface->address;
}

void*
om_get_concrete_prop(const om_object_t *obj, const char *prop_name)
{
    assert(obj != NULL);
    assert(prop_name != NULL);

    om_prop_t *prop = om_get_prop(obj->cls, prop_name);
    
    return obj->data + prop->offset;
}


om_class_t*
om_get_object_class(const om_object_t *obj)
{
    assert(obj != NULL);
    
    return obj->cls;
}

void*
om_get_concrete_method(const om_class_t *cls, const char *iface_name,
                       const char *method_name)
{
    assert(cls != NULL);
    assert(iface_name != NULL);
    assert(method_name != NULL);
    
    om_iface_t *iface = om_get_iface(cls, iface_name);
    
    ptrdiff_t method_offset = (ptrdiff_t) hashtable_lookup(
                                            iface->meta_interface->methods,
                                            method_name);
    
    return *(void**)(iface->address + method_offset);
}



static void
om_delete_prop_obj(om_prop_t *prop)
{
    assert(prop != NULL);
    
    free(prop->name);
    free(prop);
}

static void
om_delete_iface_obj(om_iface_t *iface)
{
    assert(iface != NULL);
    
    free(iface);
}
// TODO: Ensure that runtime errors are propagated on invalid prop names
#define _OM_GET_PROP_FN_(T, N, TC)                                      \
T                                                                       \
om_get_ ## N ## _prop(const om_object_t *obj, const char *prop_name)    \
{                                                                       \
    assert(obj != NULL);                                                \
    assert(prop_name != NULL);                                          \
                                                                        \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                 \
    if (prop && prop->type_code == (TC)) {                              \
        T *concrete_prop = om_get_concrete_prop(obj, prop_name);        \
        return *concrete_prop;                                          \
    }                                                                   \
    return (T)0;                                                        \
}


// TODO: Ensure that runtime errors are propagated on invalid prop names
#define _OM_SET_PROP_FN_(T, N, TC)                                          \
void                                                                        \
om_set_ ## N ## _prop(om_object_t *obj, const char *prop_name, T val)       \
{                                                                           \
    assert(obj != NULL);                                                    \
    assert(prop_name != NULL);                                              \
                                                                            \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                     \
    if (prop && prop->type_code == (TC)) {                                  \
        T *concrete_prop = om_get_concrete_prop(obj, prop_name);            \
        *concrete_prop = val;                                               \
    }                                                                       \
}

// TODO: Ensure that runtime errors are propagated on invalid prop names
#define _OM_GET_PROP_IDX_FN_(T, N, TC)                                  \
T                                                                       \
om_get_ ## N ## _idx_prop(const om_object_t *obj, const char *prop_name,\
                          unsigned int idx)                             \
{                                                                       \
    assert(obj != NULL);                                                \
    assert(prop_name != NULL);                                          \
                                                                        \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                 \
    if (prop && prop->type_code == (OM_ARRAY|(TC))) {                   \
        T *concrete_prop = (T*)(obj->data + prop->offset);              \
        return concrete_prop[idx];                                      \
    }                                                                   \
    return (T)0;                                                        \
}


// TODO: Ensure that runtime errors are propagated on invalid prop names
// TODO: Remove reliance on abort, should probably have some exception
#define _OM_SET_PROP_IDX_FN_(T, N, TC)                                      \
void                                                                        \
om_set_ ## N ## _idx_prop(om_object_t *obj, const char *prop_name,          \
                          unsigned int idx, T val)                          \
{                                                                           \
    assert(obj != NULL);                                                    \
    assert(prop_name != NULL);                                              \
                                                                            \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                     \
    if (prop && prop->type_code == (OM_ARRAY|(TC))) {                       \
        if (idx < prop->info.static_array.length) {                         \
            T *concrete_prop = (T*)(obj->data + prop->offset);              \
            concrete_prop[idx] = val;                                       \
        } else {                                                            \
            abort();                                                        \
        }                                                                   \
    } else if (prop && prop->type_code == (OM_ARRAY|OM_REF|(TC))) {         \
        om_prop_t *length_prop = prop->info.dynamic_array.length;           \
        size_t *sz = obj->data + length_prop->offset;                       \
        if (idx < *sz ) {                                                   \
            T **concrete_prop = om_get_concrete_prop(obj, prop_name);       \
            (*concrete_prop)[idx] = val;                                    \
        } else {                                                            \
            abort();                                                        \
        }                                                                   \
    }                                                                       \
}

#define _OM_ACCESSOR_PAIR_(T, N , TC)   \
    _OM_GET_PROP_FN_(T, N, TC)          \
    _OM_SET_PROP_FN_(T, N, TC)          \
    _OM_GET_PROP_IDX_FN_(T, N, TC)      \
    _OM_SET_PROP_IDX_FN_(T, N, TC)
    

_OM_ACCESSOR_PAIR_(char, char, OM_CHAR);
_OM_ACCESSOR_PAIR_(short, short, OM_SHORT);
_OM_ACCESSOR_PAIR_(unsigned short, ushort, OM_UNSIGNED|OM_SHORT);
_OM_ACCESSOR_PAIR_(int, int,OM_INT);
_OM_ACCESSOR_PAIR_(unsigned int, uint, OM_UNSIGNED|OM_INT);
_OM_ACCESSOR_PAIR_(long, long, OM_LONG);
_OM_ACCESSOR_PAIR_(unsigned long, ulong, OM_UNSIGNED|OM_LONG);
_OM_ACCESSOR_PAIR_(float, float, OM_FLOAT);
_OM_ACCESSOR_PAIR_(double, double, OM_DOUBLE);
_OM_ACCESSOR_PAIR_(void*, object, OM_OBJECT);

_OM_ACCESSOR_PAIR_(int8_t, int8, OM_INT8);
_OM_ACCESSOR_PAIR_(uint8_t, uint8, OM_UNSIGNED|OM_INT8);
_OM_ACCESSOR_PAIR_(int16_t, int16, OM_INT16);
_OM_ACCESSOR_PAIR_(uint16_t, uint16, OM_UNSIGNED|OM_INT16);
_OM_ACCESSOR_PAIR_(int32_t, int32, OM_INT32);
_OM_ACCESSOR_PAIR_(uint32_t, uint32, OM_UNSIGNED|OM_INT32);
_OM_ACCESSOR_PAIR_(int64_t, int64, OM_INT64); 
_OM_ACCESSOR_PAIR_(uint64_t, uint64, OM_UNSIGNED|OM_INT64);
