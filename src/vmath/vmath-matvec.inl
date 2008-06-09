#include <vmath/vmath.h>

static inline scalar_t
v_dot(vector_t a, vector_t b) {
	vector_t vres;
    vres.v = a.v * b.v;
    return vres.s.x + vres.s.y + vres.s.z + vres.s.w;
}


static inline vector_t
m_v_mul(const matrix_t a, const vector_t v)
{
	vector_t vr;
	matrix_t p;
    p.v[0] = a.v[0] * v.v;
    p.v[1] = a.v[1] * v.v;
    p.v[2] = a.v[2] * v.v;
    p.v[3] = a.v[3] * v.v;
    
    vr.s.x = p.a[0][0] + p.a[0][1] + p.a[0][2] + p.a[0][3];
    vr.s.y = p.a[1][0] + p.a[1][1] + p.a[1][2] + p.a[1][3];
    vr.s.z = p.a[2][0] + p.a[2][1] + p.a[2][2] + p.a[2][3];
    vr.s.w = p.a[3][0] + p.a[3][1] + p.a[3][2] + p.a[3][3];

	return vr;
}

static inline matrix_t
m_add(const matrix_t a, const matrix_t b)
{
	matrix_t res;
	res.v[0] = a.v[0] + b.v[0];
    res.v[1] = a.v[1] + b.v[1];
    res.v[2] = a.v[2] + b.v[2];
    res.v[3] = a.v[3] + b.v[3];
	return res;
}

static inline vector_t
v_s_add(vector_t a, scalar_t b)
{
	vector_t bv = {.s.x = b,.s.y = b,.s.z = b,.s.w = b};
	return a.v + b.v;
}

static inline vector_t
v_add(vector_t a, vector_t b)
{
	return a.v + b.v;
}

static inline vector_t
v_sub(vector_t a, vector_t b)
{
	return a.v - b.v;
}


static inline vector_t
v_s_mul(vector_t a, scalar_t b)
{
	vector_t bv = {.s.x = b,.s.y = b,.s.z = b,.s.w = b};
	return a.v * b.v;
}

static inline vector_t
v_s_div(vector_t a, scalar_t b)
{
	vector_t bv = {.s.x = b,.s.y = b,.s.z = b,.s.w = b};
	return a.v / b.v;
}

static inline vector_t
v_set(scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
{
	vector_t v = {.s.x = v0,.s.y = v1,.s.z = v2,.s.w = v3};
	return v;
}
