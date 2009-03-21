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
#include <assert.h>
#include <glob.h>


#include <gencds/hashtable.h>

#include "res-manager.h"
#include "plugin-handler.h"
#include "log.h"

#ifndef SO_EXT
#define SO_EXT ".so"
#endif /* ! SO_EXT */

static hashtable_t *gPLUGIN_interface_dict = NULL;
static hashtable_t *gPLUGIN_dict = NULL;

void
ooPluginInit(void)
{
    if (! (gPLUGIN_interface_dict = hashtable_new_with_str_keys(512)) ) {
      ooLogFatal("plugin: interface dictionary not created");
    }
    if (! (gPLUGIN_dict = hashtable_new_with_str_keys(512)) ) {
        hashtable_delete(gPLUGIN_interface_dict);
        ooLogFatal("plugin: plugin dictionary not created");
    }
}

/* read in all dynlibs in the plug-in directories */
void
ooPluginLoadAll(void)
{
  glob_t glob = ooResGetFilePaths("plugins/*" SO_EXT);

  ooLogInfo("%u plugins found with extension " SO_EXT, glob.gl_pathc);
  for (int i = 0 ; i < glob.gl_pathc ; i ++) {
    ooPluginLoad(glob.gl_pathv[i]);
  }

  globfree(&glob);
}


static char*
ooPluginLoadSO(char *filename)
{
    void *plugin_handle = dlopen(filename, RTLD_NOW|RTLD_LOCAL);
    
    if (! plugin_handle) {
        return NULL;
    }
    
    OOpluginversion *vers = dlsym(plugin_handle, "oopluginversion");
    
    init_f init_func = dlsym(plugin_handle, PLUGIN_INIT_SYMBOL);
    
    if (! init_func) {
        ooLogError("plugin %s does not supply " PLUGIN_INIT_SYMBOL "()",
                   filename);
        dlclose(plugin_handle);
        return NULL;
    }
    
    if (vers && *vers == OO_Plugin_Ver_1_00) {
      OOplugincontext_v1 ctxt = { NULL };
      OOplugin *plugin = init_func(&ctxt);
      plugin->dynlib_handle = plugin_handle; // this is a runtime param not set by the plugin itself
      /* insert plugin in our plugin registry */
      hashtable_insert(gPLUGIN_dict, plugin->key, plugin);
      ooLogInfo("plugin %s loaded", plugin->key);
      return plugin->key;
    } else {
      if (!vers) ooLogError("plugin %s does not specify oopluginversion", filename);
      else ooLogError("plugin %s incompatible (plugin v = %d, supported = %d)",
                      filename, *vers, OO_Plugin_Ver_1_00);
    }
    
    return NULL;
}

// Check what kind of plugin is available in the search path and load it
char*
ooPluginLoad(char *filename)
{
    if (! filename) return NULL;
    
    // relative path? if so, append the SO_EXT to the file name and load in WD
    if (filename[0] != '/') {
        char *file = malloc(strlen(filename) + strlen(SO_EXT) + 1);
        strcpy(file, filename);
        strcat(file, SO_EXT);
        
        char *key = ooPluginLoadSO(file);

        free(file);
        
        return key;
    }
    
    // this is an absolute path, we should load the file as is
    
    return ooPluginLoadSO(filename);
        
    return NULL;
}

void
unload_plugin(char *key)
{
    OOplugin *plugin = hashtable_remove(gPLUGIN_dict, key);
    
    OOpluginversion *vers = dlsym(plugin->dynlib_handle, "oopluginversion");
    
    if (! vers) ooLogFatal("pluginversion not found when unloading");
    switch (*vers) {
    case OO_Plugin_Ver_1_00:
      {
        void (*finalise_func)(void*) = dlsym(plugin->dynlib_handle,
                                             PLUGIN_FINALISE_SYMBOL);
        if (finalise_func == NULL) ooLogFatal("no finalisation function in plugin");
        finalise_func(NULL);// TODO: Pass in proper context
      }
    default:
      assert(0 && "invalid case");
    }

    
    dlclose(plugin->dynlib_handle);
}


void
register_plugin_interface(char *interface_key, void *interface)
{
    hashtable_insert(gPLUGIN_interface_dict, interface_key, interface);
}

void
remove_plugin_interface(char *interface_key)
{
    hashtable_remove(gPLUGIN_interface_dict, interface_key);
}

void
ooPluginPrintAll()
{
  // Prints info on all loaded plugins
  list_entry_t *entry = hashtable_first(gPLUGIN_dict);

  printf("=============== PLUGIN LIST ==============\n");
  printf("|KEY     |NAME            |REV|STATUS|VER|\n");
  printf("|--------|----------------|---|------|---|\n");
  
  while (entry) {
    OOplugin *plugin = (OOplugin*) hashtable_entry_data(entry);
    printf("|%-8s|%-16s|%3u|LOADED|%3d|\n",
           plugin->key, plugin->name, plugin->rev, plugin->vers);    
    
    entry = list_entry_next(entry);
  }

  printf("==========================================\n");

}
