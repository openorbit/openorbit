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


#include "settings.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// TODO: Remove libconfig req
//#include <libconfig.h>

#include "parsers/hrml.h"
#include "log.h"

struct OOconf
{
  HRMLdocument *doc;
};


static OOconf gConfSingleton;

void
ooConfInit(void)
{
  gConfSingleton.doc = NULL;
}

void
ooConfLoad(const char *name)
{
  HRMLdocument *doc = hrmlParse(name);
  if (!doc) {
    ooLogError("could not load config file '%s'", name);
  }
  
  gConfSingleton.doc = doc;
}

void
ooConfDelete()
{
  hrmlFreeDocument(gConfSingleton.doc);
}


int
ooConfGetBoolDef(const char *key, bool *val, bool defVal)
{
  HRMLobject *obj = hrmlGetObject(gConfSingleton.doc, key);
  if (obj && obj->val.typ == HRMLBool) {
    *val = hrmlGetBool(obj);
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetBoolAsIntDef(const char *key, int *val, int defVal)
{
  HRMLobject *obj = hrmlGetObject(gConfSingleton.doc, key);
  if (obj && obj->val.typ == HRMLBool) {
    *val = (int) hrmlGetBool(obj);
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetIntDef(const char *key, int *val, int defVal)
{
  HRMLobject *obj = hrmlGetObject(gConfSingleton.doc, key);
  if (obj && obj->val.typ == HRMLInt) {
    *val = hrmlGetInt(obj);
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetFloatDef(const char *key, float *val, float defVal)
{
  HRMLobject *obj = hrmlGetObject(gConfSingleton.doc, key);
  if (obj && obj->val.typ == HRMLFloat) {
    *val = hrmlGetReal(obj);
  } else {
    *val = defVal;
  }

  return 0;
}

int
ooConfGetStrDef(const char *key, const char **val, const char *defVal)
{
  HRMLobject *obj = hrmlGetObject(gConfSingleton.doc, key);
  if (obj && obj->val.typ == HRMLStr) {
    *val = hrmlGetStr(obj);
  } else {
    *val = defVal;    
  }

  return 0;
}

HRMLobject*
ooConfGetNode(const char *key)
{
  return hrmlGetObject(gConfSingleton.doc, key);
}
