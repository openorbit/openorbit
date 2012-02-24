/*
  Copyright 2006,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef __SETTINGS_H__
#define __SETTINGS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "parsers/hrml.h"
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

void ooConfLoad(const char *name);

int ooConfGetBoolDef(const char *key, bool *val, bool defVal);
int ooConfGetBoolAsIntDef(const char *key, int *val, int defVal);
int ooConfGetIntDef(const char *key, int *val, int defVal);
int ooConfGetFloatDef(const char *key, float *val, float defVal);
int ooConfGetStrDef(const char *key, const char **val, const char *defVal);

void ooConfRegisterBoolDef(const char *key, bool val);
void ooConfRegisterFloatDef(const char *key, float val);
void ooConfRegisterIntegerDef(const char *key, long val);
void ooConfRegisterStringDef(const char *key, const char *val);


typedef struct OOconfnode OOconfnode;
typedef struct OOconfarr OOconfarr;

OOconfnode* ooConfGetNode(const char *key);
bool ooConfNodeNext(OOconfnode *node);
void ooConfNodeDispose(OOconfnode *node);
bool ooConfGetBool(OOconfnode *node);
OOconfnode* ooConfGetObj(OOconfnode *node);
long ooConfGetInt(OOconfnode *node);
float ooConfGetFloat(OOconfnode *node);
OOconfarr* ooConfGetArr(OOconfnode *node);
const char* ooConfGetStr(OOconfnode *node);

OOconfarr* ooConfGetNamedArr(const char *key);

OOconfarr* ooConfGetArrByName(OOconfnode *node, const char *name);
int ooConfGetArrLen(OOconfarr *arr);
OOconfnode* ooConfGetArrObj(OOconfarr *arr, int i);
long ooConfGetArrInt(OOconfarr *arr, int i);
float ooConfGetArrFloat(OOconfarr *arr, int i);
bool ooConfGetArrBool(OOconfarr *arr, int i);
const char* ooConfGetArrStr(OOconfarr *arr, int i);


bool ooConfGetBoolByName(OOconfnode *node, const char *name);
OOconfnode* ooConfGetObjByName(OOconfnode *node, const char *name);
long ooConfGetIntByName(OOconfnode *node, const char *name);
float ooConfGetFloatByName(OOconfnode *node, const char *name);
const char* ooConfGetStrByName(OOconfnode *node, const char *name);


const char* ooConfName(OOconfnode *node);
bool ooConfIsFloat(OOconfnode *node);
bool ooConfIsInt(OOconfnode *node);
bool ooConfIsObj(OOconfnode *node);
bool ooConfIsBool(OOconfnode *node);
bool ooConfIsArr(OOconfnode *node);
bool ooConfIsStr(OOconfnode *node);


#ifdef __cplusplus
}
#endif

#endif /* ! __SETTINGS_H__ */
