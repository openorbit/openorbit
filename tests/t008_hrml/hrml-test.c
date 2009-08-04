/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdlib.h>
#include <check.h>
#include "hrml.h"

static char *validHrmlFileName;

START_TEST(test_valid)
{
  HRMLdocument *doc = hrmlParse(validHrmlFileName);
  fail_if( doc == 0, "hrmlParse returned null");
  hrmlFreeDocument(doc);
}
END_TEST

START_TEST(test_path)
{
  HRMLdocument *doc = hrmlParse(validHrmlFileName);

  HRMLobject *obj = hrmlGetObject(doc, "document/header/title");
  fail_if(obj == NULL);
  fail_unless(obj->val.typ == HRMLStr);
  fail_if(strcmp(obj->val.u.str, "My Document"));

  HRMLobject *obj2 = hrmlGetObject(doc, "document/header/blaht");
  fail_unless(obj2 == NULL);

  HRMLobject *obj3 = hrmlGetObject(doc, "document/body/paragraph/anArray");
  fail_if(obj3 == NULL);
  fail_unless(obj3->val.typ == HRMLFloatArray);
  fail_unless(obj3->val.alen == 6);
  fail_unless(obj3->val.u.realArray[0] == 0.0);
  fail_unless(obj3->val.u.realArray[5] == 5.5);

  hrmlFreeDocument(doc);
}
END_TEST

Suite
*test_suite(int argc, char **argv)
{
    if (argc != 1) {
      validHrmlFileName = argv[1];
    }

    Suite *s = suite_create ("HRML Test");

    /* Core test case */
    TCase *tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_valid);
    tcase_add_test(tc_core, test_path);

    suite_add_tcase(s, tc_core);

    return s;
}

