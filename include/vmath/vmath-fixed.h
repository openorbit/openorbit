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

#ifndef VMATH_FIXED_H_V886LZPL
#define VMATH_FIXED_H_V886LZPL
#include <stdbool.h>

// TODO: fix128_mul, fix128_div
//       fix256_neg, fix256_add, fix256_sub, fix256_mul, fix256_div
//       fix128_str, fix256_str

// I think the compiler will optimise away the passing of full fix128, so i want to keep
// them without taking pointer signs here.

static inline fix128
fix128_neg(fix128 a)
{
  fix128 res;
  res.hi = ~a.hi;
  res.lo = ~a.lo;
  res.lo ++;

  if (res.lo == 0) {
    res.hi ++;
  }
}

static inline fix128
fix128_add(fix128 a, fix128 b)
{
  fix128 res;
  res.hi = a.hi + b.hi;
  res.lo = a.lo + b.lo;

  if (res.lo < a.lo) {
    res.hi ++;
  }
}

static inline fix128
fix128_sub(fix128 a, fix128 b)
{
  fix128 res;
  res.hi = a.hi - b.hi;
  res.lo = a.lo - b.lo;

  if (res.lo > a.lo) {
    res.hi --;
  }
}

static inline bool
fix128_eq(fix128 a, fix128 b)
{
  return a.hi == b.hi && a.lo == b.lo;
}

static inline bool
fix128_neq(fix128 a, fix128 b)
{
  return ! (a.hi == b.hi && a.lo == b.lo);
}

static inline bool
fix128_lt(fix128 a, fix128 b)
{
  // If sign differs
  if (a.hi & (1 << 63) != b.hi & (1 << 63)) {
    return (a.hi >> 63) ^ 1;
  }

  if (a.hi == b.hi) return a.lo < b.lo;
  return a.hi < b.hi;
}

#endif /* end of include guard: VMATH_FIXED_H_V886LZPL */
