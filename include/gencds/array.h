/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit. Open Orbit is free software: you can
  redistribute it and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

  Some files of Open Orbit have relaxed licensing conditions. This file is
  licenced under the 2-clause BSD licence.

  Redistribution and use of this file in source and binary forms, with or
  without modification, are permitted provided that the following conditions are
  met:
 
    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
 
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ARRAY_H_QLY2MS7T
#define ARRAY_H_QLY2MS7T

#include <stdlib.h>
#include <stdint.h>
#include <err.h>
#include <sysexits.h>

typedef struct {
    size_t asize; //!< Size of elems array in objects
    size_t length; //!< Usage of elems array in objects
    void **elems;
} array_t;

// Arrays of pointers
void array_init(array_t *vec);
void array_compress(array_t *vec);

#define ARRAY_FOR_EACH(I, VEC) for (size_t I = 0 ; I < VEC.length ; I ++)
#define ARRAY_ELEM(VEC, I) (VEC).elems[I]
#define ARRAY_LEN(VEC) ((VEC).length)

#define DECL_ARRAY(typ,name)                                      \
  typedef struct {                                                \
    size_t asize;                                                 \
    size_t length;                                                \
    typ *elems;                                                   \
  } name##_array_t;                                               \
                                                                  \
  void name##_array_init(name##_array_t *vec);                    \
  void name##_array_push(name##_array_t *vec, typ obj);           \
  typ name##_array_pop(name##_array_t *vec);                      \
  typ name##_array_get(name##_array_t *vec, size_t i);            \
  void name##_array_set(name##_array_t *vec, size_t i, typ obj);  \
  typ name##_array_remove(name##_array_t *vec, size_t i);         \
  void name##_array_dispose(name##_array_t *vec);




#define DEF_ARRAY(typ,name)                                                    \
  void name##_array_init(name##_array_t *vec) {                                \
    vec->asize = 16;                                                           \
    vec->length = 0;                                                           \
    vec->elems = calloc(vec->asize, sizeof(void*));                            \
  }                                                                            \
  void name##_array_push(name##_array_t *vec, typ obj) {                       \
    if (vec->length >= vec->asize) {                                           \
      void *newVec = realloc(vec->elems,                                       \
                              vec->asize * sizeof(void*) * 2);                 \
      if (newVec == NULL) errx(EX_SOFTWARE, "realloc of vector failed");       \
      vec->asize *= 2;                                                         \
      vec->elems = newVec;                                                     \
    }                                                                          \
    vec->elems[vec->length ++] = obj;                                          \
  }                                                                            \
typ name##_array_pop(name##_array_t *vec) {                                    \
  assert(vec->length >= 1);                                                    \
  return vec->elems[--vec->length];                                            \
}                                                                              \
typ name##_array_get(name##_array_t *vec, size_t i) {                          \
  assert(i < vec->length);                                                     \
  if (vec->length <= i)                                                        \
    errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d",             \
         (int)vec->length, (int)i);                                            \
  else                                                                         \
    return vec->elems[i];                                                      \
}                                                                              \
void name##_array_set(name##_array_t *vec, size_t i, typ obj) {                \
  assert(i < vec->length);                                                     \
  if (vec->length <= i)                                                        \
    errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d",             \
         (int)vec->length, (int)i);                                            \
  else                                                                         \
    vec->elems[i] = obj;                                                       \
}                                                                              \
typ name##_array_remove(name##_array_t *vec, size_t i) {                       \
  assert(i < vec->length);                                                     \
  if (vec->length <= i)                                                        \
    errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d",             \
         (int)vec->length, (int)i);                                            \
  else {                                                                       \
    typ tmp = vec->elems[i];                                                   \
    vec->elems[i] = vec->elems[vec->length - 1];                               \
    vec->length --;                                                            \
    return tmp;                                                                \
  }                                                                            \
}                                                                              \
void name##_array_dispose(name##_array_t *vec) {                               \
  vec->asize = 0;                                                              \
  vec->length = 0;                                                             \
  free(vec->elems);                                                            \
  vec->elems = NULL;                                                           \
}                                                                              \

DECL_ARRAY(_Bool,bool);

DECL_ARRAY(char,char);
DECL_ARRAY(unsigned char,uchar);

DECL_ARRAY(short,short);
DECL_ARRAY(unsigned short,ushort);
DECL_ARRAY(int,int);
DECL_ARRAY(unsigned int,uint);
DECL_ARRAY(long,long);
DECL_ARRAY(unsigned long,ulong);

DECL_ARRAY(uint8_t,u8);
DECL_ARRAY(uint16_t,u16);
DECL_ARRAY(uint32_t,u32);
DECL_ARRAY(uint64_t,u64);

DECL_ARRAY(int8_t,i8);
DECL_ARRAY(int16_t,i16);
DECL_ARRAY(int32_t,i32);
DECL_ARRAY(int64_t,i64);

DECL_ARRAY(float,float);
DECL_ARRAY(double,double);

DECL_ARRAY(_Complex float,complex_float);
DECL_ARRAY(_Complex double,complex_double);

DECL_ARRAY(void*,obj);

#endif /* end of include guard: ARRAY_H_QLY2MS7T */
