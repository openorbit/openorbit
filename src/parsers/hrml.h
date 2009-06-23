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
    created by the Initial Developer are Copyright (C) 2009 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

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

/*!
  Human Readable Markup Language Parser

  The human reaable markup language is usable when you don't want xml because
  it is not hand editable. This include the times when you have revision control
  on the data and you need to edit the data in programs such as Emacs and Vi.

  It supports schemas (or is intended to), and the basic syntax is as follows.

  document (schema: "http://blah"): {
    subtag (properties): {
      subkey (properties): value;
    }
  }

  A html document might be represented as (note that nodes may be unnamed):
    html () {
      head () {
        title: "My Document";
      }
      body () {
        p (id: foo): {
          : "My Paragraph";
          i: "Italic";
          : "more text";
      }
    }
  Note that the anonymous entry syntax : ""; is subject to change.
  The data is typed, and may consit of the following types: integers, reals,
  dates, strings, values with SI units and enums (defined in schemas).

  The properties include the reserved "uid" property that contains a unique id
  for the object, this allows the tree-like layout of the system to form a graph
*/
#ifndef HRML_H_SJH7PZB3
#define HRML_H_SJH7PZB3
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef enum HRMLtype {
  HRMLInt,
  HRMLFloat,
  HRMLStr,
  HRMLNode,
  HRMLDate,
  HRMLBool,
  HRMLIntArray,
  HRMLFloatArray,
  HRMLBoolArray,
  HRMLNull
} HRMLtype;

typedef enum HRMLsiprefix {
  HRML_yocto = 0,
  HRML_zepto,
  HRML_atto,
  HRML_femto,
  HRML_pico,
  HRML_nano,
  HRML_micro,
  HRML_milli,
  HRML_centi,
  HRML_deci,
  HRML_none,
  HRML_deca,
  HRML_hecto,
  HRML_kilo,
  HRML_mega,
  HRML_giga,
  HRML_tera,
  HRML_peta,
  HRML_exa,
  HRML_zetta,
  HRML_yotta,
  HRML_siprefix_size
} HRMLsiprefix;

typedef enum HRMLsitype {
  /* First the base units */
  HRML_metre = 0,
  HRML_gram, /* Note that since the base is actually kg, you have to use the prefix to set the proper unit */
  HRML_second,
  HRML_ampere,
  HRML_kelvin,
  HRML_mole,
  HRML_candela,

  /* Derived units */
  HRML_hertz,
  HRML_radian,
  HRML_steradian,
  HRML_newton,
  HRML_pascal,
  HRML_joule,
  HRML_watt,
  HRML_coulomb,
  HRML_volt,
  HRML_farad,
  HRML_ohm,
  HRML_siemens,
  HRML_weber,
  HRML_tesla,
  HRML_henry,
  HRML_celsius,
  HRML_lumen,
  HRML_lux,
  HRML_becquerel,
  HRML_gray,
  HRML_sievert,
  HRML_katal,

  /* Other units */
  HRML_minute,
  HRML_hour,
  HRML_day,
  HRML_arcdeg,
  HRML_arcminute,
  HRML_arcsec,
  HRML_hectare,
  HRML_litre,
  HRML_tonne,

  HRML_electronvolt,
  HRML_atomic_mass_unit,
  HRML_astronomical_unit,

  HRML_parsec,
  HRML_lightyear,

  HRML_bar,
  HRML_siunit_size,
  HRML_siunit_invalid
} HRMLsitype;

typedef struct HRMLvalue {
  HRMLtype typ;
  size_t alen; // Array length if applicable
  union {
    uint64_t integer;
    uint64_t *intArray;

    double real;
    double *realArray;

    char *str;
    bool boolean;
    bool *boolArray;

    struct {
      double val;
      HRMLsiprefix siPrefix;
      HRMLsitype siType;
    } typedVal;
  } u;
} HRMLvalue;

typedef struct HRMLattr {
  char *name;
  HRMLvalue val;
  struct HRMLattr *next;
} HRMLattr;


typedef struct HRMLattrlist {
  size_t allocLen;
  size_t attrCount;
  char **names;
  HRMLvalue *values;
} HRMLattrlist;

typedef struct HRMLobject {
  char *name;
  HRMLattrlist *attrs;
  HRMLvalue val;
  struct HRMLobject *previous;
  struct HRMLobject *next;
  struct HRMLobject *children;
} HRMLobject;

static inline HRMLvalue
hrmlGetAttrForName(HRMLobject *obj, const char *attrName)
{
  for (size_t i = 0 ; i < obj->attrs->attrCount; i ++) {
    if (!strcmp(attrName, obj->attrs->names[i])) {
      return obj->attrs->values[i];
    }
  }

  HRMLvalue nullVal = {.typ = HRMLNull, .u.integer = 0};
  return nullVal;
}

static inline uint64_t
hrmlGetInt(HRMLobject *obj)
{
  assert(obj->val.typ == HRMLInt);
  return obj->val.u.integer;
}

static inline double
hrmlGetReal(HRMLobject *obj)
{
  assert(obj->val.typ == HRMLFloat);
  return obj->val.u.real;
}

static inline const char*
hrmlGetStr(HRMLobject *obj)
{
  assert(obj->val.typ == HRMLStr);
  return obj->val.u.str;
}

static inline HRMLobject*
hrmlGetNode(HRMLobject *obj)
{
  assert(obj->val.typ == HRMLNode);
  return obj->children;
}

typedef struct HRMLdocument {
  HRMLobject *rootNode;
} HRMLdocument;

static inline HRMLobject*
hrmlGetNodeHead(HRMLobject *node) {
  assert(node->val.typ == HRMLNode);
  return node->children;
}

static inline HRMLobject*
hrmlGetNodeTail(HRMLobject *node) {
  assert(node->val.typ == HRMLNode);
  return node->children;
}


static inline HRMLobject*
hrmlGetRoot(HRMLdocument *doc){
  return doc->rootNode;
}

typedef struct HRMLschema {

} HRMLschema;

HRMLdocument* hrmlParse(FILE *f);
bool hrmlValidate(HRMLdocument *doc, HRMLschema *sc);
void hrmlFreeDocument(HRMLdocument *doc);
HRMLobject* hrmlGetObject(HRMLdocument *doc, const char *docPath);


// Node iteration macros
#define FOR_ALL_ATTR(attrib, node) \
   for (HRMLattr *attrib = node->attr ; attrib != NULL; attrib = attrib->next)

#define FOR_ALL_CHILDREN(obj, node) \
   for (HRMLobject *obj = node->children ; obj != NULL; obj = obj->next)



#endif /* end of include guard: HRML_H_SJH7PZB3 */

