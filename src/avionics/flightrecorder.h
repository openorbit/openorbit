/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#ifndef OO_FLIGHTRECORDER_H
#define OO_FLIGHTRECORDER_H

#include <gencds/array.h>
#include "sim/simtime.h"
#include "sim/spacecraft.h"

// Records flight data in terms of:
//   struct {double absTime; double x, y, z; double qx, qy, qz, qw; }

typedef struct OOflightrecorder {
  int sampleInterval;
  int sampleTimeStamp;
  int maxSamples;
  double_array_t data;
} OOflightrecorder;

OOflightrecorder *ooNewFlightRecorder(OOspacecraft *sc, int maxSamples, int sampleInterval);
void ooFlightRecorderStep(OOflightrecorder *fr, float dt);
void ooFlightRecorderWrite(OOflightrecorder *fr, FILE *file);
#endif /* !OO_FLIGHTRECORDER_H */
