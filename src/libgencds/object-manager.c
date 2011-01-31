/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <err.h>
#include <assert.h>
#include <sysexits.h>

#include <gencds/object-manager.h>
#include "object-manager-private.h"

#include <gencds/hashtable.h>
#include <gencds/list.h>

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
    ctxt->object_dict = hashtable_new_with_str_keys(1024);
    ctxt->reverse_object_dict = hashtable_new_with_ptr_keys(1024);

    ctxt->interface_dict = hashtable_new_with_str_keys(1024);
    
    if ((!ctxt->class_dict) || (!ctxt->object_dict) || (!ctxt->interface_dict)
        || (!ctxt->reverse_object_dict)) {
        
        hashtable_delete(ctxt->class_dict);
        hashtable_delete(ctxt->object_dict);
        hashtable_delete(ctxt->reverse_object_dict);
        hashtable_delete(ctxt->interface_dict);
        
        free(ctxt);
        return NULL;
    }
    
    return ctxt;
}

void
om_delete_ctxt(om_ctxt_t *ctxt)
{
    assert(ctxt != NULL);
    list_entry_t *entry;
    
    entry = hashtable_first(ctxt->object_dict);
    while (entry) {
        om_delete_object(hashtable_entry_data(entry));
        entry = list_entry_next(entry);
    }
    
    entry = hashtable_first(ctxt->class_dict);
    while (entry) {
        om_delete_class(hashtable_entry_data(entry));
        entry = list_entry_next(entry);
    }
    
    entry = hashtable_first(ctxt->interface_dict);
    while (entry) {
        om_delete_meta_interface(hashtable_entry_data(entry));
        entry = list_entry_next(entry);
    }
    
    
    // Delete the top level data
    hashtable_delete(ctxt->class_dict);
    hashtable_delete(ctxt->object_dict);
    hashtable_delete(ctxt->interface_dict);
    hashtable_delete(ctxt->reverse_object_dict);

    free(ctxt);
}
 
 
int
om_save_ctxt(om_ctxt_t *ctxt)
{
    return 1;
}

static void
_print_type_code(om_prop_type_t tc)
{
    
}

static void
_write_object(om_object_t *obj, FILE *fp)
{
    om_class_t *cls = obj->cls;
    fprintf(fp, "%s : %s = {\n", obj->name, cls->class_name);
    
    
    list_entry_t *entry = hashtable_first(cls->property_dict);

    while (entry) {
        //  data = hashtable_entry_data(entry);
        //  struct _om_prop_t *prop = list_entry_data(entry);
        //        fprintf(fp, "    %s : ", prop->name);
        //        _print_type_code();
        
        entry = list_entry_next(entry);
    }
    
    fprintf(fp, "}\n");
}

int
om_archive_ctxt(om_ctxt_t *ctxt, const char *fname)
{
    assert(fname != NULL);
    
    FILE *fp = fopen(fname, "r");
    
    if ( fp == NULL ) return 1;

    list_entry_t *entry = hashtable_first(ctxt->object_dict);
    while (entry) {
        //  data = hashtable_entry_data(entry);
        //  _write_object(data, fp);
        entry = list_entry_next(entry);
    }
    
    
    return 1;
}
int
om_restore_ctxt(om_ctxt_t *ctxt)
{
    return 1;
}
int
om_load_ctxt(om_ctxt_t *ctxt, const char *fname)
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
    class_obj->is_proxy = false;
    
    class_obj->property_dict = hashtable_new_with_str_keys(32);
    class_obj->interface_dict = hashtable_new_with_str_keys(32);
    
    if (!(class_obj->property_dict) || !(class_obj->interface_dict)) {
        hashtable_delete(class_obj->property_dict);
        hashtable_delete(class_obj->interface_dict);
        return NULL;
    }
    
    hashtable_insert(ctxt->class_dict, class_name, class_obj);

    return class_obj;
}

