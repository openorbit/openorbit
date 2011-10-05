/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include <gencds/array.h>
#include <err.h>
#include <sysexits.h>
#include <stdint.h>
#include <assert.h>
void
array_init(array_t *vec)
{
    vec->asize = 16;
    vec->length = 0;
    vec->elems = calloc(vec->asize, sizeof(void*));
}

void
array_compress(array_t *vec)
{
  // Remove last NULL pointers
  for (size_t i = vec->length; i > 0; i --) {
    if (vec->elems[i-1] != NULL) {
      vec->length = i;
      break;
    }
  }

  // Move pointers at end of vector to first free entries
  for (size_t i = 0; i < vec->length ; i ++) {
    if (vec->elems[i] == NULL) {
      vec->elems[i] = vec->elems[vec->length-1];
      vec->length --;
    }

    // Remove any unused space at the end of the vector
    for (size_t j = vec->length; j > 0; j --) {
      if (vec->elems[j-1] != NULL) {
        vec->length = j;
        break;
      }
    }

  }
}

DEF_ARRAY(_Bool,bool);

DEF_ARRAY(char,char);
DEF_ARRAY(unsigned char,uchar);
DEF_ARRAY(short,short);
DEF_ARRAY(unsigned short,ushort);
DEF_ARRAY(int,int);
DEF_ARRAY(unsigned int,uint);
DEF_ARRAY(long,long);
DEF_ARRAY(unsigned long,ulong);

DEF_ARRAY(uint8_t,u8);
DEF_ARRAY(uint16_t,u16);
DEF_ARRAY(uint32_t,u32);
DEF_ARRAY(uint64_t,u64);

DEF_ARRAY(int8_t,i8);
DEF_ARRAY(int16_t,i16);
DEF_ARRAY(int32_t,i32);
DEF_ARRAY(int64_t,i64);


DEF_ARRAY(float,float);
DEF_ARRAY(double,double);
DEF_ARRAY(_Complex float,complex_float);
DEF_ARRAY(_Complex double,complex_double);

DEF_ARRAY(void*,obj);

