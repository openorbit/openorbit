/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


#ifndef OBJECT_MANAGER2_H_VT1NTC32
#define OBJECT_MANAGER2_H_VT1NTC32

#include <gencds/hashtable.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define OM_SIZE_MASK 0x3f
#define OM_FP   0x00000040
#define OM_SIGN 0x00000080
#define OM_PTR  0x00000100
#define OM_ARR  0x00000200
#define OM_VEC  0x00000400
#define OM_ARRLEN_MASK 0xffff0000

typedef char __attribute__((vector_size (16))) v16c_t;
typedef v16c_t OMkey;

static inline OMkey
omMakeKey(const char *key)
{
  union {
    v16c_t v;
    char a[16];
  } u;
  
  memset(&u.v, 0, sizeof(v16c_t));
  
  for (size_t i = 0; i < sizeof(v16c_t) ; i ++) {
    if (key[i] == '\0') break;
    u.a[i] = key[i];
  }
  
  return u.v;
}

static inline int32_t
omKeyCmp(OMkey a, OMkey b)
{
  // This is a very general vector comparison, it is valid for checking
  // whether two vectors are ordered or not in big endian order.
  // On the other hand, ordering in this case will only be a permutation of
  // normal order, so it is working, but you cannot easily see the sorting by
  // looking on the keys
  union {
    OMkey v;
    int32_t a[2];
  } u;
  
  u.v = a - b;

  if (u.a[0] != 0) return u.a[0];
  if (u.a[1] != 0) return u.a[1];
//  if (u.a[2] != 0) return u.a[2];
//  if (u.a[3] != 0) return u.a[3];
  
  return 0; // Vectors are equal
}

typedef enum OMpropkind {
  OM_Byte = sizeof(uint8_t),
  OM_UChar = sizeof(unsigned char),
  OM_Char = sizeof(char) | OM_SIGN,
  
  OM_Short = sizeof(short) | OM_SIGN,
  OM_Int16 = sizeof(int16_t) | OM_SIGN,
  OM_UShort = sizeof(unsigned short),
  OM_UInt16 = sizeof(uint16_t),
  
  OM_Int = sizeof(int) | OM_SIGN,
  OM_Int32 = sizeof(int32_t) | OM_SIGN,
  OM_UInt = sizeof(unsigned int),
  OM_UInt32 = sizeof(uint32_t),
  
  OM_Long = sizeof(long) | OM_SIGN,
  OM_Ulong = sizeof(unsigned long),
  
  OM_Int64 = sizeof(int64_t) | OM_SIGN,
  OM_UInt64 = sizeof(uint64_t),
  
  OM_Float = sizeof(float) | OM_FP,
  OM_Double = sizeof(double) | OM_FP,
  OM_Long_Double = sizeof(long double) | OM_FP,  
  
  OM_Object = sizeof(void*) | OM_PTR,
  
  /* Vector types */
  OM_Float_Vec = OM_VEC|OM_FP|(sizeof(float)*4),
  
  OM_UInt_Vec = OM_VEC|sizeof(unsigned)*4,
  OM_Int_Vec =  OM_VEC|sizeof(int)*4,
  OM_Short_Vec =  OM_VEC|sizeof(short)*8,
  OM_UShort_Vec =  OM_VEC|sizeof(unsigned short)*8,
  OM_Byte_Vec =  OM_VEC|sizeof(uint8_t)*16
} OMpropkind;

typedef struct OMproperty {
  char *name;
  OMpropkind type;
  size_t offset;
} OMproperty;

typedef struct OMinterface {
  char *name;
  void *ifacePtr;
} OMinterface;

typedef struct OMclass {
  char *name;
  size_t objSize;
  
  size_t ifaceAlloc;
  size_t ifaceCount;
  OMinterface *ifaces;
  
  size_t propAlloc;
  size_t propCount;
  OMproperty *props;
} OMclass;

typedef struct OMobjdesc {
  char *name;
  OMclass *cls;
  void *obj;
} OMobjdesc;

typedef struct OMtreenode {
//  struct OMtreenode *parent;
  struct OMtreenode *left;
  struct OMtreenode *right;
  uintptr_t key; // Can be any integer, also a pointer
  void *data;
  int8_t balance; // Constrained to +/-1
} OMtreenode;

typedef struct OMtree {
  OMtreenode *root;
} OMtree;

typedef struct OMcontext {
  hashtable_t *classes;
  OMtree *objects;
} OMcontext;

OMcontext* omCtxtNew(void);

OMtree* omTreeNew(void);
OMobjdesc* omObjDescNew(const char *name, OMclass *cls, void *obj);

void omAddIface(OMclass *cls, const char *name, void *iface);
void* omGetIface(OMclass *cls, const char *name);
void omAddProp(OMclass *cls, const char *name, OMpropkind typ, size_t offset);
void omTreeRotateLeft(OMtreenode *root);
void omTreeRotateRight(OMtreenode *root);
void omTreeRebalance(OMtreenode *node);
void omTreeInsert(OMtree *tree, uintptr_t key, void *data);
OMobjdesc* omTreeLookup(const OMtreenode *root, void *obj);

void omDbgDumpTree(FILE *file, OMtree *root);


void* omObjGetIface(void *obj, const char *iface);
void* omNewObject(OMcontext *ctxt, const char *cls);
OMclass* omNewClass(OMcontext *ctxt, const char *clsName, size_t objSize);

void omAVLInsert(OMtree *tree, OMtreenode *node);
void omTreeInsert2(OMtree *tree, uintptr_t key, void *data);

#endif /* end of include guard: OBJECT_MANAGER2_H_VT1NTC32 */
