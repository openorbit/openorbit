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


#ifndef __PLUGIN_HANDLER_H__
#define __PLUGIN_HANDLER_H__
#ifdef __cplusplus
extern "C" {
#endif 

#include <stdbool.h>

#include "libgencds/object-manager.h"

// All OSes do not support resolving backlinks, so we export an interface with
// pointers to all the functions in the API
typedef struct {
    char *name; //!< Proper name of the plugin (e.g. "Space Shuttle")
    unsigned rev; //!< Revision number, greater is later
    char *key; //!< Hashkey (e.g. "SPACE_SHTL")
    char *description; //!< Long string describing the plugin, e.g. "All shuttle components"
    void *dynlib_handle; //!< Admin data, filled in automatically, do not touch
} plugin_t;

typedef plugin_t* (*init_f)(om_ctxt_t*);
#define PLUGIN_INIT_SYMBOL "plugin_init"


bool init_plugin_manager(void);

/*!
    Loads a plugin
    
    The function can be used to load an open orbit plugin. There exist a number
    of different plugins such as vessels, engines and instruments. The
    recommended layout of the plugin-directory is that it should contain the
    directories: vessels, instruments, actuators
    \param filename The name of the file that shall be loaded. Per default the
        following paths are searched in this order:
        ~/.openorbit/plugins/, $PREFIX/share/openorbit/plugins on a regular
        UNIX-like system. On the Mac OS X, the following paths are searched as
        well after the previous mentioned paths:
        "~/Library/Application Support/Open Orbit/Plugins",
        "/Library/Application Support/Open Orbit/Plugins" and
        "/Network/Library/Application Support/Open Orbit/Plugins".
        
        The filename shall be supplied without its extension, the extention will
        be determined from the platform and added automatically (this is .so on
        Linux and .dylib on MacOS X or .scm for Scheme plugins).
    \return A zero terminated c-string containing the identifier of the plugin.
*/

bool load_plugins(void);
char *load_plugin(char *filename);
void unload_plugin(char *key);

void register_plugin_interface(char *interface_key, void *interface);
void remove_plugin_interface(char *interface_key);

#ifdef __cplusplus
}
#endif 

#endif /* ! __PLUGIN_HANDLER_H__ */
