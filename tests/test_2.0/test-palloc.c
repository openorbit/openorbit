// RUN: clang %s -o %t
// RUN: %t

#include "common/palloc.h"
#include <assert.h>

typedef struct {
  int a, b, c;
} foo_t;

int
main(int argc, char **argv)
{
  pool_t *pool = pinit(1024, sizeof(foo_t));
  assert(pool != NULL);

  // Allocate complete pool
  foo_t *foop1 = palloc(pool, 1024);
  assert(foop1 != NULL);

  // No more space in pool
  foo_t *foop2 = palloc(pool, 1);
  assert(foop2 == NULL);

  // Clear pool
  pclear(pool);
  foo_t *foop3 = palloc(pool, 1);
  assert(foop3 == foop1);

  // Pool free, note that the passing of the pool pointer here is passing an
  // invalid pointer since it has been freed, this test relies on the internals.
  pfree(pool);
  foo_t *foop4 = palloc(pool, 1);
  assert(foop4 == NULL);

  return 0;
}
