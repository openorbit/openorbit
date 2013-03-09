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

/*! 
    Resource manager header
 
    The resource manager is not a manager per see at the moment, but might
    become one in the future. At the moment it supplies a number of functions
    that can be used to locate resource files in a platform independent manner.
    
    Resource location is heavily dependant on the target platform. On the Mac
    for example, resources are expected to be located in the application bundle
    and we use the core foundataion API to locate these. On normal posix
    systems (not including MacOS X), we will probably just look through
    $PREFIX/share/openorbit/resources and $HOME/.openorbit/resources. On the Mac
    the user directories will be
    "~/Library/Application Support/Open Orbit/Resources".
 */
#ifndef _RES_MANAGER_H_
#define _RES_MANAGER_H_
#include <stdio.h>
#include <glob.h>

const char* rsrc_get_conf_path(void);
const char* rsrc_get_json_conf_path(void);

char* rsrc_get_path(const char *fileName);
FILE* rsrc_get_file(const char *fileName);
int rsrc_get_fd(const char *fileName);
glob_t rsrc_get_file_paths(const char *pattern);

char* rsrc_get_plugin_path(const char *fileName);
FILE* rsrc_get_plugin_file(const char *fileName);
int rsrc_get_plugin_fd(const char *fileName);

#endif /* _RES_MANAGER_H_ */
