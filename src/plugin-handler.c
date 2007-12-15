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

#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "plugin-handler.h"
#include "libgencds/hashtable.h"

#ifndef SO_EXT
#define SO_EXT ".so"
#endif /* ! SO_EXT */

#ifndef INSTALL_PREFIX
#define INSTALL_PREFIX "/usr/local"
#endif /* ! INSTALL_PREFIX */

static hashtable_t *PLUGIN_interface_dict = NULL;
static hashtable_t *PLUGIN_dict = NULL;

bool
init_plugin_manager(void)
{
    if (! (PLUGIN_interface_dict = hashtable_new_with_str_keys(512)) ) return false;
    if (! (PLUGIN_dict = hashtable_new_with_str_keys(512)) ) {
        hashtable_delete(PLUGIN_interface_dict);
        return false;
    }
    return true;
}

/* read in all dynlibs in the plug-in directories */
bool
load_plugins(void)
{

//#ifdef _UNIX_
    char *paths[] = {
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
    
    for (int i = 0 ; i < sizeof(paths) ; i ++) {
        char *path = paths[i];
        DIR *dirp = opendir(path);
        struct dirent *dp;
        if (dirp != NULL) {
            while ((dp = readdir(dirp)) != NULL) {
                // Only load files with the proper extension
                if (!strncmp(dp->d_name + dp->d_namlen - strlen(SO_EXT),
                             SO_EXT, strlen(SO_EXT))) {
                    // Make an absolute path
                    char *file_name = malloc(strlen(path) + dp->d_namlen + 1);
                    strcpy(file_name, path);
                    strcat(file_name, dp->d_name);
                    load_plugin(file_name);
                    free(file_name);
                }
            }
            closedir(dirp);
        }
    }
}


static char*
load_so_plugin(char *filename)
{
    void *plugin_handle = dlopen(filename, RTLD_NOW|RTLD_LOCAL);
    
    if (! plugin_handle) {
        return NULL;
    }
    
    init_f init_func = dlsym(plugin_handle, PLUGIN_INIT_SYMBOL);
    
    if (! init_func) {
        fprintf(stderr, "plugin %s does not supply " PLUGIN_INIT_SYMBOL "()\n",
                filename);
        dlclose(plugin_handle);
        return NULL;
    }
    
    plugin_t *plugin = init_func(0);
    plugin->dynlib_handle = plugin_handle; // this is a runtime param not set by the plugin itself
    
    /* insert plugin in our plugin registry */
    hashtable_insert(PLUGIN_dict, plugin->key, plugin);
    
    return plugin->key;
}

// Check what kind of plugin is available in the search path and load it
char*
load_plugin(char *filename)
{
    if (! filename) return NULL;
    
    // relative path? if so, append the SO_EXT to the file name and load in WD
    if (filename[0] != '/') {
        char *file = malloc(strlen(filename) + strlen(SO_EXT) + 1);
        strcpy(file, filename);
        strcat(file, SO_EXT);
        
        char *key = load_so_plugin(file);

        free(file);
        
        return key;
    }
    
    // this is an absolute path, we should load the file as is
    
    return load_so_plugin(filename);
        
    return NULL;
}

void
unload_plugin(char *key)
{
    plugin_t *plugin = hashtable_remove(PLUGIN_dict, key);
    
    if (! plugin) return;
    
    dlclose(plugin->dynlib_handle);
}


void
register_plugin_interface(char *interface_key, void *interface)
{
    hashtable_insert(PLUGIN_interface_dict, interface_key, interface);
}

void
remove_plugin_interface(char *interface_key)
{
    hashtable_remove(PLUGIN_interface_dict, interface_key);
}
