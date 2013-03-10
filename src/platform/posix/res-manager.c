/*
  Copyright 2006,2013 Mattias Holm <lorrden(at)openorbit.org>

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
rsrc_get_path(const char *file_name)
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
rsrc_get_file(const char *file_name)
{
    char *path = rsrc_get_path(file_name);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }

    return NULL;
}

int
rsrc_get_fd(const char *file_name)
{
    char *path = rsrc_get_path(file_name);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);
        
        return fd;
    }

    return -1;
}


char*
rsrc_get_plugin_path(const char *file_name)
{
    
}

FILE*
rsrc_get_plugin_file(const char *file_name)
{
    char *path = rsrc_get_plugin_path(file_name);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }
    
    return NULL;    
}

int
rsrc_get_plugin_fd(const char *file_name)
{
    char *path = rsrc_get_plugin_path(file_name);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);
        
        return fd;
    }
    
    return -1;    
}

