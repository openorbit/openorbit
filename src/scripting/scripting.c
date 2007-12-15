/* Open Orbit - An interactive spacecraft simulator.
 * Copyright (C) 2006  Mattias Holm (mattias.holm@contra.nu)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdarg.h>
#include <stdint.h>

 
#ifdef WITH_PYTHON

#include <Python.h>

#include "io.h"
#include "config.h"

#endif /* WITH_PYTHON */
 
#include "scripting.h"

#ifdef WITH_PYTHON
bool
init_scripting(void)
{
    Py_Initialize();
    initio();
    initconfig();
    oo_error_t err = load_setup_script();
    
    if (err) {
        print_error_message(err);
        return true;
    }
    
    return false;
}

void
finilise_scripting(void)
{
    Py_Finalize();
}

oo_error_t
load_setup_script(void)
{
    char *home=getenv("HOME");
    assert(home);
    
    char *path = malloc((strlen(home) +
                         strlen("/.openorbit/" SCR_INIT_SCRIPT_NAME)+1) *
                         sizeof(char));
    if (!path) {
        return ERROR_MEM;
    }
    
    strcpy(path, home);
    strcat(path, "/.openorbit/" SCR_INIT_SCRIPT_NAME);    
    
    FILE *fp = fopen(path, "r");
    if (! fp) {
        printf("%s\n", path);
        free(path);
        return ERROR_FILE_NOT_FOUND;
    }
    if (PyRun_SimpleFile(fp, SCR_INIT_SCRIPT_NAME)) {
        free(path);
        return ERROR_SCRIPT;
    }
    
    free(path);
    return ERROR_NONE;
}

#elif defined(WITH_GUILE)
bool
init_scripting(void)
{
    oo_error_t err = load_setup_script();
    
    if (err) {
        print_error_message(err);
        return true;
    }
    
    return false;
}

void
finilise_scripting(void)
{
}

oo_error_t
load_setup_script(void)
{
    char *home=getenv("HOME");
    assert(home);
    
    char *path = malloc((strlen(home) +
                         strlen("/.openorbit/" SCR_INIT_SCRIPT_NAME)+1) *
                         sizeof(char));
    if (!path) {
        return ERROR_MEM;
    }
    
    strcpy(path, home);
    strcat(path, "/.openorbit/" SCR_INIT_SCRIPT_NAME);
    
    scm_c_primitive_load(path);

    free(path);
    return ERROR_NONE;
}

#else /* WITH_GUILE */
    #error "Enable Guile or Python"
#endif 