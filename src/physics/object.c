
/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "physics.h"
#include "object.h"
#include <vmath/lwcoord.h>
#include <openorbit/log.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
  One of the problems with space is that it is big

  For our physics system, this unfortunatelly means that we need to reset the position
  based on the local segmented position after every step. This is OK in general, but
  will cause big problems for collission detection. Though, since the data needs to be
  converted to a single segment for rendering, it may be possible to use that data for
  colission detection.
*/
void
pl_object_compute_derived(pl_object_t *obj)
{
  qd_md3_convert(obj->R, obj->q); // Convert our quaternion to rotation matrix
  md3_basis(obj->I_inv_world, obj->m.I_inv, obj->R);
}
void
pl_object_init(pl_object_t *obj)
{
  pl_mass_set(&obj->m, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f);

  lwc_set(&obj->p, 0.0, 0.0, 0.0);

  obj->world = NULL;
  obj->name = NULL;
  obj->parent = NULL;

  obj->f_ack = vd3_set(0.0, 0.0, 0.0);
  obj->t_ack = vd3_set(0.0, 0.0, 0.0);
  obj->g_ack = vd3_set(0.0, 0.0, 0.0);

  obj->v = vd3_set(0.0, 0.0, 0.0);
  pl_object_set_angular_vel3f(obj, 0.0f, 0.0f, 0.0f);

  obj->q = qd_rot(1.0, 0.0, 0.0, 0.0); // Rotation quaternion
  obj->p_offset = vd3_set(0.0, 0.0, 0.0);

  obj->airPressure = 0.0;
  obj->dragCoef = 0.0;
  obj->area = 0.0;
  obj->radius = 1.0;

  obj_array_init(&obj->children);
  obj_array_init(&obj->psystem);
  obj_array_init(&obj->aerofoils);

  pl_object_compute_derived(obj);
}

pl_object_t*
pl_new_object(pl_world_t *world, const char *name)
{
  assert(world != NULL);

  pl_object_t *obj = calloc(1, sizeof(pl_object_t));
  pl_object_init(obj);
  obj->name = strdup(name);
  obj->world = world;

  obj_array_push(&world->rigid_bodies, obj);
  obj_array_push(&world->root_bodies, obj);
  pl_object_compute_derived(obj);

  pl_collide_insert_object(world->coll_ctxt, obj);
  pl_octtree_insert_rbody(world->octtree, obj);
  return obj;
}

void
pl_delete_object(pl_object_t *obj)
{
  pl_world_t *world = obj->world;
  if (obj->parent) {
    pl_object_detatch(obj);
  }

  ARRAY_FOR_EACH(i, world->rigid_bodies) {
    if (ARRAY_ELEM(world->rigid_bodies, i) == obj) {
      obj_array_remove(&world->rigid_bodies, i);
    }
  }

  ARRAY_FOR_EACH(i, world->root_bodies) {
    if (ARRAY_ELEM(world->root_bodies, i) == obj) {
      obj_array_remove(&world->root_bodies, i);
    }
  }

  // BUG: Must also be removed from collission context and octtree
  free(obj);
}

pl_object_t*
pl_new_sub_object3f(pl_world_t *world, pl_object_t *parent, const char * name,
              float x, float y, float z)
{
  assert(parent != NULL);

  pl_object_t *obj = calloc(1, sizeof(pl_object_t));
  pl_object_init(obj);
  obj->name = strdup(name);
  obj->world = parent->world;
  obj->parent = parent;

  obj->p_offset = vd3_set(x, y, z);
  obj_array_push(&world->rigid_bodies, obj);
  obj_array_push(&parent->children, obj);
  return obj;
}

void
pl_object_detatch(pl_object_t *obj)
{
  assert(obj != NULL);
  assert(obj->parent != NULL);
  //pl_world_t *world = obj->sys->world;
  pl_object_t *parent = obj->parent;

  obj->parent = NULL;
  //obj->sys = parent->sys;
  for (int i = 0 ; i < parent->children.length ; ++i) {
    if (parent->children.elems[i] == obj) {
      obj_array_remove(&parent->children, i);
      obj_array_push(&parent->world->root_bodies, obj);
      break;
    }
  }

  pl_object_update_mass(parent);
  pl_object_compute_derived(obj);
  pl_collide_insert_object(obj->world->coll_ctxt, obj);
  pl_octtree_insert_rbody(obj->world->octtree, obj);
}

