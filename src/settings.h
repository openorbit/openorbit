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
typedef struct config_t config_t;

void config_load(const char *name);

int config_get_bool_def(const char *key, bool *val, bool defVal);
int config_get_bool_as_int_def(const char *key, int *val, int defVal);
int config_get_int_def(const char *key, int *val, int defVal);
int config_get_float_def(const char *key, float *val, float defVal);
int config_get_str_def(const char *key, const char **val, const char *defVal);

void config_register_bool_def(const char *key, bool val);
void config_register_float_def(const char *key, float val);
void config_register_integer_def(const char *key, long val);
void config_register_string_def(const char *key, const char *val);


typedef struct config_node_t config_node_t;
typedef struct config_arr_t config_arr_t;

config_node_t* config_get_node(const char *key);
bool config_node_next(config_node_t *node);
void config_node_dispose(config_node_t *node);
bool config_get_bool(config_node_t *node);
config_node_t* config_get_obj(config_node_t *node);
long config_get_int(config_node_t *node);
float config_get_float(config_node_t *node);
config_arr_t* config_get_arr(config_node_t *node);
const char* config_get_str(config_node_t *node);

config_arr_t* config_get_named_arr(const char *key);

config_arr_t* config_get_arr_by_name(config_node_t *node, const char *name);
int config_get_arr_len(config_arr_t *arr);
config_node_t* config_get_arr_obj(config_arr_t *arr, int i);
long config_get_arr_int(config_arr_t *arr, int i);
float config_get_arr_float(config_arr_t *arr, int i);
bool config_get_arr_bool(config_arr_t *arr, int i);
const char* config_get_arr_str(config_arr_t *arr, int i);


bool config_get_bool_by_name(config_node_t *node, const char *name);
config_node_t* config_get_obj_by_name(config_node_t *node, const char *name);
long config_get_int_by_name(config_node_t *node, const char *name);
float config_get_float_by_name(config_node_t *node, const char *name);
const char* config_get_str_by_name(config_node_t *node, const char *name);


const char* config_name(config_node_t *node);
bool config_is_float(config_node_t *node);
bool config_is_int(config_node_t *node);
bool config_is_obj(config_node_t *node);
bool config_is_bool(config_node_t *node);
bool config_is_arr(config_node_t *node);
bool config_is_str(config_node_t *node);


#ifdef __cplusplus
}
#endif

#endif /* ! __SETTINGS_H__ */
