/*
 Copyright 2011 Mattias Holm <mattias.holm(at)openorbit.org>
 
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


/*!
 The haps interface define a simple interface for intercepting events or "haps"
 in openorbit. There is a hardcoded set of events and a limited number of hap
 handlers that can be registered per event.
 */

#ifndef SIM_HAPS_H
#define SIM_HAPS_H


typedef void (*SIMhaphandler)(void *, void *);

enum SIMhapID {
  HAP_NONE = 0,
  HAP_POWER_OVERLOAD,
  HAP_COUNT,
};


/*!
  Register a hap handler
 */
void simRegisterHapHandler(enum SIMhapID hap, SIMhaphandler handler);

/*!
  Notify that a hap occured. This will invoke all the hap handlers registered
  to that hap.
 */
void simHapOccured(enum SIMhapID hap, void *arg0, void *arg1);

#endif /* ! SIM_HAPS_H */
