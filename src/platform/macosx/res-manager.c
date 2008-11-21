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

// Open Orbit headers
#include "res-manager.h"


// TODO: More thurough error checking and handling
char*
ooResGetPath(const char *fileName)
{
    assert(fileName != NULL);
    
    // The resources are located without the full name on the mac, we have to
    // split up the name of the file and its extension
    char *end = strrchr(fileName, '\0');
    char *last_dot = strrchr(fileName, '.');
    char *last_slash = strrchr(fileName, '/');
    
    if (! last_dot) {
        return NULL;
    }

    char *fileBase;
    char *fileType;
    char *fileSubDir;

    if (last_slash) {
        fileBase = malloc(last_dot - last_slash + 1);
        fileType = malloc(end - last_dot + 1);
        fileSubDir = malloc(last_slash - fileName + 1);
        
        memset(fileBase, 0, last_dot - last_slash + 1);
        memset(fileType, 0, end - last_dot + 1);
        memset(fileSubDir, 0, last_slash - fileName + 1);
        
        memcpy(fileBase, last_slash + 1, last_dot - last_slash);
        memcpy(fileType, last_dot + 1, end - last_dot);
        memcpy(fileSubDir, fileName, last_slash - fileName);
    } else {
        fileBase = malloc(last_dot - fileName + 1);
        fileType = malloc(end - last_dot + 1);
        fileSubDir = NULL;

        memset(fileBase, 0, last_dot - fileName + 1);
        memset(fileType, 0, end - last_dot + 1);
        
        memcpy(fileBase, fileName, last_dot - fileName);
        memcpy(fileType, last_dot + 1, end - last_dot);
    }

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle == NULL) return NULL;
    
    CFStringRef resName = CFStringCreateWithCString(kCFAllocatorDefault,
                                                    fileBase,
                                                    kCFStringEncodingUTF8);

    CFStringRef resType = CFStringCreateWithCString(kCFAllocatorDefault,
                                                    fileType,
                                                    kCFStringEncodingUTF8);

    CFStringRef resSubDir;
    if (fileSubDir != NULL) {
        resSubDir = CFStringCreateWithCString(kCFAllocatorDefault,
                                              fileSubDir,
                                              kCFStringEncodingUTF8);
    } else {
        resSubDir = NULL;
    }
    
    CFURLRef url = CFBundleCopyResourceURL(bundle, resName, resType, resSubDir);
    
    UInt8 *path = malloc(PATH_MAX+1);
    // true if absolute path is to be used...
    if (!CFURLGetFileSystemRepresentation(url, false, path, PATH_MAX)) {
        free(path);
        path = NULL;
    }
    
    free(fileBase);
    free(fileType);
    free(fileSubDir);
    if (resName) CFRelease(resName);
    if (resType) CFRelease(resType);
    if (resSubDir) CFRelease(resSubDir);
    if (url) CFRelease(url);
    
    return (char*)path;
}

FILE*
ooResGetFile(const char *fileName)
{
    char *path = ooResGetPath(fileName);
    
    if (path != NULL) {
        FILE *file = fopen(path, "r");
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

