/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


/* Main test program file used by all unit tests.
 * This will not deallocate the allocated space on termination
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>

Suite *test_suite(int argc, char **argv);

int
main(int argc, char **argv)
{
    int number_failed;
    Suite *s = test_suite(argc, argv);
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_VERBOSE);
    
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
