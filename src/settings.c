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


#include "settings.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <jansson.h>

#include <math.h>

#include "res-manager.h"
#include "parsers/hrml.h"
#include <openorbit/log.h>
#include "common/moduleinit.h"
#include "common/mapped-file.h"
#include "palloc.h"

struct OOconf
{
  HRMLdocument *doc;
};

static OOconf gConfSingleton = {.doc = NULL};

json_t *conf_doc;

void
ooConfLoad(const char *name)
{
  HRMLdocument *doc = hrmlParse(name);
  if (!doc) {
    ooLogError("could not load config file '%s'", name);
  }

  gConfSingleton.doc = doc;
}

#define MIN(a, b) ((a<b) ? a : b)
json_t*
lookup(const char *key)
{
  json_t *current_json = conf_doc;

  char current_key[128];

  const char *key_start = key;
  while (key_start && *key_start) {
    const char *key_end = strchr(key_start, '/');
    if (!key_end) key_end = strchr(key_start, '\0');

    strlcpy(current_key, key_start, MIN(key_end-key_start+1,
                                        sizeof(current_key)));
    //printf("access '%s'\n", current_key);
    if (!json_is_object(current_json)) {
      return NULL;
    }
    current_json = json_object_get(current_json, current_key);

    if (*key_end) {
      key_start = key_end + 1;
    } else {
      key_start = NULL;
    }
  }
  return current_json;
}

json_t*
getparent(const char *key)
{
  json_t *current_json = conf_doc;
  json_t *next_json = conf_doc;

  char current_key[128];

  const char *key_start = key;
  while (key_start && *key_start) {
    current_json = next_json;
    const char *key_end = strchr(key_start, '/');
    if (!key_end) key_end = strchr(key_start, '\0');

    strlcpy(current_key, key_start, MIN(key_end-key_start+1,
                                        sizeof(current_key)));
    //printf("access '%s'\n", current_key);
    next_json = json_object_get(current_json, current_key);
    if (!next_json) {
      if (*key_end) { // Not the end
        next_json = json_object();
        json_object_set(current_json, current_key, next_json);
      }
    }
    if (*key_end) {
      key_start = key_end + 1;
    } else {
      key_start = NULL;
    }
  }
  return current_json;
}

bool
validate(void)
{
  // Primitive validation, this shoudl be more generic
  bool valid = true;
  if (json_is_object(conf_doc)) {
    json_t *orbit = json_object_get(conf_doc, "openorbit");
    if (json_is_object(orbit)) {
      json_t *sys = json_object_get(orbit, "sys");
      json_t *video = json_object_get(orbit, "video");
      json_t *audio = json_object_get(orbit, "audio");
      json_t *sim = json_object_get(orbit, "sim");
      json_t *controls = json_object_get(orbit, "controls");

      if (json_is_object(sys)) {
        json_t *j = json_object_get(sys, "log-level");
        if (!json_is_string(j)) {
          valid = false;
          ooLogError("openorbit/sys/log-level should be a string");
        }
      } else valid = false;
      if (json_is_object(video)) {
        json_t *j = json_object_get(video, "fullscreen");
        if (!json_is_boolean(j)) {
          valid = false;
          ooLogError("openorbit/video/fullscreen should be a bool");
        }
        j = json_object_get(video, "width");
        if (!json_is_integer(j)) {
          valid = false;
          ooLogError("openorbit/video/width should be an integer");
        }
        j = json_object_get(video, "height");
        if (!json_is_integer(j)) {
          valid = false;
          ooLogError("openorbit/video/height should be an integer");
        }
        j = json_object_get(video, "depth");
        if (!json_is_integer(j)) {
          valid = false;
          ooLogError("openorbit/video/depth should be an integer");
        }
      } else {
        valid = false;
        ooLogError("openorbit/video is not an object");
      }

      if (json_is_object(audio)) {
      } else {
        valid = false;
        ooLogError("openorbit/audio is not an object");
      }

      if (json_is_object(sim)) {
        json_t *j = json_object_get(sim, "freq");
        if (!json_is_number(j)) valid = false;
        j = json_object_get(sim, "period");
        if (j && !json_is_number(j)) valid = false;
      } else {
        valid = false;
        ooLogError("openorbit/sim is not an object");
      }

      if (json_is_object(controls)) {
        json_t *j = json_object_get(controls, "keys");
        if (!json_is_array(j)) {
          valid = false;
          ooLogError("openorbit/controls/keys is not an array");
        }
        j = json_object_get(controls, "mouse");
        if (!json_is_object(j)) {
          valid = false;
          ooLogError("openorbit/controls/mouse is not an object");
        }
      } else {
        valid = false;
        ooLogError("openorbit/controls is not an object");
      }
    } else {
      valid = false;
      ooLogError("openorbit/ is not an object");
    }
  } else {
    valid = false;
    ooLogError("/ is not an object");
  }

  return valid;
}

