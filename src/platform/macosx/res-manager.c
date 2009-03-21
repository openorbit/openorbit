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

// OSX Headers
#include <CoreFoundation/CoreFoundation.h>

// Posix and UNIX headers
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <glob.h>
// Open Orbit headers
#include "res-manager.h"
#include "log.h"

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
ooResGetBasePath(void)
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
ooResGetPath(const char *fileName)
{
    assert(fileName != NULL && "File name cannot be null");
        
    char *path;
    asprintf(&path, "%s/%s\0", ooResGetBasePath(), fileName);
    
    return path; // Note, asprintf sets this to NULL if it fails
}

glob_t
ooResGetFilePaths(const char *pattern)
{
  const char *base = ooResGetBasePath();
  
  char *fullPattern;
  asprintf(&fullPattern, "%s/%s\0", base, pattern);
  ooLogInfo("search for %s", fullPattern);
  glob_t globs;
  glob(fullPattern, 0, NULL, &globs);

  free(fullPattern);
  return globs;
}


FILE*
ooResGetFile(const char *fileName)
{
    char *path = ooResGetPath(fileName);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        if (!file) ooLogWarn("file %s not readable", path);
        free(path);
        
        return file;
    }

    return NULL;
}

int
ooResGetFd(const char *fileName)
{
    char *path = ooResGetPath(fileName);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        
        if (fd == -1) ooLogWarn("file %s not readable", path);
        
        free(path);
        
        return fd;
    }

    return -1;
}


char*
ooPluginGetPath(const char *fileName)
{
    
}

FILE*
ooPluginGetFile(const char *fileName)
{
    char *path = ooPluginGetPath(fileName);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
        free(path);
        return file;
    }
    
    return NULL;    
}

int
ooPluginGetFd(const char *fileName)
{
    char *path = ooPluginGetPath(fileName);
    
    if (path != NULL) {
        int fd = open(path, O_RDONLY);
        free(path);
        
        return fd;
    }
    
    return -1;    
}

