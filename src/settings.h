/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/*! The settings structure stores global information on the game such as video
 *  and audio configuration parameters. This should be the central store for
 *  most information abour the current game instance's more technical data,
 *  with the notable exception of the IO configuraton that is handled by a
 *  special IO subsystem.
 * 
 *  \bug The information is stored in a global hashtable, and these
 *      functions are not yet threadsafe.
 * 
 *  NOTE: In the Python API, this system is called config.
 */
typedef struct OOconf OOconf;

void ooConfInit(void);
void ooConfLoad(const char *name);

int ooConfGetBoolDef(const char *key, bool *val, bool defVal);
int ooConfGetBoolAsIntDef(const char *key, int *val, int defVal);
int ooConfGetIntDef(const char *key, int *val, int defVal);
int ooConfGetFloatDef(const char *key, float *val, float defVal);
int ooConfGetStrDef(const char *key, const char **val, const char *defVal);

#ifdef __cplusplus
}
#endif

#endif /* ! __SETTINGS_H__ */
