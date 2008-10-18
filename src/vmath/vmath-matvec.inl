/* Note that this is an umbrella file as well as a default definition file, there
might be more architectural specialised files further down from here
*/

#ifndef VM_V4_NEG
#define VM_V4_NEG

static inline vector_t
v_neg(vector_t v)
{
    return (vector_t)-v.v;
}
#endif /* VM_V4_NEG */

#ifndef VM_V4_DOT
#define VM_V4_DOT

static inline scalar_t
v_dot(vector_t a, vector_t b) {
	vector_t vres;
    vres.v = a.v * b.v;
    return vres.x + vres.y + vres.z + vres.w;
}

#endif /* VM_V4_DOT */


#ifndef VM_M4_MUL
#define VM_M4_MUL
void m_transpose(matrix_t *bT, const matrix_t *b);
static inline void
m_mul(matrix_t *res, const matrix_t *a, const matrix_t *b)
{
    matrix_t tmp_res, bT;
    m_transpose(&bT, b);
    for (int i = 0 ; i < 4 ; i ++) {
        for (int j = 0; j < 4 ; j ++) {
            tmp_res.a[i][j] = v_dot((vector_t)a->v[i], (vector_t)bT.v[j]);
        }
    }
    *res = tmp_res;
}
#endif

#ifndef VM_M4_V4_MUL
#define VM_M4_V4_MUL

static inline vector_t
m_v_mul(const matrix_t *a, const vector_t v)
{
	vector_t vr;
	matrix_t p;
    p.v[0] = a->v[0] * v.v;
    p.v[1] = a->v[1] * v.v;
    p.v[2] = a->v[2] * v.v;
    p.v[3] = a->v[3] * v.v;
    
    vr.x = p.a[0][0] + p.a[0][1] + p.a[0][2] + p.a[0][3];
    vr.y = p.a[1][0] + p.a[1][1] + p.a[1][2] + p.a[1][3];
    vr.z = p.a[2][0] + p.a[2][1] + p.a[2][2] + p.a[2][3];
    vr.w = p.a[3][0] + p.a[3][1] + p.a[3][2] + p.a[3][3];

	return vr;
}
#endif /*VM_M4_V4_MUL*/

#if 0
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
	vector_t b = {.a = {b,b,b,b}};
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
	vector_t b = {.a = {b,b,b,b}};
	return a.v * b.v;
}

static inline vector_t
v_s_div(vector_t a, scalar_t b)
{
	vector_t b = {.a = {b,b,b,b}};
	return a.v / b.v;
}

static inline vector_t
v_set(scalar_t v0, scalar_t v1, scalar_t v2, scalar_t v3)
{
	vector_t v = {.a = {v0,v1,v2,v3}};
	return v;
}
#endif