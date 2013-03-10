/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIM_PUBSUB_H
#define SIM_PUBSUB_H

/* We need to define a proper pub sub api for getting data references.
   In general, we need to be able to register two kinds of data, firstly
   singleton data, the second is object data, where an abstract object need to
   be created and queried.
 */
#include <stddef.h>
#include <vmath/vmath.h>
#include <gencds/array.h>
#include <gencds/hashtable.h>

typedef void* SIMpubsubref;

#define SIM_MAX_PUBSUB_COMP_KEY_LEN 256
#define SIM_MAX_PUBSUB_COMP_KEY_BUFF_SIZE (SIM_MAX_PUBSUB_COMP_KEY_LEN+1)

typedef enum {
  SIM_TYPE_RECORD = 0,
  SIM_TYPE_LINK, // Record alias

  SIM_TYPE_CLASS_PTR,
  SIM_TYPE_UUID,

  SIM_TYPE_OBJ,

  SIM_TYPE_BOOL,
  SIM_TYPE_CHAR,
  SIM_TYPE_UCHAR,
  SIM_TYPE_SHORT,
  SIM_TYPE_USHORT,
  SIM_TYPE_INT,
  SIM_TYPE_UINT,
  SIM_TYPE_LONG,
  SIM_TYPE_ULONG,
  SIM_TYPE_UINT8,
  SIM_TYPE_UINT16,
  SIM_TYPE_UINT32,
  SIM_TYPE_UINT64,
  SIM_TYPE_INT8,
  SIM_TYPE_INT16,
  SIM_TYPE_INT32,
  SIM_TYPE_INT64,
  SIM_TYPE_STR,
  SIM_TYPE_FLOAT,
  SIM_TYPE_DOUBLE,

  SIM_TYPE_COMPLEX_FLOAT,
  SIM_TYPE_COMPLEX_DOUBLE,

  SIM_TYPE_FLOAT_VEC3,
  SIM_TYPE_FLOAT_VEC4,
  SIM_TYPE_FLOAT_VEC3x3,
  SIM_TYPE_FLOAT_VEC4x4,

  // Common array types
  SIM_TYPE_BOOL_ARR,
  SIM_TYPE_CHAR_ARR,
  SIM_TYPE_UCHAR_ARR,
  SIM_TYPE_SHORT_ARR,
  SIM_TYPE_USHORT_ARR,
  SIM_TYPE_INT_ARR,
  SIM_TYPE_UINT_ARR,
  SIM_TYPE_LONG_ARR,
  SIM_TYPE_ULONG_ARR,

  SIM_TYPE_UINT8_ARR,
  SIM_TYPE_UINT16_ARR,
  SIM_TYPE_UINT32_ARR,
  SIM_TYPE_UINT64_ARR,

  SIM_TYPE_INT8_ARR,
  SIM_TYPE_INT16_ARR,
  SIM_TYPE_INT32_ARR,
  SIM_TYPE_INT64_ARR,

  SIM_TYPE_FLOAT_ARR,
  SIM_TYPE_DOUBLE_ARR,

  SIM_TYPE_COMPLEX_FLOAT_ARR,
  SIM_TYPE_COMPLEX_DOUBLE_ARR,

  SIM_TYPE_OBJ_ARR,
  SIM_TYPE_COUNT,
} sim_type_id_t;

typedef struct {
  sim_type_id_t type;
  const char *name;
} sim_base_t;

typedef struct {
  sim_base_t super;
  obj_array_t updateFuncs;
  union {
    bool *b;

    char *c;
    short *s;
    int *i;
    long *l;

    unsigned char *uc;
    unsigned short *us;
    unsigned int *ui;
    unsigned long *ul;

    char **str;
    uint8_t *u8;
    uint16_t *u16;
    uint32_t *u32;
    uint64_t *u64;
    int8_t *i8;
    int16_t *i16;
    int32_t *i32;
    int64_t *i64;
    float *f;
    double *d;

    _Complex float *cf;
    _Complex double *cd;

    float3 *fv3;
    float4 *fv4;
    float3x3 *fv9;
    float4x4 *fv16;

    void *blob;

    char_array_t *char_arr;
    uchar_array_t *uchar_arr;
    short_array_t *short_arr;
    ushort_array_t *ushort_arr;
    int_array_t *int_arr;
    uint_array_t *uint_arr;
    long_array_t *long_arr;
    ulong_array_t *ulong_arr;

    u8_array_t *u8_arr;
    u16_array_t *u16_arr;
    u32_array_t *u32_arr;
    u64_array_t *u64_arr;

    i8_array_t *i8_arr;
    i16_array_t *i16_arr;
    i32_array_t *i32_arr;
    i64_array_t *i64_arr;

    float_array_t *float_arr;
    double_array_t *double_arr;

    complex_float_array_t *complex_float_arr;
    complex_double_array_t *complex_double_arr;

    obj_array_t *obj_arr;
  };
} sim_value_t;

