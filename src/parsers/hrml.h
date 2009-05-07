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

typedef enum HRMLtype {
  HRMLInt,
  HRMLFloat,
  HRMLStr,
  HRMLNode,
  HRMLDate,
} HRMLtype;

typedef struct HRMLlist HRMLlist;

typedef struct HRMLlistentry HRMLlistentry;

typedef struct HRMLobject {
  HRMLtype typ;
  union {
    uint64_t integer;
    double real;
    char *str;
    HRMLlist *node;
  } u;
} HRMLobject;

struct HRMLlistentry {
  HRMLlistentry *previous;
  HRMLlistentry *next; 
  HRMLobject *data;
};

struct HRMLlist {
  HRMLlistentry *head;
  HRMLlistentry *tail;
};

typedef struct HRMLdocument {
  HRMLobject *rootNode;
} HRMLdocument;

typedef struct HRMLschema {
  
} HRMLschema;

typedef HRMLlistentry HRMLiterator;
  
HRMLdocument* hrmlParse(FILE *f);
bool hrmlValidate(HRMLdocument *doc, HRMLschema *sc);
HRMLiterator *hrmlRootIterator(HRMLdocument *doc);
HRMLiterator *hrmlIteratorNext(HRMLiterator *it);
HRMLtype hrmlIteratorType(HRMLiterator *it);
HRMLobject* hrmlIteratorValue(HRMLiterator *it);


#endif /* end of include guard: HRML_H_SJH7PZB3 */

