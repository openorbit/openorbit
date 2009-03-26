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
    created by the Initial Developer are Copyright (C) 2009 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)openorbit.org>.

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
