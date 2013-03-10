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
#include <stdlib.h>

#include "flightrecorder.h"
#include "sim/spacecraft.h"
#include <gencds/array.h>
#include "palloc.h"

OOflightrecorder*
ooNewFlightRecorder(sim_spacecraft_t *sc, int maxSamples, int sampleInterval)
{
  OOflightrecorder *fr = smalloc(sizeof(OOflightrecorder));
  double_array_init(&fr->data);
  fr->sampleInterval = sampleInterval;
  fr->sampleTimeStamp = 0;
  fr->maxSamples = maxSamples;
  return fr;
}

void
ooFlightRecorderStep(OOflightrecorder *fr, float dt)
{
  double currentSimTime = sim_time_get_jd();

  // Time
  double_array_push(&fr->data, currentSimTime);

  // Position
  double_array_push(&fr->data, 0.0);
  double_array_push(&fr->data, 0.0);
  double_array_push(&fr->data, 0.0);

  // Quaternion
  double_array_push(&fr->data, 0.0);
  double_array_push(&fr->data, 0.0);
  double_array_push(&fr->data, 0.0);
  double_array_push(&fr->data, 0.0);
}

void
ooFlightRecorderWrite(OOflightrecorder *fr, FILE *file)
{
  fprintf(file, "JD, X, Y, Z, QX, QY, QZ, QW\n");

  for (size_t i = 0 ; i < fr->data.length / 8; ++ i) {
    fprintf(file, "%f, %f, %f, %f, %f, %f, %f, %f\n",
            fr->data.elems[i * 8 + 0],

            fr->data.elems[i * 8 + 1],
            fr->data.elems[i * 8 + 2],
            fr->data.elems[i * 8 + 3],

            fr->data.elems[i * 8 + 4],
            fr->data.elems[i * 8 + 5],
            fr->data.elems[i * 8 + 6],
            fr->data.elems[i * 8 + 7]);
  }
}
