
#include "sim/pubsub.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
static bool notify_int_pass = false;
static bool notify_float_pass = false;
static bool notify_record_pass = false;

static void
notify_int(OOpubsubref ref)
{
  int a = simRetrieveInt(ref);
  assert(a == 42);
  notify_int_pass = true;
}

static void
notify_float(OOpubsubref ref)
{
  float a = simRetrieveFloat(ref);
  assert(a == 42.0f);
  notify_float_pass = true;
}

struct foo {
  int a, b, c;
};

static void
notify_record(OOpubsubref ref)
{
  struct foo *rec = simRetrieveRecord(ref);
  assert(rec->a == 42);
  assert(rec->b == 2);
  assert(rec->c == 3);
  notify_record_pass = true;
}


int
main(int argc, char **argv)
{
  int ival = 0;
  float fval = 0.0f;
  OOpubsubref iref = simPublishInt("myint", &ival);
  OOpubsubref fref = simPublishFloat("myfloat", &fval);

  simSubscribe(simQueryValueRef("myint"), notify_int);
  simSubscribe(simQueryValueRef("myfloat"), notify_float);

  ival = 42;
  simNotifyChange(iref);
  assert(notify_int_pass && "notify_int was not called");

  fval = 42.0f;
  simNotifyChange(fref);
  assert(notify_float_pass && "notify_float was not called");


  struct foo myrec = {1,2,3};
  OOrecordtyperef rtyp = simRegisterRecordType("foo");
  simRegisterRecordInt(rtyp, "a", offsetof(struct foo, a));
  simRegisterRecordInt(rtyp, "b", offsetof(struct foo, b));
  simRegisterRecordInt(rtyp, "b", offsetof(struct foo, c));
  OOpubsubref rref = simPublishRecord(rtyp, "myrec", &myrec);

  simSubscribe(simQueryValueRef("myrec"), notify_record);

  myrec.a = 42;
  simNotifyChange(rref);

  assert(notify_record_pass && "notify_record was not called");

  fprintf(stderr, "PASSED\n");
  return 0;
}
