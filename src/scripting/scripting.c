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