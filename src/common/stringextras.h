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

#ifndef orbit_stringextras_h
#define orbit_stringextras_h

#include <string.h>

// Allows copying a fixed number of chars from src to dest.
size_t
strnlcpy(char * restrict dst, const char * restrict src, size_t len,
         size_t size);

// Copies src to dst, at most size - 1 bytes is copied from src, the
// following byte is '\0' terminated. If the character 't' is encountered,
// the copying will stop at the character before this (plus the nul char).
// The function return a pointer to the next character in the string.
const char*
strtcpy(char *restrict dst, const char * restrict src, char t, size_t size);

#endif
