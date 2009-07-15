/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __PLUGIN_HANDLER_H__
#define __PLUGIN_HANDLER_H__
#ifdef __cplusplus
extern "C" {
#endif 

#include <stdbool.h>

#include <openorbit/plugin.h>

void ooPluginInit(void);

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

void ooPluginLoadAll(void);
char *ooPluginLoad(char *filename);
void ooPluginUnload(char *key);

void ooPluginRegisterInterface(char *interface_key, void *interface);
void ooPluginRemoveInterface(char *interface_key);

void ooPluginPrintAll(void);


#ifdef __cplusplus
}
#endif 

#endif /* ! __PLUGIN_HANDLER_H__ */
