/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "physics.h"
#include "object.h"
#include "common/lwcoord.h"
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
plComputeDerived(PLobject *obj)
{
  q_mf3_convert(obj->R, obj->q); // Convert our quaternion to rotation matrix
  mf3_basis(obj->I_inv_world, obj->m.I_inv, obj->R);
}
void
plInitObject(PLobject *obj)
{
  plMassSet(&obj->m, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f);

  ooLwcSet(&obj->p, 0.0, 0.0, 0.0);
  obj->sys = NULL;
  obj->name = NULL;
  obj->parent = NULL;
  obj->drawable = NULL;
  obj->f_ack = vf3_set(0.0, 0.0, 0.0);
  obj->t_ack = vf3_set(0.0, 0.0, 0.0);

  obj->v = vf3_set(0.0, 0.0, 0.0);
  plSetAngularVel3f(obj, 0.0f, 0.0f, 0.0f);

  obj->q = q_rot(1.0, 0.0, 0.0, 0.0); // Rotation quaternion
  obj->p_offset = vf3_set(0.0, 0.0, 0.0);

  obj->dragCoef = 0.0;
  obj->area = 0.0;
  obj_array_init(&obj->children);
  obj_array_init(&obj->psystem);

  plComputeDerived(obj);
}

PLobject*
plObject(PLworld *world)
{
  assert(world != NULL);

  PLobject *obj = malloc(sizeof(PLobject));
  plInitObject(obj);

  obj_array_push(&world->objs, obj);

  plComputeDerived(obj);

  return obj;
}

PLobject*
plSubObject3f(PLworld *world, PLobject *parent, float x, float y, float z)
{
  assert(parent != NULL);

  PLobject *obj = malloc(sizeof(PLobject));
  plInitObject(obj);

  obj->sys = parent->sys;
  obj->parent = parent;
  obj->p_offset = vf3_set(x, y, z);
  obj_array_push(&world->objs, obj);
  obj_array_push(&parent->children, obj);
  return obj;
}

void
plDetatchObject(PLobject *obj)
{
  // Should a detatched object be removed from the compound object completetlly
  // or should it be kept around for future references? At the moment, the
  // parent of the detatched object is set to NULL in order to indcate the
  // detachement but the object pointer is still left in the parents child array
  assert(obj != NULL);
  assert(obj->parent != NULL);

  obj_array_push(&obj->parent->sys->rigidObjs, obj);
  PLobject *parent = obj->parent;
  obj->parent = NULL;

  plUpdateMass(parent);
}

void
plUpdateMass(PLobject *obj)
{
  memset(&obj->m, 0, sizeof(PLmass));

  plMassSet(&obj->m, 0.0,
            0.0, 0.0, 0.0,
            1.0, 1.0, 1.0,
            0.0, 0.0, 0.0);

  for (int i = 0 ; i < obj->children.length ; ++ i) {
    PLobject *child = obj->children.elems[i];
    if (child->parent) { // Only for attached objects
      PLmass tmp = child->m;
      plMassTranslate(&tmp,
                      -child->m.cog[0],
                      -child->m.cog[1],
                      -child->m.cog[2]);
      plMassAdd(&obj->m, &tmp);
    }
  }
}


void
plSetDrawableForObject(PLobject *obj, SGdrawable *drawable)
{
  assert(obj != NULL);

  obj->drawable = drawable;
}

void
plSetObjectPos3d(PLobject *obj, double x, double y, double z)
{
  ooLwcSet(&obj->p, x, y, z);
}


void
plSetObjectPosExt3f(PLobject *obj,
                    int32_t i, int32_t j, int32_t k,
                    float x, float y, float z)
{
  obj->p.seg = vi3_set(i, j, k);
  obj->p.offs = vf3_set(x, y, z);
  ooLwcNormalise(&obj->p);
}

void
plSetObjectPosRel3d(PLobject * restrict obj, const PLobject * restrict otherObj,
                    double x, double y, double z)
{
  obj->p = otherObj->p;
  ooLwcTranslate3f(&obj->p, x, y, z);
}

void
plSetObjectPosRel3fv(PLobject * restrict obj,
                     const PLobject * restrict otherObj,
                     float3 rp)
{
  obj->p = otherObj->p;
  ooLwcTranslate3fv(&obj->p, rp);
  ooLwcDump(&obj->p);
}
void
plForce3f(PLobject *obj, float x, float y, float z)
{
  obj->f_ack.x += x;
  obj->f_ack.y += y;
  obj->f_ack.z += z;
}
void
plForce3fv(PLobject *obj, float3 f)
{
  obj->f_ack += f;
}


