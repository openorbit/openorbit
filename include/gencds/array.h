#ifndef ARRAY_H_QLY2MS7T
#define ARRAY_H_QLY2MS7T

#include <stdlib.h>
#include <stdint.h>
typedef struct {
    size_t asize; //!< Size of elems array in objects
    size_t length; //!< Usage of elems array in objects
    void **elems;
} array_t;

// Arrays of pointers
void array_init(array_t *vec);
void array_compress(array_t *vec);

#define ARRAY_FOR_EACH(I, VEC) for (size_t I = 0 ; I < VEC.length ; I ++)


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
  void name##_array_set(name##_array_t *vec, size_t i, typ obj)

DECL_ARRAY(int,int);
DECL_ARRAY(uint64_t,u64);
DECL_ARRAY(float,float);
DECL_ARRAY(double,double);
DECL_ARRAY(void*,obj);

#endif /* end of include guard: ARRAY_H_QLY2MS7T */
