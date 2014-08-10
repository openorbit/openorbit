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
#include <openorbit/log.h>
#include "common/palloc.h"


#ifndef SO_EXT
#define SO_EXT ".so"
#endif /* ! SO_EXT */

static hashtable_t *gPLUGIN_interface_dict = NULL;
static hashtable_t *gPLUGIN_dict = NULL;

OOplugincontext_v1 ctxt = { NULL };

void
plugin_init(void)
{
    if (! (gPLUGIN_interface_dict = hashtable_new_with_str_keys(512)) ) {
      log_fatal("plugin: interface dictionary not created");
    }
    if (! (gPLUGIN_dict = hashtable_new_with_str_keys(512)) ) {
        hashtable_delete(gPLUGIN_interface_dict);
        log_fatal("plugin: plugin dictionary not created");
    }

    //ctxt.objectManager = omCtxtNew();
}

/* read in all dynlibs in the plug-in directories */
void
plugin_load_all(void)
{
  glob_t glob = rsrc_get_file_paths("plugins/*.so");

  log_info("%u plugins found with extension " SO_EXT, glob.gl_pathc);
  for (int i = 0 ; i < glob.gl_pathc ; i ++) {
    plugin_load(glob.gl_pathv[i]);
  }

  globfree(&glob);
}


static char*
ooPluginLoadSO(char *filename)
{
    void *plugin_handle = dlopen(filename, RTLD_NOW|RTLD_LOCAL);

    if (! plugin_handle) {
      log_error("plugin load failed: %s", dlerror());
      return NULL;
    }

    OOpluginversion *vers = dlsym(plugin_handle, "oopluginversion");

    init_f init_func = dlsym(plugin_handle, PLUGIN_INIT_SYMBOL);

    if (! init_func) {
        log_error("plugin %s does not supply " PLUGIN_INIT_SYMBOL "()",
                   filename);
        dlclose(plugin_handle);
        return NULL;
    }

    if (vers && *vers == OO_Plugin_Ver_1_00) {
      OOplugin *plugin = init_func(&ctxt);
      plugin->dynlib_handle = plugin_handle; // this is a runtime param not set by the plugin itself
      /* insert plugin in our plugin registry */
      hashtable_insert(gPLUGIN_dict, plugin->key, plugin);
      log_info("plugin %s loaded", plugin->key);
      return plugin->key;
    } else {
      if (!vers) log_error("plugin %s does not specify oopluginversion", filename);
      else log_error("plugin %s incompatible (plugin v = %d, supported = %d)",
                      filename, *vers, OO_Plugin_Ver_1_00);
    }

    return NULL;
}

// Check what kind of plugin is available in the search path and load it
char*
plugin_load(char *filename)
{
    if (! filename) return NULL;

    // relative path? if so, append the SO_EXT to the file name and load in WD
    if (filename[0] != '/') {
        char *file = smalloc(strlen(filename) + strlen(SO_EXT) + 1);
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

    if (! vers) log_fatal("pluginversion not found when unloading");
    switch (*vers) {
    case OO_Plugin_Ver_1_00:
      {
        void (*finalise_func)(void*) = dlsym(plugin->dynlib_handle,
                                             PLUGIN_FINALISE_SYMBOL);
        if (finalise_func == NULL) log_fatal("no finalisation function in plugin");
        finalise_func(NULL);// TODO: Pass in proper context
      }
    default:
      assert(0 && "invalid case");
    }


    dlclose(plugin->dynlib_handle);
}


void
plugin_register_interface(char *interface_key, void *interface)
{
    hashtable_insert(gPLUGIN_interface_dict, interface_key, interface);
}

void
plugin_remove_interface(char *interface_key)
{
    hashtable_remove(gPLUGIN_interface_dict, interface_key);
}

void
plugin_print_all()
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
