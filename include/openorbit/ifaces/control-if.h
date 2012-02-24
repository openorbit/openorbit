/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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

 
 
#ifndef CONTROL_IF_H__
#define CONTROL_IF_H__

#include <stdbool.h>

typedef struct {
    void (*aft_thrusters)(OOobject *self, bool on);
    void (*fwd_thrusters)(OOobject *self, bool on);
    void (*left_thrusters)(OOobject *self, bool on);
    void (*right_thrusters)(OOobject *self, bool on);
    void (*top_thrusters)(OOobject *self, bool on);
    void (*bottom_thrusters)(OOobject *self, bool on);
} thr_iface;

#define THR_IF_KEY "pub.control.thr"


typedef struct {
    void (*alpha_rcs)(OOobject *self, bool on);
    void (*beta_rcs)(OOobject *self, bool on);
    void (*gamma_rcs)(OOobject *self, bool on);
} rcs_iface;

#define RCS_IF_KEY "pub.control.rcs"


typedef struct {
    void (*alpha_mt)(OOobject *self, bool on);
    void (*beta_mt)(OOobject *self, bool on);
    void (*gamma_mt)(OOobject *self, bool on);
} mt_iface;

#define MT_IF_KEY "pub.control.mt"


#endif /* ! CONTROL_IF_H__ */