om_class_t*
om_new_proxy_class(om_ctxt_t *ctxt, const char *class_name)
{
    assert(ctxt != NULL);
    assert(class_name != NULL);
    
    om_class_t *class_obj = malloc(sizeof(om_class_t));
    if (! class_obj) {
        return NULL;
    }
    
    class_obj->class_name = strdup(class_name);
    class_obj->ctxt = ctxt;
    class_obj->create = NULL; // only neeeded for allocation
    class_obj->destroy = NULL; // only nedded for deallocation
    class_obj->object_size = 0; // only needed for allocation
    class_obj->is_proxy = true;
    
    class_obj->property_dict = hashtable_new_with_str_keys(32);
    class_obj->interface_dict = hashtable_new_with_str_keys(32);
    
    if (!(class_obj->property_dict) || !(class_obj->interface_dict)) {
        hashtable_delete(class_obj->property_dict);
        hashtable_delete(class_obj->interface_dict);
    }
    
    hashtable_insert(ctxt->class_dict, class_name, class_obj);

    return class_obj;
}

void
om_delete_class(om_class_t *cls)
{
    assert(cls != NULL);
    
    list_entry_t *entry;
    entry = hashtable_first(cls->interface_dict);
    while (entry) {
        om_delete_iface_obj(hashtable_entry_data(entry));
        entry = list_entry_next(entry);
    }

    entry = hashtable_first(cls->property_dict);
    while (entry) {
        om_delete_prop_obj(hashtable_entry_data(entry));
        entry = list_entry_next(entry);
    }
        
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
    if (! class_object) {
        warnx("om: cannot allocate object %s (%s class not found)",
              object_name, class_name);
        return NULL;
    }
    if (class_object->is_proxy) {
        warn("om: cannot allocate %s, as the class %s is a proxy",
             object_name, class_name);
        return NULL;
    }
    om_object_t *obj = malloc(sizeof(om_object_t));
    if (obj == NULL) {
        warn("om: object allocation of %s failed", object_name);
        return NULL;
    }
    
    obj->cls = class_object;
    obj->name = strdup(object_name);
    obj->data = class_object->create();

    
    hashtable_insert(ctxt->object_dict, object_name, obj);   
    hashtable_insert(ctxt->reverse_object_dict, obj->data, obj);   
    
    return obj;
}

om_object_t*
om_insert_proxy_obj(om_ctxt_t *ctxt, const char *class_name,
                 const char *object_name, void *obj_addr)
{
    assert(ctxt != NULL);
    assert(class_name != NULL);
    assert(object_name != NULL);
    assert(obj_addr != NULL);
    
    om_class_t *class_object = om_get_class(ctxt, class_name);
    if (! class_object) {
        warnx("om: cannot insert proxy object %s (%s class not found)",
              object_name, class_name);
        return NULL;
    }
    
    if (! class_object->is_proxy) {
        warnx("om: cannot insert proxy object %s (%s is not a proxy class)",
              object_name, class_name);
        return NULL;
    }
    
    om_object_t *obj = hashtable_lookup(ctxt->object_dict, object_name);
    
    if (obj == NULL) {
        obj = malloc(sizeof(om_object_t));
        if (obj == NULL) {
            warn("om: proxy object allocation (objname = %s)", object_name);
            return NULL;
        }
        obj->cls = class_object;
        obj->name = strdup(object_name);
        obj->data = obj_addr;
    
        hashtable_insert(ctxt->object_dict, object_name, obj);
        hashtable_insert(ctxt->reverse_object_dict, obj->data, obj);
    } else {
        // object exist and of the same class
        if (obj->cls == class_object) {
            // remove old reverse pointer and insert the new one
            hashtable_remove(ctxt->reverse_object_dict, obj->data);
            obj->data = obj_addr;
            hashtable_insert(ctxt->reverse_object_dict, obj->data, obj);
        } else {
            warnx("om: proxy insertion class error (objname = %s, prev = %s, "
                  "new = %s)", object_name, obj->cls->class_name, class_name);
            return NULL;
        }
    }
    
    return obj;
}


void
om_delete_object(om_object_t *obj)
{
    assert(obj != NULL);
    
    hashtable_remove(obj->cls->ctxt->object_dict, obj->name);
    hashtable_remove(obj->cls->ctxt->reverse_object_dict, obj->data);
    
    // Delete object only if this is a non proxy class
    if (! obj->cls->is_proxy) {
        obj->cls->destroy(obj->data);
    }
    
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

    return mi;
}

