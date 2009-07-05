
#include <gencds/array.h>
#include <err.h>
#include <sysexits.h>
#include <stdint.h>
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



#define DEF_ARRAY(typ,name)                                                           \
  void name##_array_init(name##_array_t *vec) {                                       \
    vec->asize = 16;                                                                  \
    vec->length = 0;                                                                  \
    vec->elems = calloc(vec->asize, sizeof(void*));                                   \
  }                                                                                   \
  void name##_array_push(name##_array_t *vec, typ obj) {                              \
    if (vec->length >= vec->asize) {                                                  \
        void *newVec = realloc(vec->elems,                                            \
                                vec->asize * sizeof(void*) * 2);                      \
        if (newVec == NULL) errx(EX_SOFTWARE, "realloc of vector failed");            \
        vec->asize *= 2;                                                              \
        vec->elems = newVec;                                                          \
    }                                                                                 \
    vec->elems[vec->length ++] = obj;                                                 \
  }                                                                                   \
  typ name##_array_pop(name##_array_t *vec) {                                         \
    return vec->elems[vec->length --];                                                \
  }                                                                                   \
  typ name##_array_get(name##_array_t *vec, size_t i) {                               \
    if (vec->length <= i)                                                             \
      errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d", vec->length, i); \
    else                                                                              \
      return vec->elems[i];                                                           \
    }                                                                                 \
  void name##_array_set(name##_array_t *vec, size_t i, typ obj) {                     \
    if (vec->length <= i)                                                             \
      errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d", vec->length, i); \
    else                                                                              \
      vec->elems[i] = obj;                                                            \
  }


DEF_ARRAY(int,int);
DEF_ARRAY(uint64_t,u64);
DEF_ARRAY(float,float);
DEF_ARRAY(double,double);
DEF_ARRAY(void*,obj);