void
pl_object_update_mass(pl_object_t *obj)
{
  memset(&obj->m, 0, sizeof(pl_mass_t));

  pl_mass_set(&obj->m, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0);

  for (int i = 0 ; i < obj->children.length ; ++ i) {
    pl_object_t *child = obj->children.elems[i];
    pl_mass_t tmp = child->m;
    pl_mass_translate(&tmp,
                    child->p_offset.x,
                    child->p_offset.y,
                    child->p_offset.z);
    pl_mass_add(&obj->m, &tmp);
  }
  pl_object_compute_derived(obj);
}


void
pl_object_set_pos3d(pl_object_t *obj, double x, double y, double z)
{
  PL_CHECK_OBJ(obj);

  lwc_set(&obj->p, x, y, z);

  PL_CHECK_OBJ(obj);
}


void
pl_object_set_pos_ext3f(pl_object_t *obj,
                    int32_t i, int32_t j, int32_t k,
                    float x, float y, float z)
{
  PL_CHECK_OBJ(obj);

  obj->p.seg = vl3_set(i, j, k);
  obj->p.offs = vd3_set(x, y, z);
  lwc_normalise(&obj->p);

  PL_CHECK_OBJ(obj);
}

void
pl_object_set_pos_rel3d(pl_object_t * restrict obj, const pl_object_t * restrict otherObj,
                    double x, double y, double z)
{
  PL_CHECK_OBJ(obj);

  obj->p = otherObj->p;
  lwc_translate3f(&obj->p, x, y, z);

  PL_CHECK_OBJ(obj);
}

void
pl_object_set_pos_rel3fv(pl_object_t * restrict obj,
                     const pl_object_t * restrict otherObj,
                     float3 rp)
{
  PL_CHECK_OBJ(obj);

  obj->p = otherObj->p;
  lwc_translate3fv(&obj->p, rp);
  lwc_dump(&otherObj->p);
  lwc_dump(&obj->p);

  PL_CHECK_OBJ(obj);
}

void
pl_object_set_pos_celobj_rel(pl_object_t * restrict obj,
                             pl_celobject_t * restrict otherObj,
                             double3 rp)
{
  PL_CHECK_OBJ(obj);

  double3 celobj_p = otherObj->cm_orbit->p;
  lwc_set(&obj->p, celobj_p.x, celobj_p.y, celobj_p.z);
  lwc_translate3dv(&obj->p, rp);
  lwc_dump(&obj->p);

  obj->dominator = otherObj;

  PL_CHECK_OBJ(obj);
}

void
pl_object_force3f(pl_object_t *obj, float x, float y, float z)
{
  while (obj->parent) obj = obj->parent;

  PL_CHECK_OBJ(obj);

  obj->f_ack.x += x;
  obj->f_ack.y += y;
  obj->f_ack.z += z;

  PL_CHECK_OBJ(obj);
}
void
pl_object_force3fv(pl_object_t *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;

  double3 fd = vf3_to_vd3(f);

  PL_CHECK_OBJ(obj);

  obj->f_ack += fd;

  PL_CHECK_OBJ(obj);
}


void
pl_object_force_relative3f(pl_object_t *obj, float fx, float fy, float fz)
{
  while (obj->parent) obj = obj->parent;
  PL_CHECK_OBJ(obj);

  double3 f = vd3_set(fx, fy, fz);
  double3 f_rot = md3_v_mul(obj->R, f);
  obj->f_ack += f_rot;
}

void
pl_force_relative3fv(pl_object_t *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;
  PL_CHECK_OBJ(obj);

  double3 fd = vf3_to_vd3(f);
  double3 f_rot = md3_v_mul(obj->R, fd);
  obj->f_ack += f_rot;
}

