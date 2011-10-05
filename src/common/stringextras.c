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

#include <string.h>
#include "common/stringextras.h"

size_t
strnlcpy(char * restrict dst, const char * restrict src, size_t len,
         size_t size)
{
  if (len + 1 < size) {
    return strlcpy(dst, src, len+1);
  } else {
    return strlcpy(dst, src, size);
  }
}

const char*
strtcpy(char *restrict dst, const char * restrict src, char t, size_t size)
{
  size_t i;
  for (i = 0 ; i < size-1 ; i ++) {
    if (src[i] == '\0' || src[i] == t) break;
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return &src[i];
}
