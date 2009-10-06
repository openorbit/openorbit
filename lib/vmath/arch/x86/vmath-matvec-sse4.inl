#ifdef __SSE4__

#include <smmintrin.h>


static inline scalar_t
v_dot(const vector_t a, const vector_t b) {
	vector_t res;
	
	res.v = _mm_dp_ps( a.v, b.v, 0xf8);

    return res.s.x;
}

static inline vector_t
m_v_mul(const matrix_t a, const vector_t v) {
	vector_t res;
	tmp.v = _mm_dp_ps( a.v[1], v.v, 0xf4);
	res.v = res.v + tmp.v;
	tmp.v = _mm_dp_ps( a.v[2], v.v, 0xf2);
	res.v = res.v + tmp.v;
	tmp.v = _mm_dp_ps( a.v[3], v.v, 0xf1);
	res.v = res.v + tmp.v;
	
	return res;
}


#endif /* __SSE4__ */