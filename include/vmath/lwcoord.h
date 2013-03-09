/*
 Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit. Open Orbit is free software: you can
 redistribute it and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

 Some files of Open Orbit have relaxed licensing conditions. This file is
 licenced under the 2-clause BSD licence.

 Redistribution and use of this file in source and binary forms, with or
 without modification, are permitted provided that the following conditions are
 met:

 - Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
  Large World Coordinate Support

  Large world coordinates are based on a segmented universe where each segment is a cube
  with 1024.0 * 2 units length.

  Large world coordinates allow us to represent really huge worlds with very good
  precision locally. Although a fixed point solution would probably look better from
  certain astetic points, the floating point + segment id is used in order to maintain
  a reasonably straight forward compatibility with the rendering pipeline.
*/
#ifndef LWCOORD_H_94L1BHQN
#define LWCOORD_H_94L1BHQN

#include <vmath/vmath.h>

#define OO_LW_SEGMENT_LEN 1024.0f
#define OO_LW_SEGMENT_LEN64 1024.0

struct lwcoord_t {
  float3 offs;
  int3 seg;
};

typedef struct lwcoord_t lwcoord_t;

void lwc_set(lwcoord_t *coord, float x, float y, float z);
void lwc_normalise(lwcoord_t *coord);
void lwc_translate3fv(lwcoord_t *coord, float3 offs);
void lwc_translate3f(lwcoord_t *coord, float dx, float dy, float dz);
float3 lwc_global(const lwcoord_t *coord);
float3 lwc_relvec(const lwcoord_t *coord, int3 seg);
float3 lwc_dist(const lwcoord_t *a, const lwcoord_t * b);
void lwc_dump(const lwcoord_t *lwc);
int lwc_octant(const lwcoord_t *a, const lwcoord_t * b);

struct lwcoord64_t {
  double3 offs;
  long3 seg;
};

typedef struct lwcoord64_t lwcoord64_t;

void lwc_set64(lwcoord64_t *coord, double x, double y, double z);
void lwc_normalise64(lwcoord64_t *coord);
void lwc_translate64(lwcoord64_t *coord, double3 offs);
double3 lwc_global64(const lwcoord64_t *coord);
double3 lwc_relvec64(const lwcoord64_t *coord, long3 seg);
double3 lwc_dist64(const lwcoord64_t *a, const lwcoord64_t * b);



#endif /* end of include guard: LWCOORD_H_94L1BHQN */