void
pl_object_force_relative_pos3f(pl_object_t *obj,
                     float fx, float fy, float fz,
                     float px, float py, float pz)
{
  PL_CHECK_OBJ(obj);

  double3 f = vd3_set(fx, fy, fz);
  double3 p = vd3_set(px, py, pz);

  while (obj->parent) { p += obj->p_offset; obj = obj->parent; }

  double3 f_rot = md3_v_mul(obj->R, f);
  double3 t_rot = vd3_cross(md3_v_mul(obj->R, p - obj->m.cog), f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}

void
pl_object_torque3f(pl_object_t *obj, float tx, float ty, float tz)
{
  obj->t_ack += vd3_set(tx, ty, tz);
}

void
pl_object_torque3fv(pl_object_t *obj, float3 t)
{
  obj->t_ack += vf3_to_vd3(t);
}

void
pl_object_torque_relative3f(pl_object_t *obj, float tx, float ty, float tz,
                   float px, float py, float pz)
{
  obj->t_ack += md3_v_mul(obj->R, vd3_set(tx, ty, tz));
}

void
pl_object_torque_relative3fv(pl_object_t *obj, float3 t, float3 p)
{
  obj->t_ack += md3_v_mul(obj->R, vf3_to_vd3(t));
}



void
pl_object_force_relative_pos3fv(pl_object_t *obj, float3 f, float3 p)
{
  double3 pos = vf3_to_vd3(p);
  while (obj->parent) { pos += obj->p_offset; obj = obj->parent; }
  double3x3 Rt;
  md3_transpose2(Rt, obj->R);
  double3 f_rot = md3_v_mul(Rt, vf3_to_vd3(f));
  double3 t_rot = vd3_cross(md3_v_mul(Rt, pos - obj->m.cog), f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}

void
pl_object_set_gravity3f(pl_object_t *obj, float x, float y, float z)
{
  while (obj->parent) obj = obj->parent;
  obj->g_ack += vd3_set(x, y, z);
}


void
pl_object_set_gravity3fv(pl_object_t *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;
  obj->g_ack += vf3_to_vd3(f);
}


void
pl_object_dump(pl_object_t *obj)
{
  fprintf(stderr, "obj: %p\n", (void*)obj);
  fprintf(stderr, "\tmass: %f [%f, %f, %f]\n", obj->m.m, obj->m.In[0][0], obj->m.In[1][1], obj->m.In[2][2]);
  fprintf(stderr, "\tcog: [%f, %f, %f]\n", obj->m.cog.x, obj->m.cog.y, obj->m.cog.z);
  fprintf(stderr, "\tv: %f %f %f\n", obj->v.x, obj->v.y, obj->v.z);
  fprintf(stderr, "\tf_acc: %f %f %f\n", obj->f_ack.x, obj->f_ack.y, obj->f_ack.z);
  fprintf(stderr, "\tt_acc: %f %f %f\n", obj->t_ack.x, obj->t_ack.y, obj->t_ack.z);
  fprintf(stderr, "\tp: "); lwc_dump(&obj->p);
  fprintf(stderr, "\tp_offset: [%f %f %f]\n",
          obj->p_offset.x, obj->p_offset.y, obj->p_offset.z);

}

void
pl_object_step(pl_object_t *obj, float dt)
{
  // TOOD: Fix air pressure computations
#if 0
  obj->airPressure = pl_object_compute_airpressure(obj);
  obj->airDensity = pl_object_compute_airdensity(obj);
#endif
  PL_CHECK_OBJ(obj);
  double3 fm = ((obj->f_ack + obj->g_ack)/ obj->m.m);

  obj->v += fm * dt; // Update velocity from force
  double3 dv = vd3_s_mul(obj->v, dt);
  lwc_translate3dv(&obj->p, dv); // Update position from velocity

  obj->angVel += md3_v_mul(obj->I_inv_world, obj->t_ack) * dt; // Update angular velocity with torque
  obj->q = qd_normalise(qd_vd3_rot(obj->q, obj->angVel, dt));    // Update quaternion with rotational velocity

  pl_object_compute_derived(obj); // Compute derived data (world inverted inertia tensor etc)

  pl_object_clear(obj); // Clear accums

  for (int i = 0 ; i < obj->children.length ; ++ i) {
    pl_object_step_child(obj->children.elems[i], dt);
  }
}

void
pl_object_step_child(pl_object_t *obj, float dt)
{
  PL_CHECK_OBJ(obj);

  obj->v = obj->parent->v; // Update velocity from force
  obj->p = obj->parent->p;

  double3 p_offset_rot = md3_v_mul(obj->parent->R, obj->p_offset);
  lwc_translate3dv(&obj->p, p_offset_rot); // Update position from parent
  obj->angVel = obj->parent->angVel;
  obj->q = obj->parent->q;

  pl_object_compute_derived(obj); // Compute derived data (world inverted inertia tensor etc)
  pl_object_clear(obj); // Clear accums
}

void
pl_object_normalise(pl_object_t *obj)
{
  PL_CHECK_OBJ(obj);
  // Since we are using non safe casts here, we must ensure this in case someone upgrades
  // to 64 bit positions
  assert(sizeof(obj->p.offs) == sizeof(float3));

  obj->q = qd_normalise(obj->q);
  qd_md3_convert(obj->R, obj->q);

  lwc_normalise(&obj->p);
}

void
pl_object_clear(pl_object_t *obj)
{
  obj->g = obj->g_ack;
  obj->t = obj->t_ack;
  obj->f = obj->f_ack;

  obj->f_ack = vd3_set(0.0f, 0.0f, 0.0f);
  obj->t_ack = vd3_set(0.0f, 0.0f, 0.0f);
  obj->g_ack = vd3_set(0.0f, 0.0f, 0.0f);
}

void
pl_object_set_angular_vel3f(pl_object_t *obj, float rx, float ry, float rz)
{
  obj->angVel = vd3_set(rx, ry, rz);
}

void
pl_object_set_vel3f(pl_object_t *obj, float dx, float dy, float dz)
{
  obj->v = vd3_set(dx, dy, dz);
}
void
pl_object_set_vel3fv(pl_object_t *obj, float3 dp)
{
  obj->v = vf3_to_vd3(dp);
}

void
pl_object_set_vel3dv(pl_object_t *obj, double3 dp)
{
  obj->v = dp;
}


void
pl_object_set_drag_coef(pl_object_t *obj, float coef)
{
  // TODO: What about parent objects?
  obj->dragCoef = coef;
}

void
pl_object_set_area(pl_object_t *obj, float area)
{
  // TODO: What about parent objects?
  obj->area = area;
}


// Sets angular velocity by a given quaternion and rotational velocity
// The quaternion should represent rotation around dec and ra of the main rot
// vector and r should represent the rads per second that the rotation will be
void
plSetAngularVel4fq(pl_object_t *obj, quatd_t q, float r)
{
  double3 v = vd3_set(1, 0, 0);
  quatd_t qrx = qd_rot(1, 0, 0, r);
  quatd_t qr = qd_mul(q, qrx);

  double3x3 m;
  qd_md3_convert(m, qr);

  double3 ra = md3_v_mul(m, v);

  obj->angVel = ra;
}


void
pl_object_set_angular_vel3fv(pl_object_t *obj, float3 r)
{
  obj->angVel = vf3_to_vd3(r);
}


double3
pl_object_get_vel(pl_object_t *obj)
{
  return obj->v;
}

double3
pl_object_get_angular_vel(pl_object_t *obj)
{
  return obj->angVel;
}

quatd_t
pl_object_get_quat(pl_object_t *obj)
{
  return obj->q;
}

#if 0
void
pl_system_add_object(pl_system_t *sys, pl_object_t *obj)
{
  if (obj->sys != NULL) {
    // Not the most efficient way if there are several objects in the system,
    // but should be fine for now
    for (int i = 0 ; i < obj->sys->rigidObjs.length ; ++i) {
      if (obj->sys->rigidObjs.elems[i] == obj) {
        obj_array_remove(&obj->sys->rigidObjs, i);
        break;
      }
    }
  }
  obj->sys = sys;
  obj_array_push(&sys->rigidObjs, obj);
}
#endif

void
pl_object_check(pl_object_t *obj, const char *file, int line)
{
  if (!isfinite(obj->p.offs.x) || !isfinite(obj->p.offs.y) || !isfinite(obj->p.offs.z)) {
    log_abort("%s:%d obj position not finite", file, line);
  }
  if (!isfinite(obj->f_ack.x) || !isfinite(obj->f_ack.y) || !isfinite(obj->f_ack.z)) {
    log_abort("%s:%d obj force ack not finite", file, line);
  }
  if (!isfinite(obj->t_ack.x) || !isfinite(obj->t_ack.y) || !isfinite(obj->t_ack.z)) {
    log_abort("%s:%d obj torque ack not finite", file, line);
  }
  if (!isfinite(obj->g_ack.x) || !isfinite(obj->g_ack.y) || !isfinite(obj->g_ack.z)) {
    log_abort("%s:%d obj gravity ack not finite", file, line);
  }

  if (!isfinite(obj->v.x) || !isfinite(obj->v.y) || !isfinite(obj->v.z)) {
    log_abort("%s:%d obj velocity not finite", file, line);
  }
}

lwcoord_t
pl_object_get_lwc(pl_object_t *obj)
{
  return obj->p;
}
