/* 
    The contents of this file are subject to the Mozilla Public License
    Version 1.1 (the "License"); you may not use this file except in compliance
    with the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Original Code is the Open Orbit space flight simulator.

    The Initial Developer of the Original Code is Mattias Holm. Portions
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
    which case the provisions of GPL or the LGPL License are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of the GPL or the LGPL and not to allow others to use your
    version of this file under the MPL, indicate your decision by deleting the
    provisions above and replace  them with the notice and other provisions
    required by the GPL or the LGPL.  If you do not delete the provisions
    above, a recipient may use your version of this file under either the MPL,
    the GPL or the LGPL."
 */


#include "settings.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>
#include <gencds/hashtable.h>
#include <assert.h>

// TODO: Remove libconfig req
#include <libconfig.h>

#include "parsers/hrml.h"
#include "log.h"

typedef enum {
    CONF_int,
    CONF_float,
    CONF_str,
    CONF_bool
} conf_val_kind_t;

typedef struct {
    conf_val_kind_t kind;
    union {
        int i;
        float f;
        char *str;
        bool b;
    } u;
} conf_val_t;


hashtable_t *g_config;
config_t conf;
void
ooConfInit(void)
{
    g_config = hashtable_new_with_str_keys(512);
    assert(g_config != NULL);

    config_init(&conf);
}

void
ooConfLoad(const char *name)
{
  if (! config_read_file(&conf, name)) {
    ooLogError("%s:%d %s", name, config_error_line(&conf),
               config_error_text(&conf));
  }

  char hrmlFileName[strlen(name) + 5 + 1];
  strcpy(hrmlFileName, name);
  strncat(hrmlFileName, ".hrml", 5);
  HRMLdocument *doc = hrmlParse(hrmlFileName);
  hrmlFreeDocument(doc);
}

// Entry point for reading options from file, at the moment this is handled by
// reading an .ini file in the startup script, I would prefer in the long run that
// this was moved into a simple file parser reading some c-like configuration
// files.
// E.g.:
// enum loglev {trace = 0, info = 1, warn = 2, err = 3, fatal = 4};
// config default {
//    log.level = trace;
//    log.file = "stderr";
//    video.fullscreen = true;
//    video.gl.fovy = 45.0;
// }
void
ooConfParse(const char *file)
{
  // TODO: Hook this up with yacc (yyparse, but we want multiple parsers
  // embedded, so yyprefix must be changed)
}

int
ooConfSetInt(const char *key, int val)
{
    conf_val_t *confval;
    if (confval = hashtable_lookup(g_config, key)) {
        if (confval->kind == CONF_str) {
            free(confval->u.str);
        }
        confval->kind = CONF_int;
        confval->u.i = val;
    } else {
        confval = malloc(sizeof(conf_val_t));
        if (confval) {
            confval->kind = CONF_int;
            confval->u.i = val;
            hashtable_insert(g_config, key, confval);
        } else {
            return -1;
        }
    }
    return 0;
}
int
ooConfSetBool(const char *key, bool val)
{
    conf_val_t *confval;

    if (confval = hashtable_lookup(g_config, key)) {
        if (confval->kind == CONF_str) {
            free(confval->u.str);
        }
        confval->kind = CONF_bool;
        confval->u.b = val;
    } else {
        confval = malloc(sizeof(conf_val_t));
        if (confval) {
            confval->kind = CONF_bool;
            confval->u.b = val;
            hashtable_insert(g_config, key, confval);
        } else {
            return -1;
        }
    }
    return 0;
}

int
ooConfSetBoolAsInt(const char *key, int val)
{
    conf_val_t *confval;
    
    if (confval = hashtable_lookup(g_config, key)) {
        if (confval->kind == CONF_str) {
            free(confval->u.str);
        }
        confval->kind = CONF_bool;
        confval->u.b = val;
    } else {
        confval = malloc(sizeof(conf_val_t));
        if (confval) {
            confval->kind = CONF_bool;
            confval->u.b = (bool)val;
            hashtable_insert(g_config, key, confval);
        } else {
            return -1;
        }
    }
    return 0;
}