typedef struct {
  sim_base_t super;
  obj_array_t entries;
  hashtable_t *key_index_map;
} sim_record_t;

typedef struct {
  sim_base_t super;
  sim_record_t *target;
} sim_link_t;


#define SIM_TYPEDEF(N, T) \
  typedef struct {        \
    sim_value_t *ref;     \
    T val;                \
  } sim_ ## N ## _t


#define SIM_VAL(name) name.val
#define SIM_REF(name) name.ref

// Non typsafe accessors to ref vals (i.e. content of sim_value_t)

#define SIM_REF_BOOL(ref) *(ref->b)
#define SIM_REF_CHAR(ref) *(ref->c)
#define SIM_REF_UCHAR(ref) *(ref->uc)
#define SIM_REF_SHORT(ref) *(ref->s)
#define SIM_REF_USHORT(ref) *(ref->us)
#define SIM_REF_INT(ref) *(ref->i)
#define SIM_REF_UINT(ref) *(ref->ui)
#define SIM_REF_LONG(ref) *(ref->l)
#define SIM_REF_ULONG(ref) *(ref->ul)

#define SIM_REF_STR(ref) *(ref->str)

#define SIM_REF_UINT8(ref) *(ref->u8)
#define SIM_REF_UINT16(ref) *(ref->u16)
#define SIM_REF_UINT32(ref) *(ref->u32)
#define SIM_REF_UINT64(ref) *(ref->u64)

#define SIM_REF_INT8(ref) *(ref->i8)
#define SIM_REF_INT16(ref) *(ref->i16)
#define SIM_REF_INT32(ref) *(ref->i32)
#define SIM_REF_INT64(ref) *(ref->i64)

#define SIM_REF_FLOAT(ref) *(ref->f)
#define SIM_REF_DOUBLE(ref) *(ref->d)

#define SIM_REF_COMPLEX_FLOAT(ref) *(ref->cf)
#define SIM_REF_COMPLEX_DOUBLE(ref) *(ref->cd)

#define SIM_REF_FLOAT_VEC3(ref) *(ref->fv3)
#define SIM_REF_FLOAT_VEC4(ref) *(ref->fv4)
#define SIM_REF_FLOAT_VEC3x3(ref) *(ref->fv9)
#define SIM_REF_FLOAT_VEC4x4(ref) *(ref->fv16)

#define SIM_REF_BLOB(ref) ref->blob

#define SIM_REF_CHAR_ARRAY(ref) *(ref->char_arr)
#define SIM_REF_UCHAR_ARRAY(ref) *(ref->uchar_arr)

#define SIM_REF_SHORT_ARRAY(ref) *(ref->short_arr)
#define SIM_REF_USHORT_ARRAY(ref) *(ref->ushort_arr)

#define SIM_REF_INT_ARRAY(ref) *(ref->int_arr)
#define SIM_REF_UINT_ARRAY(ref) *(ref->uint_arr)

#define SIM_REF_LONG_ARRAY(ref) *(ref->long_arr)
#define SIM_REF_ULONG_ARRAY(ref) *(ref->ulong_arr)


#define SIM_REF_UINT8_ARRAY(ref) *(ref->u8_arr)
#define SIM_REF_UINT16_ARRAY(ref) *(ref->u16_arr)
#define SIM_REF_UINT32_ARRAY(ref) *(ref->u32_arr)
#define SIM_REF_UINT64_ARRAY(ref) *(ref->u64_arr)

#define SIM_REF_INT8_ARRAY(ref) *(ref->i8_arr)
#define SIM_REF_INT16_ARRAY(ref) *(ref->i16_arr)
#define SIM_REF_INT32_ARRAY(ref) *(ref->i32_arr)
#define SIM_REF_INT64_ARRAY(ref) *(ref->i64_arr)

#define SIM_REF_FLOAT_ARRAY(ref) *(ref->float_arr)
#define SIM_REF_DOUBLE_ARRAY(ref) *(ref->double_arr)

#define SIM_REF_COMPLEX_FLOAT_ARRAY(ref) *(ref->complex_float_arr)
#define SIM_REF_COMPLEX_DOUBLE_ARRAY(ref) *(ref->complex_double_arr)

#define SIM_REF_OBJ_ARRAY(ref) *(ref->obj_arr)

// C99 _Bool
SIM_TYPEDEF(bool,_Bool);