MODULE_INIT(settings, NULL)
{
  ooLogTrace("initialising 'settings' module");

  //ooConfLoad(ooResGetConfPath());

  json_error_t err;
  conf_doc = json_load_file(rsrc_get_json_conf_path(), 0, &err);
  if (conf_doc == NULL) {
    // Either there was a parse error or the file was not found, if it was not
    // found, we copy the default config to the config directory.
    // conf_doc is not located where it should be, taken the default one and
    // copy it to the destination

    // OK, we know that access is not the best tool...
    if (access(rsrc_get_json_conf_path(), W_OK) == -1) {
      // Copy over default config
      char *path = rsrc_get_path("default-conf.json");
      mapped_file_t fIn = map_file(path);

      if (fIn.data) {
        FILE *fOut = fopen(rsrc_get_json_conf_path(), "wb");
        fwrite(fIn.data, 1, fIn.fileLenght, fOut);
        fclose(fOut);
        unmap_file(&fIn);
      }
      free(path);

      // Now try again...
      conf_doc = json_load_file(rsrc_get_json_conf_path(), 0, &err);
      if (conf_doc == NULL) {
        // Now lets panic...
        ooLogError("load %s:%d:%d: '%s'", err.source,
                   err.line, err.column, err.text);
        return;
      }
    } else {
      // Parse errro on the input file, we do not want to replace it
      ooLogFatal("load %s:%d:%d: '%s'", err.source,
                 err.line, err.column, err.text);
      return;
    }
  }

  if (!validate()) {
    ooLogError("config not compliant with specs");
  }
}



void
ooConfRegisterBoolDef(const char *key, bool val)
{
  json_t *json = lookup(key);

  if (!json) {
    json_t *parent = getparent(key);
    const char *end_key = strrchr(key, '/') + 1;
    assert(parent);
    json_object_set(parent, end_key, val ? json_true() : json_false());
  }
}

void
ooConfRegisterFloatDef(const char *key, float val)
{
  json_t *json = lookup(key);

  if (!json) {
    json_t *parent = getparent(key);
    const char *end_key = strrchr(key, '/') + 1;
    assert(parent);
    json_object_set(parent, end_key, json_real(val));
  }

}

void
ooConfRegisterIntegerDef(const char *key, long val)
{
  json_t *json = lookup(key);

  if (!json) {
    json_t *parent = getparent(key);
    const char *end_key = strrchr(key, '/') + 1;
    assert(parent);
    json_object_set(parent, end_key, json_integer(val));
  }
}

void
ooConfRegisterStringDef(const char *key, const char *val)
{
  json_t *json = lookup(key);

  if (!json) {
    json_t *parent = getparent(key);
    const char *end_key = strrchr(key, '/') + 1;
    assert(parent);
    json_object_set(parent, end_key, json_string(val));
  }
}




