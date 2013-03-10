
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
pl_object_compute_derived(PLobject *obj)
{
  q_mf3_convert(obj->R, obj->q); // Convert our quaternion to rotation matrix
  mf3_basis(obj->I_inv_world, obj->m.I_inv, obj->R);
}
void
pl_object_init(PLobject *obj)
{
  pl_mass_set(&obj->m, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f);

  lwc_set(&obj->p, 0.0, 0.0, 0.0);

  obj->sys = NULL;
  obj->name = NULL;
  obj->parent = NULL;
  //obj->drawable = NULL;
  obj->f_ack = vf3_set(0.0, 0.0, 0.0);
  obj->t_ack = vf3_set(0.0, 0.0, 0.0);
  obj->g_ack = vf3_set(0.0, 0.0, 0.0);

  obj->v = vf3_set(0.0, 0.0, 0.0);
  pl_object_set_angular_vel3f(obj, 0.0f, 0.0f, 0.0f);

  obj->q = q_rot(1.0, 0.0, 0.0, 0.0); // Rotation quaternion
  obj->p_offset = vf3_set(0.0, 0.0, 0.0);

  obj->airPressure = 0.0;
  obj->dragCoef = 0.0;
  obj->area = 0.0;
  obj->radius = 0.0;

  obj_array_init(&obj->children);
  obj_array_init(&obj->psystem);

  pl_object_compute_derived(obj);
}

PLobject*
pl_new_object(pl_world_t *world, const char *name)
{
  assert(world != NULL);

  PLobject *obj = calloc(1, sizeof(PLobject));
  pl_object_init(obj);
  obj->name = strdup(name);

  obj_array_push(&world->objs, obj);

  pl_object_compute_derived(obj);

  pl_collide_insert_object(world->collCtxt, obj);

  return obj;
}

PLobject*
pl_new_sub_object3f(pl_world_t *world, PLobject *parent, const char * name,
              float x, float y, float z)
{
  assert(parent != NULL);

  PLobject *obj = calloc(1, sizeof(PLobject));
  pl_object_init(obj);
  obj->name = strdup(name);

  obj->sys = parent->sys;
  obj->parent = parent;
  obj->p_offset = vf3_set(x, y, z);
  obj_array_push(&world->objs, obj);
  obj_array_push(&parent->children, obj);
  return obj;
}

void
pl_object_detatch(PLobject *obj)
{
  assert(obj != NULL);
  assert(obj->parent != NULL);
  pl_world_t *world = obj->sys->world;
  PLobject *parent = obj->parent;

  obj->parent = NULL;
  obj->sys = parent->sys;
  for (int i = 0 ; i < parent->children.length ; ++i) {
    if (parent->children.elems[i] == obj) {
      obj_array_remove(&parent->children, i);
      obj_array_push(&parent->sys->rigidObjs, obj);
      break;
    }
  }

  pl_object_update_mass(parent);
  pl_object_compute_derived(obj);
  pl_collide_insert_object(world->collCtxt, obj);
}