// C integer types
SIM_TYPEDEF(char,char);
SIM_TYPEDEF(uchar,unsigned char);
SIM_TYPEDEF(short,short);
SIM_TYPEDEF(ushort,unsigned short);
SIM_TYPEDEF(int,int);
SIM_TYPEDEF(uint,unsigned int);
SIM_TYPEDEF(long,long);
SIM_TYPEDEF(ulong,unsigned long);

// C99 uintN_t
SIM_TYPEDEF(uint8, uint8_t);
SIM_TYPEDEF(uint16, uint16_t);
SIM_TYPEDEF(uint32, uint32_t);
SIM_TYPEDEF(uint64, uint64_t);

// C99 intN_t
SIM_TYPEDEF(int8, int8_t);
SIM_TYPEDEF(int16, int16_t);
SIM_TYPEDEF(int32, int32_t);
SIM_TYPEDEF(int64, int64_t);

// C floating point types
SIM_TYPEDEF(float,float);
SIM_TYPEDEF(double,double);

// C99 complex types
SIM_TYPEDEF(complex_float,_Complex float);
SIM_TYPEDEF(complex_double,_Complex double);

// Vectors
SIM_TYPEDEF(float3,float3);
SIM_TYPEDEF(float4,float4);
SIM_TYPEDEF(float3x3,float3x3);
SIM_TYPEDEF(float4x4,float4x4);

// Our array types
SIM_TYPEDEF(bool_array, bool_array_t);
SIM_TYPEDEF(char_array, char_array_t);
SIM_TYPEDEF(uchar_array, uchar_array_t);
SIM_TYPEDEF(short_array, short_array_t);
SIM_TYPEDEF(ushort_array, ushort_array_t);
SIM_TYPEDEF(int_array, int_array_t);
SIM_TYPEDEF(uint_array, uint_array_t);
SIM_TYPEDEF(long_array, long_array_t);
SIM_TYPEDEF(ulong_array, ulong_array_t);

SIM_TYPEDEF(uint8_array, u8_array_t);
SIM_TYPEDEF(uint16_array, u16_array_t);
SIM_TYPEDEF(uint32_array, u32_array_t);
SIM_TYPEDEF(uint64_array, u64_array_t);

SIM_TYPEDEF(int8_array, i8_array_t);
SIM_TYPEDEF(int16_array, i16_array_t);
SIM_TYPEDEF(int32_array, i32_array_t);
SIM_TYPEDEF(int64_array, i64_array_t);

SIM_TYPEDEF(float_array, float_array_t);
SIM_TYPEDEF(double_array, double_array_t);

SIM_TYPEDEF(complex_float_array, complex_float_array_t);
SIM_TYPEDEF(complex_double_array, complex_double_array_t);
SIM_TYPEDEF(obj_array, obj_array_t);

// C strings
SIM_TYPEDEF(str, char*);

typedef void (*sim_valueobserver_fn_t)(sim_value_t *val);

sim_record_t* sim_pubsub_get_record_with_comps(const char *fst_comp, ...)
              __attribute__ ((sentinel));

sim_record_t* sim_pubsub_get_record(const char *path);
sim_value_t* sim_pubsub_get_value(const char *path);
sim_record_t* sim_pubsub_get_record_by_name(sim_record_t *rec, const char *name);
sim_record_t* sim_pubsub_get_record_by_index(sim_record_t *rec, int idx);
sim_value_t* sim_pubsub_get_value_by_index(sim_record_t *rec, int idx);
sim_value_t* sim_pubsub_get_value_by_name(sim_record_t *rec, const char *name);

sim_record_t* sim_pubsub_make_record(sim_record_t *parent, const char *name);
sim_value_t* sim_pubsub_make_value(sim_record_t *parent, sim_type_id_t typ,
                                const char *name, void *val);
sim_link_t* sim_pubsub_make_link(sim_record_t *parent, const char *name);

sim_record_t* sim_pubsub_create_record(const char *path);

void sim_pubsub_link_record(sim_record_t *parent, const char *key, sim_record_t *rec);

void sim_pubsub_publish_val(sim_record_t *parent, sim_type_id_t type, const char *key,
                     void *sim_val);
void sim_pubsub_observe_val(sim_value_t *val, sim_valueobserver_fn_t f);
void sim_pubsub_observe_named_val(const char *key, sim_valueobserver_fn_t f);
void sim_pubsub_observe_named_val_of_rec(sim_record_t *parent, const char *key,
                                  sim_valueobserver_fn_t f);
void sim_pubsub_notify_changed_val(void *sim_val);


void sim_pubsub_get_val(sim_value_t *val_desc, sim_type_id_t type_id, void *val);

void sim_pubsub_set_val(sim_value_t *val_desc, sim_type_id_t type_id, void *val);
/*
  Prints the contents of the pubsub database
 */
void sim_pubsub_dump_db(void);

#endif /* !SIM_PUBSUB_H */