void
ooConfDelete()
{
  hrmlFreeDocument(gConfSingleton.doc);
  json_decref(conf_doc);
}


int
ooConfGetBoolDef(const char *key, bool *val, bool defVal)
{
  json_t *obj = lookup(key);
  if (json_is_true(obj)) {
    *val = true;
  } else if (json_is_false(obj)) {
    *val = false;
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetBoolAsIntDef(const char *key, int *val, int defVal)
{
  json_t *obj = lookup(key);
  if (json_is_true(obj)) {
    *val = 1;
  } else if (json_is_false(obj)) {
    *val = 0;
  } else {
    *val = defVal;
  }
  return 0;
}

int
ooConfGetIntDef(const char *key, int *val, int defVal)
{
  json_t *obj = lookup(key);
  if (json_is_integer(obj)) {
    *val = json_integer_value(obj);
  } else {
    *val = defVal;
  }
  return 0;
}

int
ooConfGetFloatDef(const char *key, float *val, float defVal)
{
  json_t *obj = lookup(key);
  if (json_is_real(obj)) {
    *val = json_real_value(obj);
  } else {
    *val = defVal;
  }
  return 0;
}

int
ooConfGetStrDef(const char *key, const char **val, const char *defVal)
{
  json_t *obj = lookup(key);
  if (json_is_string(obj)) {
    *val = json_string_value(obj);
  } else {
    *val = defVal;
  }
  return 0;
}

struct OOconfnode {
  json_t *obj;
  void *iter;
};

OOconfnode*
ooConfGetNode(const char *key)
{
  OOconfnode *conf = smalloc(sizeof(struct OOconfnode));
  conf->obj = lookup(key);
  conf->iter = json_object_iter(conf->obj);
  return conf;
}

OOconfarr*
ooConfGetNamedArr(const char *key)
{
  json_t *obj = lookup(key);
  if (json_is_array(obj)) {
    return (OOconfarr*)obj;
  }

  return NULL;
}


bool
ooConfNodeNext(OOconfnode *node)
{
  node->iter = json_object_iter_next(node->obj, node->iter);
  if (node->iter) return true;
  return false;
}

const char*
ooConfName(OOconfnode *node)
{
  return json_object_iter_key(node->iter);
}

bool
ooConfIsFloat(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  return json_is_real(obj);
}

bool
ooConfIsInt(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  return json_is_integer(obj);
}

bool
ooConfIsObj(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  return json_is_object(obj);
}

bool
ooConfIsBool(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);

  if (json_is_boolean(obj)) {
    return json_is_boolean(obj);
  }

  assert(0 && "not a bool");
  return false;
}

float
ooConfGetFloat(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);

  if (json_is_number(obj)) {
    return json_real_value(obj);
  }

  assert(0 && "not a real value");
  return NAN;
}

long
ooConfGetInt(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);

  if (json_is_integer(obj)) {
    return json_integer_value(obj);
  }

  assert(0 && "not an integer");
  return -1;
}

// TODO: fix type
OOconfnode*
ooConfGetObj(OOconfnode *node)
{
  json_t *val = json_object_iter_value(node->iter);

  if (json_is_object(val)) {
    OOconfnode *child = smalloc(sizeof(struct OOconfnode));
    child->obj = val;
    child->iter = json_object_iter(child->obj);
    return child;
  }

  assert(0 && "not an object");
  return NULL;
}

bool
ooConfGetBool(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);

  if (json_is_boolean(obj)) {
    return json_is_true(obj);
  }

  assert(0 && "not a bool");
  return false;
}

const char*
ooConfGetStr(OOconfnode *node)
{
  json_t *val = json_object_iter_value(node->iter);

  if (json_is_string(val)) {
    return json_string_value(val);
  }
  assert(0 && "not a string");
  return NULL;
}


void
ooConfNodeDispose(OOconfnode *node)
{
  free(node);
}


