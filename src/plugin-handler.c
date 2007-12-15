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

static char*
load_scm_plugin(char *filename)
{
    return NULL;
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
        fprintf(stderr, "plugin does not supply " PLUGIN_INIT_SYMBOL "()\n");
        dlclose(plugin_handle);
        return NULL;
    }
    
    plugin_t *plugin = init_func();
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
    
    // Absolute path?
    if (filename[0] == '/') {
        char *file = malloc(strlen(filename) + strlen(SO_EXT) + 1);
        strcpy(file, filename);
        strcat(file, SO_EXT);
        
        char *key = load_so_plugin(file);

#if 0        
        // try to load as scm script if the previous load failed
        if (!key) {
            free(file);
            file = malloc(strlen(filename) + strlen(SCM_EXT) + 1);
            strcpy(file, filename);
            strcat(file, SCM_EXT);
            key = load_scm_plugin(filename);
        }
#endif        
        free(file);
        
        return key;
    }
    
    // Not absolute path, we search through the directories for so's and scm-files
    
    return load_so_plugin(filename);
    
    return load_scm_plugin(filename);
    
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
