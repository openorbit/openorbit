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


#ifndef orbit_monotonic_time_h
#define orbit_monotonic_time_h

/*!
 * Returns system dependent monotonic time stamp.
 *
 * OS X is missing clock_gettime and friends from the POSIX RT API. This
 * function returns the value of mach_absolute_time() when running on darwin,
 * and clock_gettime(CLOCK_MONOTONIC, ) when running on Linux. Because of this
 * the returned value is in system dependent time which may be identical to
 * a nanosecond, but may also be some other value. The value can be converted
 * to ns using the monotimetons function.
 *
 * Note that since the function returns monotonic time, it is leapsecond safe.
 *
 * \result Monotonic time stamp in system dependent units.
 */
uint64_t getmonotimestamp(void);

/*!
 * Subtract monotonic time stamps and return ns difference.
 *
 * \param a newer time stamp in system monotonic time.
 * \param b older time stamp in system monotonic time.
 * \result Time difference between a and b in nanoseconds.
 */
uint64_t subtractmonotime(uint64_t a, uint64_t b);

/*!
 * Convert nanoseconds to monotonic time
 * \param ns Nanoseconds
 * \result ns converted to system dependent monotonic time.
 */
uint64_t nstomonotime(uint64_t ns);

/*!
 * Convert monotonic time to nanoseconds
 * \param mono Monotonic time
 * \result mono converted to nanoseconds.
 */

uint64_t monotimetons(uint64_t mono);

#endif