bool
ooConfGetBoolByName(OOconfnode *node, const char *name)
{
  json_t *val = json_object_get(node->obj, name);

  if (json_is_boolean(val)) {
    return json_is_true(val);
  }

  assert(0 && "not a bool");
  return false;
}

OOconfnode*
ooConfGetObjByName(OOconfnode *node, const char *name)
{
  json_t *val = json_object_get(node->obj, name);

  if (json_is_object(val)) {
    OOconfnode *child = smalloc(sizeof(struct OOconfnode));
    child->obj = val;
    child->iter = json_object_iter(child->obj);
    return child;
  }

  assert(0 && "not an object");
  return NULL;
}

long
ooConfGetIntByName(OOconfnode *node, const char *name)
{
  json_t *val = json_object_get(node->obj, name);
  return json_integer_value(val);
}
float ooConfGetFloatByName(OOconfnode *node, const char *name)
{
  json_t *val = json_object_get(node->obj, name);

  if (json_is_real(val)) {
    return json_real_value(val);
  }

  assert(0 && "not a float");
  return NAN;
}

const char*
ooConfGetStrByName(OOconfnode *node, const char *name)
{
  json_t *val = json_object_get(node->obj, name);

  if (json_is_string(val)) {
    return json_string_value(val);
  }
  assert(0 && "not a string");
  return NULL;
}

OOconfarr*
ooConfGetArr(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  if (json_is_array(obj)) {
    return (OOconfarr*)obj;
  }

  assert(0 && "not an array");
  return NULL;
}

OOconfarr*
ooConfGetArrByName(OOconfnode *node, const char *name)
{
  json_t *obj = json_object_get(node->obj, name);
  if (json_is_array(obj)) {
    return (OOconfarr*)obj;
  }

  assert(0 && "not an array");
  return NULL;
}


bool
ooConfIsArr(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  return json_is_array(obj);
}

bool
ooConfIsStr(OOconfnode *node)
{
  json_t *obj = json_object_iter_value(node->iter);
  return json_is_string(obj);
}


int
ooConfGetArrLen(OOconfarr *arr)
{
  json_t *jarr = (json_t*)arr;
  return json_array_size(jarr);
}

OOconfnode*
ooConfGetArrObj(OOconfarr *arr, int i)
{
  json_t *jarr = (json_t*)arr;
  json_t *obj = json_array_get(jarr, i);

  if (json_is_object(obj)) {
    OOconfnode *child = smalloc(sizeof(struct OOconfnode));
    child->obj = obj;
    child->iter = json_object_iter(child->obj);
    return child;
  } else {
    assert(0 && "invalid object ");
    return NULL;
  }
}

long
ooConfGetArrInt(OOconfarr *arr, int i)
{
  json_t *jarr = (json_t*)arr;
  json_t *obj = json_array_get(jarr, i);
  if (json_is_integer(obj)) {
    return json_integer_value(obj);
  }

  assert(0 && "invalid integer");
  return -1;
}

float
ooConfGetArrFloat(OOconfarr *arr, int i)
{
  json_t *jarr = (json_t*)arr;
  json_t *obj = json_array_get(jarr, i);
  if (json_is_real(obj)) {
    return json_real_value(obj);
  }

  assert(0 && "invalid float");
  return -1;
}
bool
ooConfGetArrBool(OOconfarr *arr, int i)
{
  json_t *jarr = (json_t*)arr;
  json_t *obj = json_array_get(jarr, i);
  if (json_is_boolean(obj)) {
    return json_is_true(obj);
  }

  assert(0 && "invalid bool");
  return -1;
}

const char*
ooConfGetArrStr(OOconfarr *arr, int i)
{
  json_t *jarr = (json_t*)arr;
  json_t *obj = json_array_get(jarr, i);
  if (json_is_string(obj)) {
    return json_string_value(obj);
  }

  assert(0 && "invalid string");
  return NULL;
}
