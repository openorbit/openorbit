/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>
 
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
#include <stdint.h>
#include <inttypes.h>
#include <complex.h>

#include "class.h"

#include <gencds/hashtable.h>
#include <gencds/avl-tree.h>
#include "log.h"

static hashtable_t *classes;
static avl_tree_t *objects;

static void
Object_init(void *obj, void *arg)
{
  sim_object_t *sobj = obj;
  uuid_generate(sobj->uuid);
  avl_insert(objects, sobj->uuid, sobj);
}

void
sim_class_init(void)
{
  classes = hashtable_new_with_str_keys(1024);
  sim_class_t *cls = sim_register_class(NULL, "Object", Object_init,
                                        sizeof(sim_object_t));
  sim_class_add_field(cls, SIM_TYPE_CLASS_PTR,
                      "cls", offsetof(sim_object_t, cls));
  sim_class_add_field(cls, SIM_TYPE_UUID,
                      "uuid", offsetof(sim_object_t, uuid));
  sim_class_add_field(cls, SIM_TYPE_STR,
                      "name", offsetof(sim_object_t, name));

  objects = avl_uuid_new();
}

sim_class_t*
sim_class_get(const char *cls_name)
{
  sim_class_t *cls = hashtable_lookup(classes, cls_name);
  ooLogFatalIfNull(cls, "unknown class '%s'", cls_name);
  return cls;
}

sim_class_t*
sim_register_class_(const char *super, const char *name,
                    void *(*alloc)(sim_class_t *cls),
                    void (*init)(void *obj, void *arg),
                    void (*dealloc)(void *obj),
                    void (*restored)(void *obj))
{
  sim_class_t *cls = malloc(sizeof(sim_class_t));
  
  if (super)  cls->super = sim_class_get(super);
  else cls->super = NULL;

  cls->name = strdup(name);
  cls->alloc = alloc;
  cls->init = init;
  if (dealloc) cls->dealloc = dealloc;
  else cls->dealloc = free;

  cls->restored = restored;

  obj_array_init(&cls->fields);
  cls->key_index_map = hashtable_new_with_str_keys(64);
  hashtable_insert(classes, name, cls);

  return cls;
}

static void*
def_alloc(sim_class_t *cls)
{
  void* obj = malloc(cls->obj_size);
  memset(obj, 0, cls->obj_size);
  return obj;
}

sim_class_t*
sim_register_class(const char *super, const char *name,
                   void (*init)(void *obj, void *arg),
                   size_t size)
{
  sim_class_t *cls = sim_register_class_(super, name,
                                         def_alloc, init, NULL, NULL);
  cls->obj_size = size;
  return cls;
}

void
sim_class_add_field(sim_class_t *class, sim_type_id_t type,
                    const char *key, off_t offset)
{
  sim_field_t *field = malloc(sizeof(sim_field_t));
  field->name = strdup(key);
  field->typ = type;
  field->offs = offset;
  
  obj_array_push(&class->fields, field);
  
  hashtable_insert(class->key_index_map, key,
                   (void*)(ARRAY_LEN(class->fields)));
}

sim_field_t*
sim_class_get_field(sim_class_t *cls, const char *key)
{
  while (cls) {
    uintptr_t idx = (uintptr_t) hashtable_lookup(cls->key_index_map, key);
    if (idx > 0) {
      return ARRAY_ELEM(cls->fields , idx-1);
    }
    cls = cls->super;
  }

  return NULL;
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
  obj->cls->init(obj, arg);
}

#define PRINT_MACRO(T, FMT)\
  {\
    T val = *(T*)((uintptr_t)obj + field->offs);\
    fprintf(fout, "\t\"%s\" : %" #FMT "\n", field->name, val);\
    break;\
  }
#define PRINT_MACRO2(T, FMT)\
  {\
    T val = *(T*)((uintptr_t)obj + field->offs);\
    fprintf(fout, "\t\"%s\" : %" FMT "\n", field->name, val);\
    break;\
  }

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
  case SIM_TYPE_CHAR:   PRINT_MACRO(char, c)
  case SIM_TYPE_UCHAR:  PRINT_MACRO(unsigned char, c)
  case SIM_TYPE_SHORT:  PRINT_MACRO(short, d)
  case SIM_TYPE_USHORT: PRINT_MACRO(unsigned short, u)
  case SIM_TYPE_INT:    PRINT_MACRO(int, d)
  case SIM_TYPE_UINT:   PRINT_MACRO(unsigned int, u)
  case SIM_TYPE_LONG:   PRINT_MACRO(long, ld)
  case SIM_TYPE_ULONG:  PRINT_MACRO(unsigned long, lu)
  case SIM_TYPE_UINT8:  PRINT_MACRO2(uint8_t, PRIu8)
  case SIM_TYPE_UINT16: PRINT_MACRO2(uint16_t, PRIu16)
  case SIM_TYPE_UINT32: PRINT_MACRO2(uint32_t, PRIu32)
  case SIM_TYPE_UINT64: PRINT_MACRO2(uint64_t, PRIu64)
  case SIM_TYPE_INT8:   PRINT_MACRO2(int8_t, PRId8)
  case SIM_TYPE_INT16:  PRINT_MACRO2(int16_t, PRId16)
  case SIM_TYPE_INT32:  PRINT_MACRO2(int32_t, PRId32)
  case SIM_TYPE_INT64:  PRINT_MACRO2(int64_t, PRId64)
  case SIM_TYPE_STR:    PRINT_MACRO(char*, s)
  case SIM_TYPE_FLOAT:  PRINT_MACRO(float, f)
  case SIM_TYPE_DOUBLE: PRINT_MACRO(double, f)
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
  case SIM_TYPE_BOOL_ARR:
    break;
  case SIM_TYPE_CHAR_ARR:
    break;
  case SIM_TYPE_UCHAR_ARR:
    break;
  case SIM_TYPE_SHORT_ARR:
    break;
  case SIM_TYPE_USHORT_ARR:
    break;
  case SIM_TYPE_INT_ARR:
    break;
  case SIM_TYPE_UINT_ARR:
    break;
  case SIM_TYPE_LONG_ARR:
    break;
  case SIM_TYPE_ULONG_ARR:
    break;
  case SIM_TYPE_UINT8_ARR:
    break;
  case SIM_TYPE_UINT16_ARR:
    break;
  case SIM_TYPE_UINT32_ARR:
    break;
  case SIM_TYPE_UINT64_ARR:
    break;
  case SIM_TYPE_INT8_ARR:
    break;
  case SIM_TYPE_INT16_ARR:
    break;
  case SIM_TYPE_INT32_ARR:
    break;
  case SIM_TYPE_INT64_ARR:
    break;
  case SIM_TYPE_FLOAT_ARR:
    break;
  case SIM_TYPE_DOUBLE_ARR:
    break;
  case SIM_TYPE_COMPLEX_FLOAT_ARR:
    break;
  case SIM_TYPE_COMPLEX_DOUBLE_ARR:
    break;
  case SIM_TYPE_OBJ_ARR:
    break;
  default:
    assert(0 && "invalid case");
  }
}

void
sim_print_object(FILE *fout, sim_object_t *obj)
{
  fprintf(fout, "\"%s\" : {\n", obj->name);
  
  for (sim_class_t *cls = obj->cls ; cls != NULL ; cls = cls->super) {
    ARRAY_FOR_EACH(i, cls->fields) {
      sim_field_t *field = ARRAY_ELEM(cls->fields, i);
      sim_print_field(fout, obj, field);
    }
  }

  printf("}\n");
}
