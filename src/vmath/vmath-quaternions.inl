
scalar_t
q_scalar(const quaternion_t q)
{
    return q.s.w;
}

static inline vector_t 
q_vector(const quaternion_t q)
{
	vector_t r = (vector_t)q;
	r.s.w = S_ZERO;
	return r;
}

static inline quaternion_t
q_add(const quaternion_t a, const quaternion_t b)
{
    return a.v + b.v;
}


static inline quaternion_t
q_mul(const quaternion_t a, const quaternion_t b)
{
    r.s.x = QUAT_X(a.a)*QUAT_W(b.a) + QUAT_W(a.a)*QUAT_X(b.a)
		  + QUAT_Y(a.a)*QUAT_Z(b.a) - QUAT_Z(a.a)*QUAT_Y(b.a);
    r.s.y = QUAT_Y(a.a)*QUAT_W(b.a) + QUAT_W(a.a)*QUAT_Y(b.a)
		  + QUAT_Z(a.a)*QUAT_X(b.a) - QUAT_X(a.a)*QUAT_Z(b.a);
    r.s.z = QUAT_Z(a.a)*QUAT_W(b.a) + QUAT_W(a.a)*QUAT_Z(b.a)
          + QUAT_X(a.a)*QUAT_Y(b.a) - QUAT_Y(a.a)*QUAT_X(b.a);
    r.s.w = QUAT_W(a.a)*QUAT_W(b.a) - QUAT_X(a.a)*QUAT_X(b.a)
		  - QUAT_Y(a.a)*QUAT_Y(b.a) - QUAT_Z(a.a)*QUAT_Z(b.a);
}


static inline quaternion_t
q_s_div(const quaternion_t q, scalar_t d)
{
	vector_t dv = {.s.x = d, .s.y = d, .s.z = d, .s.w = d};
    return q.v / dv.v;
}

/* Note: maybe more efficient to xor the high bit of the vector component */
static inline quaternion_t
q_conj(const quaternion_t q)
{
	quaternion_t iq = {.s.x = -1.0, .s.y = -1.0, .s.z = -1.0, .s.w = 1.0};
	return q.v * iq.v;
}

/* Note: axis must have scalar component set to 1.0 */
static inline quaternion_t
q_rotv(vector_t axis, scalar_t alpha)
{
    scalar_t Omega = alpha * S_POINT_FIVE;
    scalar_t sin_Omega = sin(Omega);
	vector_t sincosv = {.s.x = sin_Omega, .s.y = sin_Omega,
						.s.z = sin_Omega, .s.w = cos(Omega)};

	return axis.v * sincosv.v;
}

static inline quaternion_t
q_rot(scalar_t x, scalar_t y, scalar_t z, scalar_t alpha)
{
	vector_t axis = {.s.x = x, .s.y = y, .s.z = z, .s.w = S_ONE};
	return q_rotv(axis, alpha);
}

