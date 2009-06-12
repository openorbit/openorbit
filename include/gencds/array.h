#ifndef ARRAY_H_QLY2MS7T
#define ARRAY_H_QLY2MS7T

#include <stdlib.h>

typedef struct {
    size_t asize; //!< Size of elems array in objects
    size_t length; //!< Usage of elems array in objects
    void **elems;
} array_t;

void array_compress(array_t *vec);
void array_push(array_t *vec, void *obj);
void* array_pop(array_t *vec);
void* array_get(array_t *vec, size_t i);
void array_set(array_t *vec, size_t i, void *obj);

#define ARRAY_FOR_EACH(I, VEC) for (size_t I = 0 ; I < VEC.length ; I ++)


#endif /* end of include guard: ARRAY_H_QLY2MS7T */
