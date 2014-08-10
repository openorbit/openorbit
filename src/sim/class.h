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

#ifndef orbit_class_h
#define orbit_class_h

#include <uuid/uuid.h>
#include <gencds/array.h>
#include <gencds/avl-tree.h>
#include "pubsub.h"

typedef struct sim_class_t {
  struct sim_class_t *super;

  const char *name;
  size_t obj_size; // Optional
  void *(*alloc)(struct sim_class_t *cls);
  void (*init)(struct sim_class_t *cls, void *obj, void *arg);
  void (*dealloc)(void *obj);
  void (*restored)(void *obj); // Called after deserialization (instead of init)

  obj_array_t fields;
  avl_tree_t *key_index_map;
  avl_tree_t *interfaces;
} sim_class_t;

typedef struct {
  sim_type_id_t typ;
  union {
    void *obj;
    bool boolean;
    char schar;
    unsigned char uchar;
    short sshort;
    unsigned short ushort;
    int sint;
    unsigned int uint;
    long slong;
    unsigned long ulong;

    float f;
    double d;
    complex float cf;
    complex double cd;

    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    float3 fv3;
    float4 fv4;

    uuid_t uuid;
    const char *str;

    struct {void *obj; void *iface;} iface;
    sim_class_t *class_ptr;
  };
} sim_prop_val_t;

typedef struct {
  sim_class_t *cls;
  uuid_t uuid;
  const char *name;
} sim_object_t;

typedef void (*sim_setter_fn)(sim_object_t *obj, sim_prop_val_t val);
typedef sim_prop_val_t (*sim_getter_fn)(sim_object_t *obj);

typedef struct {
  const char *name;
  sim_type_id_t typ;
  off_t offs;
  sim_setter_fn set;
  sim_getter_fn get;
} sim_field_t;

// Default class registry function
sim_class_t*
sim_register_class(const char *super, const char *name,
                   void (*init)(sim_class_t *cls, void *obj, void *arg),
                   size_t size);
sim_class_t*
sim_register_class_(const char *super, const char *name,
                    void *(*alloc)(sim_class_t *cls),
                    void (*init)(sim_class_t *cls, void *obj, void *arg),
                    void (*dealloc)(void *obj),
                    void (*restored)(void *obj));

sim_class_t* sim_class_get(const char *cls_name);


void sim_class_add_field(sim_class_t *class, sim_type_id_t type,
                         const char *key, off_t offset,
                         sim_setter_fn set, sim_getter_fn get);
sim_field_t* sim_class_get_field(sim_class_t *cls, const char *key);

void sim_class_add_interface(sim_class_t *class, const char *key,
                             void *iface);
void* sim_class_get_interface(sim_class_t *class, const char *key);


sim_object_t* sim_alloc_object(const char *cls_name);
void sim_init_object(sim_object_t *obj, const char *name, void *arg);
void sim_delete_object(sim_object_t *obj);

sim_prop_val_t
sim_get_field(sim_object_t *obj, const char *field_name);

void
sim_set_field(sim_object_t *obj, const char *field_name, sim_prop_val_t val);

// Sets a field, but uses an optional index to set a specific element in
// arrays, vectors and complex numbers.
void sim_set_field_by_index(sim_object_t *obj, const char *field_name,
                            size_t index, void *data);


void sim_print_object(FILE *fout, sim_object_t *obj);
void sim_read_objects(FILE *fin);

#define SIM_SUPER_INIT(c, o, a)                      \
  do {                                               \
    (c)->super->init(c->super, o, a);                \
  } while (0)


#endif
