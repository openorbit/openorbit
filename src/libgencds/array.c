
#include <gencds/array.h>
#include <err.h>
#include <sysexits.h>
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

void
array_push(array_t *vec, void *obj)
{
    if (vec->length >= vec->asize) {
        void *newVec = realloc(vec->elems,
                                vec->asize * sizeof(void*) * 2);
        if (newVec == NULL) errx(EX_SOFTWARE, "realloc of vector failed");
        vec->asize *= 2;
        vec->elems = newVec;
    }
    vec->elems[vec->length ++] = obj;
}


void*
array_pop(array_t *vec)
{    
    return vec->elems[vec->length --];
}

void*
array_get(array_t *vec, size_t i)
{
  if (vec->length <= i)
    errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d", vec->length, i);
  else
    return vec->elems[i];
}

void
array_set(array_t *vec, size_t i, void *obj)
{
  if (vec->length <= i)
    errx(EX_SOFTWARE, "vector out of bounds length = %d idx = %d", vec->length, i);
  else
    vec->elems[i] = obj;  
}
