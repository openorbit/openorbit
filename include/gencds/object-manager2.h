#ifndef OBJECT_MANAGER2_H_VT1NTC32
#define OBJECT_MANAGER2_H_VT1NTC32
#include <openorbit/openorbit.h>

#include <string.h>

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
    u.a[i] = *key;
    key ++;
    if (*key == '\0') break;
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
  if (u.a[2] != 0) return u.a[2];
  if (u.a[3] != 0) return u.a[3];
  
  return 0; // Vectors are equal
}

typedef enum {
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
  
  OM_Object = sizeof(OOobject*),
  
  /* Vector types */
  OM_Float_Vec = OM_VEC|OM_FP|(sizeof(float)*4),
  
  OM_UInt_Vec = OM_VEC|sizeof(unsigned)*4,
  OM_Int_Vec =  OM_VEC|sizeof(int)*4,
  OM_Short_Vec =  OM_VEC|sizeof(short)*8,
  OM_UShort_Vec =  OM_VEC|sizeof(unsigned short)*8,
  OM_Byte_Vec =  OM_VEC|sizeof(uint8_t)*16
} OMpropkind;

typedef struct {
  char *name;
  OMpropkind type;
  size_t offset;
} OMproperty;

typedef struct {
  char *name;
  void *ifacePtr;
} OMinterface;

typedef struct {
  char *name;
  size_t objSize;
  
  size_t ifaceAlloc;
  size_t ifaceCount;
  OMinterface *ifaces;
  
  size_t propCountAlloc;
  size_t propCount;
  OMproperty props[];
} OMclass;

typedef struct {
  char *name;
  OMclass *cls;
  OOobject *obj;
} OMobjdesc;

typedef struct OMtreenode {
  struct OMtreenode *parent;
  struct OMtreenode *left;
  struct OMtreenode *right;
  OMobjdesc *objDesc;
  int8_t balanceFactor;
} OMtreenode;


void omAddIface(OMclass *cls, const char *name, void *iface);
void* omGetIface(OMclass *cls, const char *name);
void omAddProp(OMclass **cls, const char *name, OMpropkind typ, size_t offset);
void omTreeRotateLeft(OMtreenode *root);
void omTreeRotateRight(OMtreenode *root);
void omTreeRebalance(OMtreenode *node);
void omTreeInsert(OMtreenode *root, OMobjdesc *desc);
OMobjdesc* omTreeLookup(const OMtreenode *root, OOobject *obj);
void* omObjGetIface(OOobject *obj, const char *iface);
OOobject* omNewObject(const char *cls);

#endif /* end of include guard: OBJECT_MANAGER2_H_VT1NTC32 */
