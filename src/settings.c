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
