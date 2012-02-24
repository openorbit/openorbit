/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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
 
#ifndef PLUGIN_H_ZLG8MCO9
#define PLUGIN_H_ZLG8MCO9

#include <gencds/object-manager2.h>

// Note tat in order to maintain backward compatibility, this structure may
// not change, thus, it is versioned for future extensions.
typedef struct {
  OMcontext *objectManager;
} OOplugincontext_v1;

typedef enum {
  OO_Plugin_Ver_1_00 = 0
} OOpluginversion; // Exported by each plugin in the ooplugversion symbol

// All OSes do not support resolving backlinks, so we export an interface with
// pointers to all the functions in the API
typedef struct {
    char *name; //!< Proper name of the plugin (e.g. "Space Shuttle")
    unsigned rev; //!< Revision number, greater is later
    char *key; //!< Hashkey (e.g. "SPACE_SHTL")
    char *description; //!< Long string describing the plugin, e.g. "All shuttle components"
    void *dynlib_handle; //!< Admin data, filled in automatically, do not touch
    OOpluginversion vers;
} OOplugin;

typedef OOplugin* (*init_f)(OOplugincontext_v1 *);

#define PLUGIN_INIT_SYMBOL "ooplugininit"
#define PLUGIN_FINALISE_SYMBOL "oopluginfinalise"



#endif /* end of include guard: PLUGIN_H_ZLG8MCO9 */
