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

// OSX Headers
#include <CoreFoundation/CoreFoundation.h>

// Posix and UNIX headers
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <glob.h>
// Open Orbit headers
#include "res-manager.h"
#include <openorbit/log.h>

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr/local"
#endif /* ! INSTALL_PREFIX */

    //#ifdef _UNIX_
        static char *paths[] = {
            "~/.openorbit/plugins/",
            INSTALL_PREFIX "/share/openorbit/plugins"
    #ifdef __APPLE__
            ,
            "~/Library/Application Support/Open Orbit/Plugins",
            "/Library/Application Support/Open Orbit/Plugins",
            "/Network/Library/Application Support/Open Orbit/Plugins"
    #endif /* __APPLE__ */
        };
    //#endif /* _UNIX_ */

const char*
rsrc_get_conf_path(void)
{
  char *homeDir = getenv("HOME");
  ooLogFatalIfNull(homeDir, "$HOME not set");

  static char *confPath = NULL;

  if (confPath == NULL) {
    asprintf(&confPath, "%s/%s", homeDir, "Library/Preferences/org.openorbit.conf");
  }

  return confPath;
}

const char*
rsrc_get_json_conf_path(void)
{
  char *homeDir = getenv("HOME");
  ooLogFatalIfNull(homeDir, "$HOME not set");

  static char *confPath = NULL;

  if (confPath == NULL) {
    asprintf(&confPath, "%s/%s", homeDir, "Library/Preferences/org.openorbit.json");
  }

  return confPath;
}



const char*
rsrc_get_base_path(void)
{
    static UInt8 base[PATH_MAX+1] = {0};

    // If first call, get the path to the bundle's resource directory
    if (base[0] == '\0') {
        CFBundleRef bundle = CFBundleGetMainBundle();
        CFURLRef resUrl = CFBundleCopyResourcesDirectoryURL(bundle);
        // use absolute path
        if (!CFURLGetFileSystemRepresentation(resUrl, true, base, PATH_MAX)) {
            // Something went wrong
            CFRelease(resUrl);
            ooLogFatal("%s:%d:base path not found", __FILE__, __LINE__); // no ret
        }

        CFRelease(resUrl);

        ooLogTrace("res base = %s", base);
    }

    return (const char*)base;
}

char*
rsrc_get_path(const char *fileName)
{
    assert(fileName != NULL && "File name cannot be null");

    char *path;
    asprintf(&path, "%s/%s", rsrc_get_base_path(), fileName);

    return path; // Note, asprintf sets this to NULL if it fails
}

glob_t
rsrc_get_file_paths(const char *pattern)
{
  const char *base = rsrc_get_base_path();

  char *fullPattern;
  asprintf(&fullPattern, "%s/%s", base, pattern);
  ooLogTrace("search for %s", fullPattern);
  glob_t globs;
  glob(fullPattern, 0, NULL, &globs);

  free(fullPattern);
  return globs;
}


FILE*
rsrc_get_file(const char *fileName)
{
    char *path = rsrc_get_path(fileName);

    if (path != NULL) {
        FILE *file = fopen(path, "r");
        if (!file) ooLogWarn("file %s not readable", path);
        free(path);

        return file;
    }

    return NULL;
}

int
rsrc_get_fd(const char *fileName)
{
    char *path = rsrc_get_path(fileName);

    if (path != NULL) {
        int fd = open(path, O_RDONLY);

        if (fd == -1) ooLogWarn("file %s not readable", path);

        free(path);

        return fd;
    }

    return -1;
}


char*
rsrc_get_plugin_path(const char *fileName)
{
  (void)paths; // TODO
  return NULL;
}

FILE*
rsrc_get_plugin_file(const char *fileName)
{
    char *path = rsrc_get_plugin_path(fileName);

    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }

    return NULL;
}

int
rsrc_get_plugin_fd(const char *fileName)
{
    char *path = rsrc_get_plugin_path(fileName);

    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);

        return fd;
    }

    return -1;
}

