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
  obj->parent = NULL;  
  
  obj->f_ack = vf3_set(0.0, 0.0, 0.0);
  obj->t_ack = vf3_set(0.0, 0.0, 0.0);

  obj->v = vf3_set(0.0, 0.0, 0.0);
  plSetAngularVel3f(obj, 0.0f, 0.0f, 0.0f);

  obj->q = q_rot(1.0, 0.0, 0.0, 0.0); // Rotation quaternion  
  
  plComputeDerived(obj);
}

PLobject*
plObject3f(PLsystem *sys, float x, float y, float z)
{
  assert(sys != NULL);

  PLobject *obj = malloc(sizeof(PLobject));
  //  obj->id = dBodyCreate(sys->world->world);
  //dBodySetGravityMode(obj->id, 0); // Ignore standard ode gravity effects
  //dBodySetData(obj->id, NULL);
  //dBodySetMovedCallback(obj->id, plUpdateObject);

  //dQuaternion quat = {vf4_w(q), vf4_x(q), vf4_y(q), vf4_z(q)};
  // TODO: Ensure quaternion is set for orbit
  //       asc * inc * obl ?

  //dBodySetQuaternion(obj->id, quat);
  //dBodySetAngularVel(obj->id, 0.0, 0.0, 0.05);
  plInitObject(obj);

  plSetAngularVel3f(obj, 0.0f, 0.0f, 0.05f);
  plMassSet(&obj->m, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f);
  
  ooLwcSet(&obj->p, x, y, z);
  obj->sys = sys;
  obj->parent = NULL;
  obj_array_push(&sys->objs, obj);

  plComputeDerived(obj);

  return obj;
}

PLcompound_object*
plCompoundObject3f(PLsystem *sys, float x, float y, float z)
{
  assert(sys != NULL);
  
  PLcompound_object *obj = malloc(sizeof(PLcompound_object));
  //obj->super.id = dBodyCreate(sys->world->world);
  //dBodySetGravityMode(obj->super.id, 0); // Ignore standard ode gravity effects
  //dBodySetData(obj->super.id, NULL);
  //dBodySetMovedCallback(obj->id, plUpdateObject);
  
  //dQuaternion quat = {vf4_w(q), vf4_x(q), vf4_y(q), vf4_z(q)};
  // TODO: Ensure quaternion is set for orbit
  //       asc * inc * obl ?
  
  //dBodySetQuaternion(obj->id, quat);
  //dBodySetAngularVel(obj->super.id, 0.0, 0.0, 0.05);
  plInitObject(&obj->super);

  plSetAngularVel3f(&obj->super, 0.0f, 0.0f, 0.05f);

  plMassSet(&obj->super.m, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f);
  
  ooLwcSet(&obj->super.p, x, y, z);
  obj->super.sys = sys;
  obj->super.parent = NULL;
  obj_array_push(&sys->objs, obj);
  obj_array_init(&obj->children);
  plComputeDerived(&obj->super);

  return obj;
  
}
PLobject*
plSubObject3f(PLcompound_object *parent, float x, float y, float z)
{
  assert(parent != NULL);
  
  PLobject *obj = malloc(sizeof(PLobject));
  plMassSet(&obj->m, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f,
            0.0f, 0.0f, 0.0f);
  ooLwcSet(&obj->p, x, y, z);
  obj->sys = parent->super.sys;
  obj->parent = parent;
  obj_array_push(&parent->children, obj);
  return obj;
}

void
plDetatchObject(PLobject *obj)
{
  assert(obj != NULL);
  assert(obj->parent != NULL);

  obj_array_push(&obj->parent->super.sys->objs, obj);
  obj->parent = NULL;
}

void
plUpdateMass(PLcompound_object *obj)
{
  memset(&obj->super.m, 0, sizeof(PLmass));
  
  plMassSet(&obj->super.m, 0.0,
            0.0, 0.0, 0.0,
            1.0, 1.0, 1.0,
            0.0, 0.0, 0.0);
  
  for (int i = 0 ; i < obj->children.length ; ++ i) {
    PLobject *child = obj->children.elems[i];
    PLmass tmp = child->m;
    plMassTranslate(&tmp,
                    -child->m.cog[0],
                    -child->m.cog[1],
                    -child->m.cog[2]);
      
    plMassAdd(&obj->super.m, &tmp);
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


void plStepObjectf(PLobject *obj, float dt)
{
  obj->v += (obj->f_ack / obj->m.m) * dt; // Update velocity from force
  ooLwcTranslate(&obj->p, vf3_s_mul(obj->v, dt)); // Update position from velocity

  obj->angVel += mf3_v_mul(obj->I_inv_world, obj->t_ack) * dt; // Update angular velocity with torque
  obj->q = q_normalise(q_vf3_rot(obj->q, obj->angVel, dt)); // Update quaternion with rotational velocity

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
plSetAngularVel3fv(PLobject *obj, float3 r)
{
  obj->angVel = r;
}

quaternion_t plGetQuat(PLobject *obj)
{
  return obj->q;
}