/*
 Copyright 2013 Mattias Holm <lorrden(at)openorbit.org>

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

#include <stdio.h>

#ifdef __APPLE__
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

uint64_t
getmonotimestamp(void)
{
#ifdef __APPLE__
  return mach_absolute_time();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif
}


uint64_t
subtractmonotime(uint64_t a, uint64_t b)
{
  uint64_t diff = a - b;

#ifdef __APPLE__
  static mach_timebase_info_data_t timebase;
  if (timebase.denom == 0) {
    mach_timebase_info(&timebase);
  }

  return diff * timebase.numer / timebase.denom;
#else
  return diff;
#endif
}

// Return monotonic time for ns
uint64_t
nstomonotime(uint64_t ns)
{
#ifdef __APPLE__
  static mach_timebase_info_data_t timebase;
  if (timebase.denom == 0) {
    mach_timebase_info(&timebase);
  }

  return ns * timebase.denom / timebase.numer;
#else
  return ns;
#endif
}

// Return monotonic time in ns
uint64_t
monotimetons(uint64_t mono)
{
#ifdef __APPLE__
  static mach_timebase_info_data_t timebase;
  if (timebase.denom == 0) {
    mach_timebase_info(&timebase);
  }

  return mono  * timebase.numer / timebase.denom;
#else
  return mono;
#endif
}



