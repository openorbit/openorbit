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
  void (*init)(void *obj, void *arg);
  void (*dealloc)(void *obj);
  void (*restored)(void *obj); // Called after deserialization (instead of init)
  
  obj_array_t fields;
  avl_tree_t *key_index_map;
  avl_tree_t *interfaces;
} sim_class_t;

typedef struct {
  const char *name;
  sim_type_id_t typ;
  off_t offs;
} sim_field_t;

typedef struct {
  sim_class_t *cls;
  uuid_t uuid;
  const char *name;
} sim_object_t;

// Default class registry function
sim_class_t*
sim_register_class(const char *super, const char *name,
                   void (*init)(void *obj, void *arg),
                   size_t size);
sim_class_t*
sim_register_class_(const char *super, const char *name,
                    void *(*alloc)(sim_class_t *cls),
                    void (*init)(void *obj, void *arg),
                    void (*dealloc)(void *obj),
                    void (*restored)(void *obj));


void sim_class_add_field(sim_class_t *class, sim_type_id_t type,
                         const char *key, off_t offset);
sim_field_t* sim_class_get_field(sim_class_t *cls, const char *key);

void sim_class_add_interface(sim_class_t *class, const char *key,
                             void *iface);
void* sim_class_get_interface(sim_class_t *class, const char *key);


sim_object_t* sim_alloc_object(const char *cls_name);
void sim_init_object(sim_object_t *obj, const char *name, void *arg);
void sim_delete_object(sim_object_t *obj);

// Set a field in an object. Len is ignored for scalars, otherwise, it is the
// length of the array pointed to by data.
void sim_set_field(sim_object_t *obj, const char *field_name,
                   size_t len, void *data);
// Sets a field, but uses an optional index to set a specific element in
// arrays, vectors and complex numbers.
void sim_set_field_by_index(sim_object_t *obj, const char *field_name,
                            size_t index, void *data);


void sim_print_object(FILE *fout, sim_object_t *obj);
void sim_read_objects(FILE *fin);

#define SIM_SUPER_INIT(o, a)                      \
  do {                                            \
    ((sim_object_t*)(o))->cls->super->init(o, a); \
  } while (0)


#endif