void
pl_object_update_mass(PLobject *obj)
{
  memset(&obj->m, 0, sizeof(pl_mass_t));

  pl_mass_set(&obj->m, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0,
            0.0, 0.0, 0.0);

  for (int i = 0 ; i < obj->children.length ; ++ i) {
    PLobject *child = obj->children.elems[i];
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
pl_object_set_pos3d(PLobject *obj, double x, double y, double z)
{
  PL_CHECK_OBJ(obj);

  lwc_set(&obj->p, x, y, z);

  PL_CHECK_OBJ(obj);
}


void
pl_object_set_pos_ext3f(PLobject *obj,
                    int32_t i, int32_t j, int32_t k,
                    float x, float y, float z)
{
  PL_CHECK_OBJ(obj);

  obj->p.seg = vi3_set(i, j, k);
  obj->p.offs = vf3_set(x, y, z);
  lwc_normalise(&obj->p);

  PL_CHECK_OBJ(obj);
}

void
pl_object_set_pos_rel3d(PLobject * restrict obj, const PLobject * restrict otherObj,
                    double x, double y, double z)
{
  PL_CHECK_OBJ(obj);

  obj->p = otherObj->p;
  lwc_translate3f(&obj->p, x, y, z);

  PL_CHECK_OBJ(obj);
}

void
pl_object_set_pos_rel3fv(PLobject * restrict obj,
                     const PLobject * restrict otherObj,
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
pl_object_force3f(PLobject *obj, float x, float y, float z)
{
  while (obj->parent) obj = obj->parent;

  PL_CHECK_OBJ(obj);

  obj->f_ack.x += x;
  obj->f_ack.y += y;
  obj->f_ack.z += z;

  PL_CHECK_OBJ(obj);
}
void
pl_object_force3fv(PLobject *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;

  PL_CHECK_OBJ(obj);

  obj->f_ack += f;

  PL_CHECK_OBJ(obj);
}


void
pl_object_force_relative3f(PLobject *obj, float fx, float fy, float fz)
{
  while (obj->parent) obj = obj->parent;
  PL_CHECK_OBJ(obj);

  float3 f = vf3_set(fx, fy, fz);
  float3 f_rot = mf3_v_mul(obj->R, f);
  obj->f_ack += f_rot;
}

void
pl_force_relative3fv(PLobject *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;
  PL_CHECK_OBJ(obj);

  float3 f_rot = mf3_v_mul(obj->R, f);
  obj->f_ack += f_rot;
}

void
pl_object_force_relative_pos3f(PLobject *obj,
                     float fx, float fy, float fz,
                     float px, float py, float pz)
{
  PL_CHECK_OBJ(obj);

  float3 f = vf3_set(fx, fy, fz);
  float3 p = vf3_set(px, py, pz);

  while (obj->parent) { p += obj->p_offset; obj = obj->parent; }

  float3 f_rot = mf3_v_mul(obj->R, f);
  float3 t_rot = vf3_cross(mf3_v_mul(obj->R, p - obj->m.cog), f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}

void
pl_object_torque3f(PLobject *obj, float tx, float ty, float tz)
{
  obj->t_ack += vf3_set(tx, ty, tz);
}

void
pl_object_torque3fv(PLobject *obj, float3 t)
{
  obj->t_ack += t;
}

void
pl_object_torque_relative3f(PLobject *obj, float tx, float ty, float tz,
                   float px, float py, float pz)
{
  obj->t_ack += mf3_v_mul(obj->R, vf3_set(tx, ty, tz));
}

void
pl_object_torque_relative3fv(PLobject *obj, float3 t, float3 p)
{
  obj->t_ack += mf3_v_mul(obj->R, t);
}



void
pl_object_force_relative_pos3fv(PLobject *obj, float3 f, float3 p)
{
  while (obj->parent) { p += obj->p_offset; obj = obj->parent; }
  float3x3 Rt;
  mf3_transpose2(Rt, obj->R);
  float3 f_rot = mf3_v_mul(Rt, f);
  float3 t_rot = vf3_cross(mf3_v_mul(Rt, p - obj->m.cog), f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}

void
pl_object_set_gravity3f(PLobject *obj, float x, float y, float z)
{
  while (obj->parent) obj = obj->parent;
  obj->g_ack += vf3_set(x, y, z);
}


void
pl_object_set_gravity3fv(PLobject *obj, float3 f)
{
  while (obj->parent) obj = obj->parent;
  obj->g_ack += f;
}


void
pl_object_dump(PLobject *obj)
{
  fprintf(stderr, "obj: %p\n", (void*)obj);
  fprintf(stderr, "\tmass: %f [%f, %f, %f]\n", obj->m.m, obj->m.I[0][0], obj->m.I[1][1], obj->m.I[2][2]);
  fprintf(stderr, "\tcog: [%f, %f, %f]\n", obj->m.cog.x, obj->m.cog.y, obj->m.cog.z);
  fprintf(stderr, "\tv: %f %f %f\n", obj->v.x, obj->v.y, obj->v.z);
  fprintf(stderr, "\tf_acc: %f %f %f\n", obj->f_ack.x, obj->f_ack.y, obj->f_ack.z);
  fprintf(stderr, "\tt_acc: %f %f %f\n", obj->t_ack.x, obj->t_ack.y, obj->t_ack.z);
  fprintf(stderr, "\tp: "); lwc_dump(&obj->p);
  fprintf(stderr, "\tp_offset: [%f %f %f]\n",
          obj->p_offset.x, obj->p_offset.y, obj->p_offset.z);

}

void
pl_object_step(PLobject *obj, float dt)
{
  obj->airPressure = pl_object_compute_airpressure(obj);
  obj->airDensity = pl_object_compute_airdensity(obj);

  PL_CHECK_OBJ(obj);
  float3 fm = (obj->f_ack / obj->m.m);

  obj->v += fm * dt; // Update velocity from force
  float3 dv = vf3_s_mul(obj->v, dt);
  lwc_translate3fv(&obj->p, dv); // Update position from velocity

  obj->angVel += mf3_v_mul(obj->I_inv_world, obj->t_ack) * dt; // Update angular velocity with torque
  obj->q = q_normalise(q_vf3_rot(obj->q, obj->angVel, dt));    // Update quaternion with rotational velocity

  pl_object_compute_derived(obj); // Compute derived data (world inverted inertia tensor etc)

  pl_object_clear(obj); // Clear accums

  for (int i = 0 ; i < obj->children.length ; ++ i) {
    pl_object_step_child(obj->children.elems[i], dt);
  }
}

void
pl_object_step_child(PLobject *obj, float dt)
{
  PL_CHECK_OBJ(obj);

  obj->v = obj->parent->v; // Update velocity from force
  obj->p = obj->parent->p;

  float3 p_offset_rot = mf3_v_mul(obj->parent->R, obj->p_offset);
  lwc_translate3fv(&obj->p, p_offset_rot); // Update position from parent
  obj->angVel = obj->parent->angVel;
  obj->q = obj->parent->q;

  pl_object_compute_derived(obj); // Compute derived data (world inverted inertia tensor etc)
  pl_object_clear(obj); // Clear accums
}

void
pl_object_normalise(PLobject *obj)
{
  PL_CHECK_OBJ(obj);
  // Since we are using non safe casts here, we must ensure this in case someone upgrades
  // to 64 bit positions
  assert(sizeof(obj->p.offs) == sizeof(float3));

  obj->q = q_normalise(obj->q);
  q_mf3_convert(obj->R, obj->q);

  lwc_normalise(&obj->p);
}

void
pl_object_clear(PLobject *obj)
{
  obj->f_ack = vf3_set(0.0f, 0.0f, 0.0f);
  obj->t_ack = vf3_set(0.0f, 0.0f, 0.0f);
  obj->g_ack = vf3_set(0.0f, 0.0f, 0.0f);
}

void
pl_object_set_angular_vel3f(PLobject *obj, float rx, float ry, float rz)
{
  obj->angVel = vf3_set(rx, ry, rz);
}

void
pl_object_set_vel3f(PLobject *obj, float dx, float dy, float dz)
{
  obj->v = vf3_set(dx, dy, dz);
}
void
pl_object_set_vel3fv(PLobject *obj, float3 dp)
{
  obj->v = dp;
}

void
pl_object_set_drag_coef(PLobject *obj, float coef)
{
  // TODO: What about parent objects?
  obj->dragCoef = coef;
}

void
pl_object_set_area(PLobject *obj, float area)
{
  // TODO: What about parent objects?
  obj->area = area;
}


// Sets angular velocity by a given quaternion and rotational velocity
// The quaternion should represent rotation around dec and ra of the main rot
// vector and r should represent the rads per second that the rotation will be
void
plSetAngularVel4fq(PLobject *obj, quaternion_t q, float r)
{
  float3 v = vf3_set(1.0f, 0.0f, 0.0f);
  quaternion_t qrx = q_rot(1.0f, 0.0f, 0.0f, r);
  quaternion_t qr = q_mul(q, qrx);

  float3x3 m;
  q_mf3_convert(m, qr);

  float3 ra = mf3_v_mul(m, v);

  obj->angVel = ra;
}


void
pl_object_set_angular_vel3fv(PLobject *obj, float3 r)
{
  obj->angVel = r;
}


float3
pl_object_get_vel(PLobject *obj)
{
  return obj->v;
}

float3
pl_object_get_angular_vel(PLobject *obj)
{
  return obj->angVel;
}

quaternion_t
pl_object_get_quat(PLobject *obj)
{
  return obj->q;
}

void
pl_system_add_object(pl_system_t *sys, PLobject *obj)
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


void
pl_object_check(PLobject *obj, const char *file, int line)
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
pl_object_get_lwc(PLobject *obj)
{
  return obj->p;
}