void
om_delete_meta_interface(om_meta_iface_t *iface)
{
    assert(iface != NULL);
    
    list_entry_t *entry = hashtable_first(iface->methods);
    while (entry) {
        // TODO: Delete from list, at the moment this is not strictly necisary
        //       since no type signatures are registered and only the method
        //       pointer
        entry = list_entry_next(entry);
    }
    hashtable_delete(iface->methods);
    free(iface->name);
    free(iface);
}

int
om_reg_method(om_meta_iface_t *meta_iface, const char *method_name,
              const char *convention, ptrdiff_t offset)
{
    assert(meta_iface != NULL);
    assert(method_name != NULL);
    
    return hashtable_insert(meta_iface->methods, method_name, (void*)offset);
}

/* TODO: Support complex types (arrays, objects, refs, et.c.) */

om_prop_t*
om_reg_overloaded_prop(om_class_t *class_object, const char *name,
                       om_prop_type_t type, ptrdiff_t offset,
                       void *getter, void *setter, void *reader, void *writer)
{
    om_prop_t *prop = om_reg_prop(class_object, name, type, offset);
    if (prop != NULL) {
        prop->get = getter;
        prop->set = setter;
        prop->rd = reader;
        prop->wr = writer;
    }
    return prop;                
}

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
    prop->get = NULL;
    prop->set = NULL;
    prop->rd = NULL;
    prop->wr = NULL;
    
    hashtable_insert(class_object->property_dict, name, prop);
    //list_append(class_object->property_list, prop);
    
    return prop;
}

om_prop_t*
om_reg_static_array_prop(om_class_t *class_object, const char *name,
                         om_prop_type_t type, ptrdiff_t offset, size_t length)
{
    assert(class_object != NULL);
    assert(name != NULL);
    
    om_prop_t *prop = om_reg_prop(class_object, name, OM_ARRAY|type, offset);
    if (prop == NULL) return NULL;
    prop->info.static_array.length = length;
    
    return prop;
}


om_prop_t*
om_reg_overloaded_static_array_prop(om_class_t *class_object, const char *name,
                                    om_prop_type_t type, ptrdiff_t offset,
                                    size_t length,
                                    void *getter, void *setter, void *reader, void *writer)
{
    om_prop_t *prop = om_reg_static_array_prop(class_object, name, type, offset, length);
    
    if (prop != NULL) {
        prop->get = getter;
        prop->set = setter;
        prop->rd = reader;
        prop->wr = writer;
    }
    return prop;                                                   
}


om_prop_t*
om_reg_dynamic_array_prop(om_class_t *class_object, const char *name,
                          om_prop_type_t type, ptrdiff_t offset,
                          char *length_prop)
{
    assert(class_object != NULL);
    assert(name != NULL);

    om_prop_t *prop = om_reg_prop(class_object, name, OM_REF|OM_ARRAY|type,
                                  offset);

    if (prop == NULL) return NULL;
    
    prop->info.dynamic_array.length = om_get_prop(class_object, length_prop);
    assert(prop->info.dynamic_array.length != NULL);
    
    return prop;
}

om_prop_t*
om_reg_overloaded_dynamic_array_prop(om_class_t *class_object, const char *name,
                                     om_prop_type_t type, ptrdiff_t offset,
                                     char *length_prop,
                                     void *getter, void *setter, void *reader, void *writer)
{
    om_prop_t *prop = om_reg_dynamic_array_prop(class_object, name, type, offset, length_prop);

    if (prop != NULL) {
        prop->get = getter;
        prop->set = setter;
        prop->rd = reader;
        prop->wr = writer;
    }
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
    
    if (meta_iface == NULL) {
        warnx("om: tried to register a concrete inteface that have no meta interface");
        return -1;
    }
    om_iface_t *iface = malloc(sizeof(om_iface_t));

    iface->meta_interface = meta_iface;
    iface->address = iface_addr;
    
    return hashtable_insert(class_object->interface_dict, name, iface);
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
    assert(obj != NULL);
    assert(class_name != NULL);
    
    if (strcmp(obj->cls->class_name, class_name)) {
        return false;
    }
    
    return true;
}