void
plForceRelative3f(PLobject *obj, float fx, float fy, float fz)
{
  float3 f = { fx, fy, fz, 0.0f };
  float3 f_rot = mf3_v_mul(obj->R, f);
  obj->f_ack += f_rot;
}

void
plForceRelative3fv(PLobject *obj, float3 f)
{
  float3 f_rot = mf3_v_mul(obj->R, f);
  obj->f_ack += f_rot;
}

void
plForceRelativePos3f(PLobject *obj,
                     float fx, float fy, float fz,
                     float px, float py, float pz)
{
  float3 f = { fx, fy, fz, 0.0f };
  float3 p = { px, py, pz, 0.0f };
  float3 f_rot = mf3_v_mul(obj->R, f);
  float3 t_rot = vf3_cross(p, f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}


void
plForceRelativePos3fv(PLobject *obj, float3 f, float3 p)
{
  float3 f_rot = mf3_v_mul(obj->R, f);
  float3 t_rot = vf3_cross(p, f_rot);
  obj->f_ack += f_rot;
  obj->t_ack += t_rot;
}

void
plDumpObject(PLobject *obj)
{
  fprintf(stderr, "obj: %p\n", (void*)obj);
  fprintf(stderr, "mass: %f [%f, %f, %f]\n", obj->m.m, obj->m.I[0][0], obj->m.I[1][1], obj->m.I[2][2]);
  fprintf(stderr, "v: %f %f %f\n", obj->v.x, obj->v.y, obj->v.z);
  fprintf(stderr, "f_acc: %f %f %f\n", obj->f_ack.x, obj->f_ack.y, obj->f_ack.z);
  fprintf(stderr, "t_acc: %f %f %f\n", obj->t_ack.x, obj->t_ack.y, obj->t_ack.z);
  fprintf(stderr, "p: "); ooLwcDump(&obj->p);
}

void
plStepObjectf(PLobject *obj, float dt)
{
  float3 fm = (obj->f_ack / obj->m.m);
  obj->v += fm * dt; // Update velocity from force

  ooLwcTranslate3fv(&obj->p, vf3_s_mul(obj->v, dt)); // Update position from velocity

  obj->angVel += mf3_v_mul(obj->I_inv_world, obj->t_ack) * dt; // Update angular velocity with torque
  obj->q = q_normalise(q_vf3_rot(obj->q, obj->angVel, dt)); // Update quaternion with rotational velocity

  plComputeDerived(obj); // Compute derived data (world inverted inertia tensor etc)

  plClearObject(obj); // Clear accums
}

void
plStepChildObjectf(PLobject *obj, float dt)
{
  obj->v = obj->parent->v; // Update velocity from force
  obj->p = obj->parent->p;

  float3 p_offset_rot = v_q_rot(obj->p_offset, obj->parent->q);
  ooLwcTranslate3fv(&obj->p, p_offset_rot); // Update position from parent
  obj->angVel = vf3_set(0.0f, 0.0f, 0.0f);
  obj->q = obj->parent->q;

  plComputeDerived(obj); // Compute derived data (world inverted inertia tensor etc)
  plClearObject(obj); // Clear accums
}

void
plNormaliseObject(PLobject *obj)
{
  // Since we are using non safe casts here, we must ensure this in case someone upgrades
  // to 64 bit positions
  assert(sizeof(obj->p.offs) == sizeof(float3));

  obj->q = q_normalise(obj->q);
  q_mf3_convert(obj->R, obj->q);

  ooLwcNormalise(&obj->p);
}

void
plClearObject(PLobject *obj)
{
  obj->f_ack = vf3_set(0.0f, 0.0f, 0.0f);
  obj->t_ack = vf3_set(0.0f, 0.0f, 0.0f);
}

void
plSetAngularVel3f(PLobject *obj, float rx, float ry, float rz)
{
  obj->angVel = vf3_set(rx, ry, rz);
}

void
plSetVel3f(PLobject *obj, float dx, float dy, float dz)
{
  obj->v = vf3_set(dx, dy, dz);
}
void
plSetVel3fv(PLobject *obj, float3 dp)
{
  obj->v = dp;
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
plSetAngularVel3fv(PLobject *obj, float3 r)
{
  obj->angVel = r;
}

quaternion_t
plGetQuat(PLobject *obj)
{
  return obj->q;
}

void
plSetSystem(PLsystem *sys, PLobject *obj)
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
