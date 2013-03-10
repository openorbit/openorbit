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

// Win Headers

// Posix and UNIX headers
#include <fcntl.h>
#include <string.h>
#include <limits.h>

// Open Orbit headers
#include "res-manager.h"


// TODO: More thurough error checking and handling
char*
rsrc_get_path(const char *file_name)
{
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

