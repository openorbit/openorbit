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