om_object_t*
om_get_object_from_ptr(const om_ctxt_t *ctxt, void *address)
{
    assert(ctxt != NULL);
    assert(address != NULL);

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

void
om_call_method(const char *obj_name, const char *iface, const char *method,
               void *res, ...)
{
    // Look up method pointer and object pointer
    // Build up call stack from this method and
    // get the result from the method
    // NOTE: This method is not very fast and does not intend to be fast, it exists to
    //       allow the object manager to be easibly integrated with a scripting system
    //       if objects are accessed in C, make sure that you know the type and use the
    //       real interfaces
    // Calling conventions: i/I int/uint
    //                      s/S short/ushort
    //                      l/L long/ulong
    //                      f/F float/double
    //                      c   char
    //                      p   pointer
    //                      b/B int8
    //                      h/H int16
    //                      w/W int32
    //                      d/D int64
    
    
    char c;
    va_list vl;
    va_start(vl, res);
    while (0) {
        switch (c) {
        case 'i': va_arg(vl, int);
        case 'I': va_arg(vl, unsigned int);
        case 's':(short)va_arg(vl, int);
        case 'S':(unsigned short)va_arg(vl, unsigned int);
        case 'l':va_arg(vl, long);
        case 'L':va_arg(vl, unsigned long);
        case 'f':(float)va_arg(vl, double);
        case 'F':va_arg(vl, double);
        case 'c':(char)va_arg(vl, int);
        case 'p':va_arg(vl, void*);
        case 'b':(int8_t)va_arg(vl, int);
        case 'B':(uint8_t)va_arg(vl, int);
        case 'h':(int16_t)va_arg(vl, int);
        case 'H':(uint16_t)va_arg(vl, int);
        case 'w':va_arg(vl, int32_t);
        case 'W':va_arg(vl, uint32_t);
        case 'd':va_arg(vl, int64_t);
        case 'D':va_arg(vl, uint64_t);
            
        }
    }
    va_end(vl);    
}

// TODO: Ensure that runtime errors are propagated on invalid prop names
#define _OM_GET_PROP_FN_(T, N, TC)                                          \
T                                                                           \
om_get_ ## N ## _prop(const om_object_t *obj, const char *prop_name)        \
{                                                                           \
    assert(obj != NULL);                                                    \
    assert(prop_name != NULL);                                              \
                                                                            \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                     \
    if (prop && prop->type_code == (TC)) {                                  \
        if (prop->get) return ((om_get_ ## N ## _f)prop->get)(obj->data);   \
        T *concrete_prop = om_get_concrete_prop(obj, prop_name);            \
        return *concrete_prop;                                              \
    }                                                                       \
    return (T)0;                                                            \
}


// TODO: Ensure that runtime errors are propagated on invalid prop names
#define _OM_SET_PROP_FN_(T, N, TC)                                      \
void                                                                    \
om_set_ ## N ## _prop(om_object_t *obj, const char *prop_name, T val)   \
{                                                                       \
    assert(obj != NULL);                                                \
    assert(prop_name != NULL);                                          \
                                                                        \
	om_prop_t *prop = om_get_prop(obj->cls, prop_name);                 \
    if (prop && prop->type_code == (TC)) {                              \
        if (prop->set) {                                                \
            ((om_set_ ## N ## _f)prop->set)(obj->data, val);            \
        } else {                                                        \
            T *concrete_prop = om_get_concrete_prop(obj, prop_name);    \
            *concrete_prop = val;                                       \
        }                                                               \
    }                                                                   \
}

// TODO: Ensure that runtime errors are propagated on invalid prop names,
//       for now, just let the application die.
#define _OM_GET_PROP_IDX_FN_(T, N, TC)                                      \
T                                                                           \
om_get_ ## N ## _idx_prop(const om_object_t *obj, const char *prop_name,    \
                          unsigned int idx)                                 \
{                                                                           \
    assert(obj != NULL);                                                    \
    assert(prop_name != NULL);                                              \
                                                                            \
    om_prop_t *prop = om_get_prop(obj->cls, prop_name);                     \
    if (prop && prop->type_code == (OM_ARRAY|(TC))) {                       \
        if (idx < prop->info.static_array.length) {                         \
            if (prop->get) {                                                \
                return ((om_idx_get_ ## N ## _f)prop->get)(obj->data, idx); \
            } else {                                                        \
                T *concrete_prop = (T*)(obj->data + prop->offset);          \
                return concrete_prop[idx];                                  \
            }                                                               \
        } else {                                                            \
            errx(EX_SOFTWARE,                                               \
                 "om: index out of static bounds (idx = %d, maxidx = %d)",  \
                 idx, prop->info.static_array.length-1);                    \
        }                                                                   \
    } else if (prop && prop->type_code == (OM_ARRAY|OM_REF|(TC))) {         \
        om_prop_t *length_prop = prop->info.dynamic_array.length;           \
        size_t *sz = obj->data + length_prop->offset;                       \
        if (idx < *sz) {                                                    \
            if (prop->get) {                                                \
                return ((om_idx_get_ ## N ## _f)prop->get)(obj->data, idx); \
            } else {                                                        \
                T **concrete_prop = om_get_concrete_prop(obj, prop_name);   \
                return (*concrete_prop)[idx];                               \
            }                                                               \
        } else {                                                            \
            errx(EX_SOFTWARE,                                               \
                 "om: index out of dynamic bounds (idx = %d, maxidx = %d)", \
                 idx, (*sz)-1);                                             \
        }                                                                   \
    }                                                                       \
  assert(0 && "should be unreachable");                                     \
  \
}


// TODO: Ensure that runtime errors are propagated on invalid prop names
//       for now, just let the application die.
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
            if (prop->set) {                                                \
                ((om_idx_set_ ## N ## _f)prop->set)(obj->data, idx, val);   \
            } else {                                                        \
                T *concrete_prop = (T*)(obj->data + prop->offset);          \
                concrete_prop[idx] = val;                                   \
            }                                                               \
        } else {                                                            \
            errx(EX_SOFTWARE,                                               \
                 "om: index out of static bounds (idx = %d, maxidx = %d)",  \
                 idx, prop->info.static_array.length-1);                    \
        }                                                                   \
    } else if (prop && prop->type_code == (OM_ARRAY|OM_REF|(TC))) {         \
        om_prop_t *length_prop = prop->info.dynamic_array.length;           \
        size_t *sz = obj->data + length_prop->offset;                       \
        if (idx < *sz ) {                                                   \
            if (prop->set) {                                                \
                ((om_idx_set_ ## N ## _f)prop->set)(obj->data, idx, val);   \
            } else {                                                        \
                T **concrete_prop = om_get_concrete_prop(obj, prop_name);   \
                (*concrete_prop)[idx] = val;                                \
            }                                                               \
        } else {                                                            \
            errx(EX_SOFTWARE,                                               \
                 "om: index out of dynamic bounds (idx = %d, maxidx = %d)", \
                 idx, (*sz)-1);                                             \
        }                                                                   \
    }                                                                       \
}

#define _OM_ACCESSOR_PAIR_(T, N , TC)   \
    _OM_GET_PROP_FN_(T, N, TC)          \
    _OM_SET_PROP_FN_(T, N, TC)          \
    _OM_GET_PROP_IDX_FN_(T, N, TC)      \
    _OM_SET_PROP_IDX_FN_(T, N, TC)


//_OM_ACCESSOR_PAIR_(bool, bool, OM_BOOL);
// bool is a macro that expands to _Bool, we have to define this here locally
// as the submacros would otherwise be passed as _Bool 
_OM_GET_PROP_FN_(bool, bool, OM_BOOL)
_OM_SET_PROP_FN_(bool, bool, OM_BOOL)
_OM_GET_PROP_IDX_FN_(bool, bool, OM_BOOL)
_OM_SET_PROP_IDX_FN_(bool, bool, OM_BOOL)


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
