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

// Open Orbit headers
#include "res-manager.h"



// TODO: replace with system constant
#define MAX_PATH_LEN 1024

// TODO: More thurough error checking and handling
char*
res_get_path(const char *file_name)
{
    assert(file_name != NULL);
    
    
    char *end = strrchr(file_name, '\0');
    char *last_dot = strrchr(file_name, '.');
    char *last_slash = strrchr(file_name, '/');
    
    if (! last_dot) {
        return NULL;
    }

    char *fileBase;
    char *fileType;
    char *fileSubDir;

    if (last_slash) {
        fileBase = malloc(last_dot - last_slash + 1);
        fileType = malloc(end - last_dot + 1);
        fileSubDir = malloc(last_slash - file_name + 1);
    } else {
        fileBase = malloc(last_dot - file_name + 1);
        fileType = malloc(end - last_dot + 1);
        fileSubDir = NULL;
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
    
    UInt8 *path = malloc(MAX_PATH_LEN+1);
    // true if absolute path is to be used...
    if (!CFURLGetFileSystemRepresentation(url, false, path, MAX_PATH_LEN)) {
        free(path);
        path = NULL;
    }
    
    free(fileBase);
    free(fileType);
    free(fileSubDir);
    CFRelease(resName);
    CFRelease(resType);
    CFRelease(resSubDir);
    CFRelease(url);
    
    return (char*)path;
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