int
ooConfSetFloat(const char *key, float val)
{
    conf_val_t *confval;
    if (confval = hashtable_lookup(g_config, key)) {
        if (confval->kind == CONF_str) {
            free(confval->u.str);
        }
        confval->kind = CONF_float;
        confval->u.f = val;
    } else {
        confval = malloc(sizeof(conf_val_t));
        if (confval) {
            confval->kind = CONF_float;
            confval->u.f = val;
            hashtable_insert(g_config, key, confval);
        } else {
            return -1;
        }
    }
    return 0;
}

int
ooConfSetStr(const char *key, char *val)
{
    conf_val_t *confval;
    if (confval = hashtable_lookup(g_config, key)) {
        if (confval->kind == CONF_str) {
            free(confval->u.str);
        }
        confval->kind = CONF_str;
        confval->u.str = strdup(val);
    } else {
        confval = malloc(sizeof(conf_val_t));
        if (confval) {
            confval->kind = CONF_int;
            confval->u.str = strdup(val);
            hashtable_insert(g_config, key, confval);
        }
    }

    if (! confval || ! confval->u.str) {
        return -1;
    }

    return 0;
}

int
ooConfGetInt(const char *key, int *val)
{
    conf_val_t *confval = hashtable_lookup(g_config, key);
    if (confval && (confval->kind == CONF_int)) {
        *val = confval->u.i;
        return 0;
    }

    ooLogWarn("settings:no integer %s exists", key);
    return -1;
}

int
ooConfGetBool(const char *key, bool *val)
{
    conf_val_t *confval = hashtable_lookup(g_config, key);
    if (confval && (confval->kind == CONF_bool)) {
        *val = confval->u.b;
        return 0;
    }

    ooLogWarn("settings:no boolean %s exists", key);
    return -1;
}

int
ooConfGetBoolAsInt(const char *key, int *val)
{
    conf_val_t *confval = hashtable_lookup(g_config, key);
    if (confval && (confval->kind == CONF_bool)) {
        *val = (int)confval->u.b;
        return 0;
    }

    ooLogWarn("settings:no boolean %s exists", key);    
    return -1;
}



int
ooConfGetFloat(const char *key, float *val)
{
    conf_val_t *confval = hashtable_lookup(g_config, key);
    if (confval && (confval->kind == CONF_float)) {
        *val = confval->u.f;
        return 0;
    }

    ooLogWarn("settings:no real %s exists", key);
    return -1;
}

char*
ooConfGetStr(const char *key)
{
    conf_val_t *confval = hashtable_lookup(g_config, key);
    if (confval && (confval->kind == CONF_str)) {
        return confval->u.str;
    }

    ooLogWarn("settings:no string %s exists", key);
    return NULL;    
}


int
ooConfGetBoolDef(const char *key, bool *val, bool defVal)
{
  int configVal;

  if (config_lookup_bool(&conf, key, &configVal)) {
    *val = (bool) configVal;
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetBoolAsIntDef(const char *key, int *val, int defVal)
{
  int configVal;

  if (config_lookup_bool(&conf, key, &configVal)) {
    *val = configVal;
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetIntDef(const char *key, int *val, int defVal)
{
  long configVal;

  if (config_lookup_int(&conf, key, &configVal)) {
    *val = configVal;
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetFloatDef(const char *key, float *val, float defVal)
{
  double configVal;

  if (config_lookup_float(&conf, key, &configVal)) {
    *val = configVal;
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetStrDef(const char *key, const char **val, const char *defVal)
{
  const char *configVal;

  if (config_lookup_string(&conf, key, &configVal)) {
    *val = configVal;
  } else {
    *val = defVal;
  }

  return 0;
}
