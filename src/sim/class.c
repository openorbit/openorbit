/*
 Copyright 2012,2013,2014 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit.

 Open Orbit is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Open Orbit is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <complex.h>
#include "common/palloc.h"

#include "sim/class.h"

#include <gencds/hashtable.h>
#include <gencds/avl-tree.h>
#include <openorbit/log.h>
#include "common/moduleinit.h"

static avl_tree_t *classes;
static avl_tree_t *objects;

void
sim_default_setter(sim_object_t *obj, sim_prop_val_t prop)
{
  (void)obj;
  (void)prop;
}

sim_prop_val_t
sim_default_getter(sim_object_t *obj)
{
  (void)obj;

  sim_prop_val_t prop = {.typ = SIM_TYPE_INVALID};
  return prop;
}

static void
Object_init(sim_class_t *cls, void *obj, void *arg)
{
  sim_object_t *sobj = obj;
  uuid_generate(sobj->uuid);
  avl_insert(objects, sobj->uuid, sobj);
}

void
sim_class_init(void)
{
  classes = avl_str_new();
  assert(classes && "avl tree not allocated");
  sim_class_t *cls = sim_register_class(NULL, "Object", Object_init,
                                        sizeof(sim_object_t));
  sim_class_add_field(cls, SIM_TYPE_CLASS_PTR,
                      "cls", offsetof(sim_object_t, cls), NULL, NULL);
  sim_class_add_field(cls, SIM_TYPE_UUID,
                      "uuid", offsetof(sim_object_t, uuid), NULL, NULL);
  sim_class_add_field(cls, SIM_TYPE_STR,
                      "name", offsetof(sim_object_t, name), NULL, NULL);

  objects = avl_uuid_new();
}

MODULE_INIT(object, NULL)
{
  log_trace("initialising 'object' module");
  sim_class_init();
}

sim_class_t*
sim_class_get(const char *cls_name)
{
  sim_class_t *cls = avl_find(classes, cls_name);
  log_fatal_if_null(cls, "unknown class '%s'", cls_name);
  return cls;
}

sim_class_t*
sim_register_class_(const char *super, const char *name,
                    void *(*alloc)(sim_class_t *cls),
                    void (*init)(sim_class_t *cls, void *obj, void *arg),
                    void (*dealloc)(void *obj),
                    void (*restored)(void *obj))
{
  sim_class_t *cls = smalloc(sizeof(sim_class_t));

  if (super)  cls->super = sim_class_get(super);
  else cls->super = NULL;

  cls->name = strdup(name);
  cls->alloc = alloc;
  cls->init = init;
  if (dealloc) cls->dealloc = dealloc;
  else cls->dealloc = free;

  cls->restored = restored;

  obj_array_init(&cls->fields);
  cls->key_index_map = avl_str_new();
  avl_insert(classes, name, cls);
  cls->interfaces = avl_str_new();

  return cls;
}

static void*
def_alloc(sim_class_t *cls)
{
  sim_object_t* obj = smalloc(cls->obj_size);
  memset(obj, 0, cls->obj_size);
  obj->cls = cls;
  return obj;
}

sim_class_t*
sim_register_class(const char *super, const char *name,
                   void (*init)(sim_class_t *cls, void *obj, void *arg),
                   size_t size)
{
  sim_class_t *cls = sim_register_class_(super, name,
                                         def_alloc, init, NULL, NULL);
  cls->obj_size = size;
  return cls;
}

void
sim_class_add_field(sim_class_t *class, sim_type_id_t type,
                    const char *key, off_t offset, sim_setter_fn set, sim_getter_fn get)
{
  sim_field_t *field = smalloc(sizeof(sim_field_t));
  field->name = strdup(key);
  field->typ = type;
  field->offs = offset;

  if (set) field->set = set;
  else field->set = sim_default_setter;

  if (get) field->get = get;
  else field->get = sim_default_getter;

  obj_array_push(&class->fields, field);

  avl_insert(class->key_index_map, key,
             (void*)(ARRAY_LEN(class->fields)));
}

sim_field_t*
sim_class_get_field(sim_class_t *cls, const char *key)
{
  while (cls) {
    uintptr_t idx = (uintptr_t) avl_find(cls->key_index_map, key);
    if (idx > 0) {
      return ARRAY_ELEM(cls->fields , idx-1);
    }
    cls = cls->super;
  }

  return NULL;
}

void
sim_class_add_interface(sim_class_t *class, const char *key,
                        void *iface)
{
  avl_insert(class->interfaces, key, iface);
}

void*
sim_class_get_interface(sim_class_t *class, const char *key)
{
  return avl_find(class->interfaces, key);
}

sim_object_t*
sim_alloc_object(const char *cls_name)
{
  sim_class_t *cls = sim_class_get(cls_name);
  sim_object_t *obj = cls->alloc(cls);
  obj->cls = cls;
  return obj;
}

void
sim_delete_object(sim_object_t *obj)
{
  char *name = (char*) obj->name;
  obj->cls->dealloc(obj);
  free(name);
}

void
sim_init_object(sim_object_t *obj, const char *name, void *arg)
{
  obj->name = strdup(name);
  obj->cls->init(obj->cls, obj, arg);
}

sim_prop_val_t
sim_get_field(sim_object_t *obj, const char *field_name)
{
  sim_prop_val_t prop = {.typ = SIM_TYPE_INVALID};

  sim_field_t *field = sim_class_get_field(obj->cls, field_name);
  if (!field) {
    log_error("field '%s' is not a member of class '%s'",
              field_name, obj->cls->name);
    return prop;
  }

  if (field->get) {
    prop = field->get(obj);
  } else {
    prop.typ = field->typ;

    switch (field->typ) {
    case SIM_TYPE_CLASS_PTR:
      prop.class_ptr = *(sim_class_t**) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UUID:
      memcpy(prop.uuid, (((uint8_t*)obj) + field->offs), sizeof(uuid_t));
      break;
    case SIM_TYPE_OBJ:
      prop.obj = *(void**) (((uint8_t*)obj) + field->offs);
    case SIM_TYPE_IFACE:
      prop.iface.obj = *(void**) (((uint8_t*)obj) + field->offs);
      prop.iface.iface =
        *(void**) (((uint8_t*)obj) + field->offs + sizeof(void*));
      break;
    case SIM_TYPE_BOOL:
      prop.boolean = *(bool*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_CHAR:
      prop.schar = *(char*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UCHAR:
      prop.uchar = *(unsigned char*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_SHORT:
      prop.sshort = *(short*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_USHORT:
      prop.ushort = *(unsigned short*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_INT:
      prop.sint = *(int*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UINT:
      prop.uint = *(unsigned int*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_LONG:
      prop.slong = *(long*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_ULONG:
      prop.ulong = *(unsigned long*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UINT8:
      prop.u8 = *(uint8_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UINT16:
      prop.u16 = *(uint16_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UINT32:
      prop.u32 = *(uint32_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_UINT64:
      prop.u64 = *(uint64_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_INT8:
      prop.i8 = *(int8_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_INT16:
      prop.i16 = *(int16_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_INT32:
      prop.i32 = *(int32_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_INT64:
      prop.i64 = *(int64_t*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_STR:
      prop.str = *(const char**) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_FLOAT:
      prop.f = *(float*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_DOUBLE:
      prop.d = *(double*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_COMPLEX_FLOAT:
      prop.cf = *(complex float*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_COMPLEX_DOUBLE:
      prop.cd = *(complex double*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_FLOAT_VEC3:
      prop.fv3 = *(float3*) (((uint8_t*)obj) + field->offs);
      break;
    case SIM_TYPE_FLOAT_VEC4:
      prop.fv4 = *(float4*) (((uint8_t*)obj) + field->offs);
      break;

      // Does not handle arrays or matrices
    default:
      assert(0 && "invalid case");
    }
  }

  return prop;
}

void
sim_set_field(sim_object_t *obj, const char *field_name, sim_prop_val_t prop)
{
  sim_field_t *field = sim_class_get_field(obj->cls, field_name);
  if (!field) {
    log_error("field '%s' is not a member of class '%s'",
               field_name, obj->cls->name);
    return;
  }

  if (field->typ != prop.typ) {
    log_error("type mismatch between field and set property");
  }

  if (field->set) {
    field->set(obj, prop);
  } else {
    switch (field->typ) {
    case SIM_TYPE_CLASS_PTR:
      *(sim_class_t**) (((uint8_t*)obj) + field->offs) = prop.class_ptr;
      break;
    case SIM_TYPE_UUID:
      memcpy((((uint8_t*)obj) + field->offs), prop.uuid, sizeof(uuid_t));
      break;
    case SIM_TYPE_OBJ:
      *(sim_object_t**) (((uint8_t*)obj) + field->offs) = prop.obj;
      break;
    case SIM_TYPE_IFACE:
      *(void**) (((uint8_t*)obj) + field->offs) = prop.iface.obj;
      *(void**) (((uint8_t*)obj) + field->offs + sizeof(void*)) = prop.iface.iface;
      break;
    case SIM_TYPE_BOOL:
      *(bool*) (((uint8_t*)obj) + field->offs) = prop.boolean;
      break;
    case SIM_TYPE_CHAR:
      *(char*) (((uint8_t*)obj) + field->offs) = prop.schar;
      break;
    case SIM_TYPE_UCHAR:
      *(unsigned char*) (((uint8_t*)obj) + field->offs) = prop.uchar;
      break;
    case SIM_TYPE_SHORT:
      *(short*) (((uint8_t*)obj) + field->offs) = prop.sshort;
      break;
    case SIM_TYPE_USHORT:
      *(unsigned short*) (((uint8_t*)obj) + field->offs) = prop.ushort;
      break;
    case SIM_TYPE_INT:
      *(int*) (((uint8_t*)obj) + field->offs) = prop.sint;
      break;
    case SIM_TYPE_UINT:
      *(unsigned int*) (((uint8_t*)obj) + field->offs) = prop.uint;
      break;
    case SIM_TYPE_LONG:
      *(long*) (((uint8_t*)obj) + field->offs) = prop.slong;
      break;
    case SIM_TYPE_ULONG:
      *(unsigned long*) (((uint8_t*)obj) + field->offs) = prop.ulong;
      break;

    case SIM_TYPE_UINT8:
      *(uint8_t*) (((uint8_t*)obj) + field->offs) = prop.u8;
      break;
    case SIM_TYPE_UINT16:
      *(uint16_t*) (((uint8_t*)obj) + field->offs) = prop.u16;
      break;
    case SIM_TYPE_UINT32:
      *(uint32_t*) (((uint8_t*)obj) + field->offs) = prop.u32;
      break;
    case SIM_TYPE_UINT64:
      *(uint64_t*) (((uint8_t*)obj) + field->offs) = prop.u64;
      break;
    case SIM_TYPE_INT8:
      *(int8_t*) (((uint8_t*)obj) + field->offs) = prop.i8;
      break;
    case SIM_TYPE_INT16:
      *(int16_t*) (((uint8_t*)obj) + field->offs) = prop.i16;
      break;
    case SIM_TYPE_INT32:
      *(int32_t*) (((uint8_t*)obj) + field->offs) = prop.i32;
      break;
    case SIM_TYPE_INT64:
      *(int64_t*) (((uint8_t*)obj) + field->offs) = prop.i64;
      break;
    case SIM_TYPE_STR:
      assert(0 && "cannot set string at the moment");
      break;
    case SIM_TYPE_FLOAT:
      *(float*) (((uint8_t*)obj) + field->offs) = prop.f;
      break;
    case SIM_TYPE_DOUBLE:
      *(double*) (((uint8_t*)obj) + field->offs) = prop.d;
      break;
    case SIM_TYPE_COMPLEX_FLOAT:
      *(complex float*) (((uint8_t*)obj) + field->offs) = prop.cf;
      break;
    case SIM_TYPE_COMPLEX_DOUBLE:
      *(complex double*) (((uint8_t*)obj) + field->offs) = prop.cd;
      break;
    case SIM_TYPE_FLOAT_VEC3:
      *(float3*) (((uint8_t*)obj) + field->offs) = prop.fv3;
      break;
    case SIM_TYPE_FLOAT_VEC4:
      *(float4*) (((uint8_t*)obj) + field->offs) = prop.fv4;
      break;
    default:
      assert(0 && "invalid case");
    }
  }
}

#define SET_ARRAY_ELEM(at,t)\
do {\
  at##_array_t *dst = (at##_array_t*)(((uintptr_t)obj) + field->offs); \
  t *src = data;\
  ARRAY_ELEM(*dst, index) = *src;\
} while (0)

void
sim_set_field_by_index(sim_object_t *obj, const char *field_name,
                       size_t index, void *data)
{
  assert(obj != NULL);
  sim_field_t *field = sim_class_get_field(obj->cls, field_name);
  if (!field) {
    log_error("field '%s' is not a member of class '%s'",
               field_name, obj->cls->name);
    return;
  }

  switch (field->typ) {
  case SIM_TYPE_COMPLEX_FLOAT: {
    complex float *dst = (complex float*)(((uintptr_t)obj) + field->offs);
    float *src = data;
    if (index == 0) {
      *dst = *src + cimagf(*dst)*I;
    } else if (index == 1) {
      *dst = crealf(*dst) + *src*I;

    }
    break;
  }
  case SIM_TYPE_COMPLEX_DOUBLE: {
    complex double *dst = (complex double*)(((uintptr_t)obj) + field->offs);
    double *src = data;
    if (index == 0) {
      *dst = *src + cimag(*dst)*I;
    } else if (index == 1) {
      *dst = creal(*dst) + *src*I;
    }
    break;
  }
  case SIM_TYPE_FLOAT_VEC3: {
    float3 *dst = (float3*)(((uintptr_t)obj) + field->offs);
    float *src = data;
    (*dst)[index] = *src;
    break;
  }
  case SIM_TYPE_FLOAT_VEC4: {
    float4 *dst = (float4*)(((uintptr_t)obj) + field->offs);
    float *src = data;
    (*dst)[index] = *src;
    break;
  }
  case SIM_TYPE_FLOAT_VEC3x3: {
    float3x3 *dst = (float3x3*)(((uintptr_t)obj) + field->offs);
    float *src = data;
    (*dst)[index/3][index%3] = *src;
    break;
  }
  case SIM_TYPE_FLOAT_VEC4x4:  {
    float4x4 *dst = (float4x4*)(((uintptr_t)obj) + field->offs);
    float *src = data;
    (*dst)[index/4][index%4] = *src;
    break;
  }
  // Common array types
  case SIM_TYPE_BOOL_ARR:
    SET_ARRAY_ELEM(bool,bool);
    break;
  case SIM_TYPE_CHAR_ARR:
    SET_ARRAY_ELEM(char,char);
    break;
  case SIM_TYPE_UCHAR_ARR:
    SET_ARRAY_ELEM(uchar,unsigned char);
    break;
  case SIM_TYPE_SHORT_ARR:
    SET_ARRAY_ELEM(short,short);
    break;
  case SIM_TYPE_USHORT_ARR:
    SET_ARRAY_ELEM(ushort,unsigned short);
    break;
  case SIM_TYPE_INT_ARR:
    SET_ARRAY_ELEM(int,int);
    break;
  case SIM_TYPE_UINT_ARR:
    SET_ARRAY_ELEM(uint, unsigned int);
    break;
  case SIM_TYPE_LONG_ARR:
    SET_ARRAY_ELEM(long,long);
    break;
  case SIM_TYPE_ULONG_ARR:
    SET_ARRAY_ELEM(ulong,unsigned long);
    break;
  case SIM_TYPE_UINT8_ARR:
    SET_ARRAY_ELEM(u8,uint8_t);
    break;
  case SIM_TYPE_UINT16_ARR:
    SET_ARRAY_ELEM(u16,uint16_t);
    break;
  case SIM_TYPE_UINT32_ARR:
    SET_ARRAY_ELEM(u32,uint32_t);
    break;
  case SIM_TYPE_UINT64_ARR:
    SET_ARRAY_ELEM(u64,uint64_t);
    break;
  case SIM_TYPE_INT8_ARR:
    SET_ARRAY_ELEM(i8,int8_t);
    break;
  case SIM_TYPE_INT16_ARR:
    SET_ARRAY_ELEM(i16,int16_t);
    break;
  case SIM_TYPE_INT32_ARR:
    SET_ARRAY_ELEM(i32,int32_t);
    break;
  case SIM_TYPE_INT64_ARR:
    SET_ARRAY_ELEM(i64,int64_t);
    break;
  case SIM_TYPE_FLOAT_ARR:
    SET_ARRAY_ELEM(float,float);
    break;
  case SIM_TYPE_DOUBLE_ARR:
    SET_ARRAY_ELEM(double,double);
    break;
  case SIM_TYPE_COMPLEX_FLOAT_ARR:
  case SIM_TYPE_COMPLEX_DOUBLE_ARR:
  case SIM_TYPE_OBJ_ARR:
    assert(0 && "FIXME");
    break;
  default:
    assert(0 && "invalid type (unknown or scalar)");
  }
}

// For printing normal scalars
#define PRINT_MACRO(T, FMT)                                   \
  do {                                                        \
    T val = *(T*)((uintptr_t)obj + field->offs);              \
    fprintf(fout, "\t\"%s\" : %" #FMT "\n", field->name, val);\
    break;                                                    \
  } while (0)

// For printing C99 fixed size scalars
#define PRINT_MACRO2(T, FMT)                                  \
  do {                                                        \
    T val = *(T*)((uintptr_t)obj + field->offs);              \
    fprintf(fout, "\t\"%s\" : %" FMT "\n", field->name, val); \
    break;                                                    \
  } while (0)

// For printing normal typed arrays
#define PRINT_MACRO3(T, FMT)                        \
  do {                                              \
    T *val = (T*)((uintptr_t)obj + field->offs);    \
    fprintf(fout, "\t\"%s\" : [", field->name);     \
    ARRAY_FOR_EACH(i, *val) {                       \
      fprintf(fout, "%"#FMT, ARRAY_ELEM(*val, i));  \
      if (i != ARRAY_LEN(*val) - 1) {               \
        fprintf(fout, ", ");                        \
      }                                             \
    }                                               \
    fprintf(fout, "]\n");                           \
    break;                                          \
  } while (0)


// For printing C99 fixed size typed arrays
#define PRINT_MACRO4(T, FMT)                        \
  do {                                              \
    T *val = (T*)((uintptr_t)obj + field->offs);    \
    fprintf(fout, "\t\"%s\" : [", field->name);     \
    ARRAY_FOR_EACH(i, *val) {                       \
      fprintf(fout, "%" FMT, ARRAY_ELEM(*val, i) ); \
      if (i != ARRAY_LEN(*val) - 1) {               \
        fprintf(fout, ", ");                        \
      }                                             \
    }                                               \
    fprintf(fout, "]\n");                           \
    break;                                          \
  } while (0)


void
sim_print_field(FILE *fout, sim_object_t *obj, sim_field_t *field)
{
  fprintf(fout, "\t\"%s\" : ", field->name);

  switch (field->typ) {
  case SIM_TYPE_RECORD: {
    sim_object_t *ptr = *(sim_object_t**)((uintptr_t)obj + field->offs);
    uuid_string_t uuid;
    uuid_unparse(ptr->uuid, uuid);
    fprintf(fout, "\t\"%s\" : @%s\n", field->name, uuid);
    break;
  }
  case SIM_TYPE_LINK:
    break;
  case SIM_TYPE_CLASS_PTR: {
    sim_class_t *cls = *(sim_class_t**)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : @%s\n", field->name, cls->name);
    break;
  }
  case SIM_TYPE_UUID:  {
    uuid_t uuid;
    uuid_copy(uuid, *(uuid_t*)((uintptr_t)obj + field->offs));
    char uuid_str[37];
    uuid_unparse(uuid, uuid_str);
    fprintf(fout, "\t\"%s\" : %s\n", field->name, uuid_str);
    break;
  }
  case SIM_TYPE_BOOL: {
    bool val = *(bool*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : %s\n", field->name, val ? "true" : "false");
    break;
  }
  case SIM_TYPE_CHAR:
    PRINT_MACRO(char, c);
    break;
  case SIM_TYPE_UCHAR:
    PRINT_MACRO(unsigned char, hhu);
    break;
  case SIM_TYPE_SHORT:
    PRINT_MACRO(short, hd);
    break;
  case SIM_TYPE_USHORT:
    PRINT_MACRO(unsigned short, hu);
    break;
  case SIM_TYPE_INT:
    PRINT_MACRO(int, d);
    break;
  case SIM_TYPE_UINT:
    PRINT_MACRO(unsigned int, u);
    break;
  case SIM_TYPE_LONG:
    PRINT_MACRO(long, ld);
    break;
  case SIM_TYPE_ULONG:
    PRINT_MACRO(unsigned long, lu);
    break;
  case SIM_TYPE_UINT8:
    PRINT_MACRO2(uint8_t, PRIu8);
    break;
  case SIM_TYPE_UINT16:
    PRINT_MACRO2(uint16_t, PRIu16);
    break;
  case SIM_TYPE_UINT32:
    PRINT_MACRO2(uint32_t, PRIu32);
    break;
  case SIM_TYPE_UINT64:
    PRINT_MACRO2(uint64_t, PRIu64);
    break;
  case SIM_TYPE_INT8:
    PRINT_MACRO2(int8_t, PRId8);
    break;
  case SIM_TYPE_INT16:
    PRINT_MACRO2(int16_t, PRId16);
    break;
  case SIM_TYPE_INT32:
    PRINT_MACRO2(int32_t, PRId32);
    break;
  case SIM_TYPE_INT64:
    PRINT_MACRO2(int64_t, PRId64);
    break;
  case SIM_TYPE_STR:
    PRINT_MACRO(char*, s);
    break;
  case SIM_TYPE_FLOAT:
    PRINT_MACRO(float, f);
    break;
  case SIM_TYPE_DOUBLE:
    PRINT_MACRO(double, f);
    break;
  case SIM_TYPE_COMPLEX_FLOAT: {
    complex float val = *(complex float*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : %f + %f I\n", field->name,
            crealf(val), cimagf(val));
    break;
  }
  case SIM_TYPE_COMPLEX_DOUBLE: {
    complex double val = *(complex double*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : %f + %f I\n", field->name,
            creal(val), cimag(val));
    break;
  }
  case SIM_TYPE_FLOAT_VEC3: {
    float3 val = *(float3*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [%f, %f, %f]\n", field->name,
            val.x, val.y, val.z);
    break;
  }
  case SIM_TYPE_FLOAT_VEC4: {
    float4 val = *(float4*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [%f, %f, %f, %f]\n", field->name,
            val.x, val.y, val.z, val.w);
    break;
  }
  case SIM_TYPE_FLOAT_VEC3x3:
    break;
  case SIM_TYPE_FLOAT_VEC4x4:
    break;
  case SIM_TYPE_BOOL_ARR: {
    bool_array_t *val = (bool_array_t*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [", field->name);
    ARRAY_FOR_EACH(i, *val) {
      fprintf(fout, "%s", ARRAY_ELEM(*val, i) ? "true" : "false");
      if (i != ARRAY_LEN(*val) - 1) {
        fprintf(fout, ", ");
      }
    }
    fprintf(fout, "]\n");
    break;
  }
  case SIM_TYPE_CHAR_ARR:
    PRINT_MACRO3(char_array_t, c);
    break;
  case SIM_TYPE_UCHAR_ARR:
    PRINT_MACRO3(uchar_array_t, hhu);
    break;
  case SIM_TYPE_SHORT_ARR:
    PRINT_MACRO3(short_array_t, hd);
    break;
  case SIM_TYPE_USHORT_ARR:
    PRINT_MACRO3(ushort_array_t, hu);
    break;
  case SIM_TYPE_INT_ARR:
    PRINT_MACRO3(int_array_t, d);
    break;
  case SIM_TYPE_UINT_ARR:
    PRINT_MACRO3(uint_array_t, u);
    break;
  case SIM_TYPE_LONG_ARR:
    PRINT_MACRO3(long_array_t, ld);
    break;
  case SIM_TYPE_ULONG_ARR:
    PRINT_MACRO3(ulong_array_t, lu);
    break;
  case SIM_TYPE_UINT8_ARR:
    PRINT_MACRO4(u8_array_t, PRIu8);
    break;
  case SIM_TYPE_UINT16_ARR:
    PRINT_MACRO4(u16_array_t, PRIu16);
    break;
  case SIM_TYPE_UINT32_ARR:
    PRINT_MACRO4(u32_array_t, PRIu32);
    break;
  case SIM_TYPE_UINT64_ARR:
    PRINT_MACRO4(u64_array_t, PRIu64);
    break;
  case SIM_TYPE_INT8_ARR:
    PRINT_MACRO4(i8_array_t, PRId8);
    break;
  case SIM_TYPE_INT16_ARR:
    PRINT_MACRO4(i16_array_t, PRId16);
    break;
  case SIM_TYPE_INT32_ARR:
    PRINT_MACRO4(i32_array_t, PRId32);
    break;
  case SIM_TYPE_INT64_ARR:
    PRINT_MACRO4(i64_array_t, PRId64);
    break;
  case SIM_TYPE_FLOAT_ARR:
    PRINT_MACRO3(float_array_t, f);
    break;
  case SIM_TYPE_DOUBLE_ARR:
    PRINT_MACRO3(double_array_t, f);
    break;
  case SIM_TYPE_COMPLEX_FLOAT_ARR: {
    complex_float_array_t *val = (complex_float_array_t*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [", field->name);
    ARRAY_FOR_EACH(i, *val) {
      fprintf(fout, "%f + %f I",
              crealf(ARRAY_ELEM(*val, i)), cimagf(ARRAY_ELEM(*val, i)));
      if (i != ARRAY_LEN(*val) - 1) {
        fprintf(fout, ", ");
      }
    }
    fprintf(fout, "]\n");
    break;
  }
  case SIM_TYPE_COMPLEX_DOUBLE_ARR: {
    complex_double_array_t *val = (complex_double_array_t*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [", field->name);
    ARRAY_FOR_EACH(i, *val) {
      fprintf(fout, "%f + %f I",
              creal(ARRAY_ELEM(*val, i)), cimag(ARRAY_ELEM(*val, i)));
      if (i != ARRAY_LEN(*val) - 1) {
        fprintf(fout, ", ");
      }
    }
    fprintf(fout, "]\n");
    break;
  }
  case SIM_TYPE_OBJ: {
    sim_object_t *sobj = *(sim_object_t**)((uintptr_t)obj + field->offs);
    uuid_string_t uuid_str;
    uuid_unparse(sobj->uuid, uuid_str);
    fprintf(fout, "\t%s : %s\n", field->name, uuid_str);
    break;
  }
  case SIM_TYPE_OBJ_ARR: {
    obj_array_t *val = (obj_array_t*)((uintptr_t)obj + field->offs);
    fprintf(fout, "\t\"%s\" : [", field->name);
    ARRAY_FOR_EACH(i, *val) {
      sim_object_t *sobj = ARRAY_ELEM(*val, i);
      uuid_string_t uuid_str;
      uuid_unparse(sobj->uuid, uuid_str);
      fprintf(fout, "%s", uuid_str);
      if (i != ARRAY_LEN(*val) - 1) {
        fprintf(fout, ", ");
      }
    }
    fprintf(fout, "]\n");
    break;
  }
  default:
    assert(0 && "invalid case");
  }
}

#if 0
static PyMethodDef OpenOrbitMethods[] = {
  {NULL, NULL, 0, NULL}
};

static PyModuleDef OpenOrbitModule = {
  PyModuleDef_HEAD_INIT, "openorbit", NULL, -1, OpenOrbitMethods,
  NULL, NULL, NULL, NULL
};

void
sim_py_init()
{
  PyObject *module = PyModule_Create(&OpenOrbitModule);
  if (module == NULL) {
    log_error("simpy: could not create python module");
  }

  PyObject *orbit_objects;
  PyObject *orbit_classes;

  if (PyModule_AddObject(module, "classes", orbit_classes == -1) {
    log_error("simpy: could not add 'classes' object");
  }

  if (PyModule_AddObject(module, "objects", orbit_objects == -1) {
    log_error("simpy: could not add 'objects' object");
  }

}


void
sim_py_set_prop()
{
  PyObject* PyObject_GetAttrString(PyObject *o, const char *attr_name)
  int PyObject_SetAttrString(PyObject *o, const char *attr_name, PyObject *v)
  PyObject* PyObject_CallObject(PyObject *callable_object, PyObject *args)
  PyObject* PyObject_CallMethod(PyObject *o, const char *method, const char *format, ...)

}
#endif

void
sim_print_object(FILE *fout, sim_object_t *obj)
{
  // TODO: Probably need to print the class name first
  fprintf(fout, "\"%s\" : {\n", obj->uuid);

  for (sim_class_t *cls = obj->cls ; cls != NULL ; cls = cls->super) {
    ARRAY_FOR_EACH(i, cls->fields) {
      sim_field_t *field = ARRAY_ELEM(cls->fields, i);
      sim_print_field(fout, obj, field);
    }
  }

  printf("}\n");
}

/*
  simstate : {
    "F68BBECC-D949-4FD5-B432-5BECAB5A0727" : {
      cls: "Spacecraft",
      name: "Mercury IX"
    }
  }
 */

void
sim_read_objects(FILE *fin)
{
  // TODO: Deserailize objects, should decide on fileformat first
}



