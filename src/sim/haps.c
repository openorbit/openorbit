/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include "haps.h"
#include <stddef.h>

#define MAX_HAP_HANDLERS 4

static SIMhaphandler haps[HAP_COUNT][MAX_HAP_HANDLERS];

void
simRegisterHapHandler(enum SIMhapID hap, SIMhaphandler handler)
{
  for (int i = 0 ; i < MAX_HAP_HANDLERS ; i ++) {
    if (haps[hap][i] == NULL) {
      haps[hap][i] = handler;
      break;
    }
  }
}

void
simHapOccured(enum SIMhapID hap, void *arg0, void *arg1)
{
  for (int i = 0 ; i < MAX_HAP_HANDLERS ; i ++) {
    if (haps[hap][i]) haps[hap][i](arg0, arg1);
  }
}
