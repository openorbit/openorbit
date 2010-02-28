#ifndef VMATH_INTEGER_H_8UWYEZ4B
#define VMATH_INTEGER_H_8UWYEZ4B

static inline int16_t
v3s_get(short3 s, int i)
{
  union {
    int16_t a[3];
    short3 v;
  } u;
  u.v = s;
  return u.a[i];
}

static inline int32_t
v3i_get(int3 s, int i)
{
  union {
    int32_t a[3];
    int3 v;
  } u;
  u.v = s;
  return u.a[i];
}

static inline int32_t
v3i_x(int3 s)
{
  return s.x;
}

static inline int32_t
v3i_y(int3 s)
{
  return s.y;
}

static inline int32_t
v3i_z(int3 s)
{
  return s.z;
}


static inline int64_t
v3l_get(long3 s, int i)
{
  union {
    int64_t a[3];
    long3 v;
  } u;
  u.v = s;
  return u.a[i];
}


static inline short3
vs3_set(int16_t a, int16_t b, int16_t c)
{
  union {
    short3 v;
    struct {
      int16_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

static inline int3
vi3_set(int32_t a, int32_t b, int32_t c)
{
  union {
    int3 v;
    struct {
      int32_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

static inline long3
vl3_set(int64_t a, int64_t b, int64_t c)
{
  union {
    long3 v;
    struct {
      int64_t x, y, z;
    } s;
  } u;

  u.s.x = a;
  u.s.y = b;
  u.s.z = c;

  return u.v;
}

#endif /* end of include guard: VMATH_INTEGER_H_8UWYEZ4B */
