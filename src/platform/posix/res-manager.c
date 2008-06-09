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


// Posix and UNIX headers
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_CONFIG
#include "config.h"
#endif

// Open Orbit headers
#include "res-manager.h"

// NOTE: The pure posix version is based on searching two static directories
//       for plugins. One is defined when configuring the build and defaults
//       to $INSTALL/share/openorbit/plugin and $HOME/.openorbit/plugin
//       The base path is defined as INSTALL_BASE in config.h
// TODO: More thurough error checking and handling
char*
res_get_path(const char *file_name)
{
    if (file_name == NULL) {
        fprintf(stderr, "res_get_path called with NULL file_name\n");
        return NULL;
    }
    path = malloc(PATH_MAX+1);
    if (path == NULL) {
        fprintf(stderr, "res_get_path out of memory\n");
        return NULL;
    }
    
    char *home = getenv("HOME");
    if (home != NULL) {
        strncpy(path, home, strlen(home));
        strcat(path, "/.openorbit/resources/");
        strncat(path, file_name, fname_len);
        int fd = open(path, O_RDONLY);
        if (fd != -1) {
            // file exists and is valid
            close(fd);
            return path;
        }
    }
    
    // we get here if the file in HOME/.openorbit does not exist, so we query
    // in the install dir
    size_t base_len = strlen(INSTALL_BASE);
    size_t fname_len = strlen(file_name);

    strncpy(path, INSTALL_BASE, base_len);
    strncat(path, file_name, fname_len);
    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        // file exists and is valid
        close(fd);
        return path;
    }
    
    
    return NULL;
}

FILE*
res_get_file(const char *file_name)
{
    char *path = res_get_path(file_name);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }

    return NULL;
}

int
res_get_fd(const char *file_name)
{
    char *path = res_get_path(file_name);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);
        
        return fd;
    }

    return -1;
}


char*
plugin_get_path(const char *file_name)
{
    
}

FILE*
plugin_get_file(const char *file_name)
{
    char *path = plugin_get_path(file_name);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }
    
    return NULL;    
}

int
plugin_get_fd(const char *file_name)
{
    char *path = plugin_get_path(file_name);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);
        
        return fd;
    }
    
    return -1;    
}

