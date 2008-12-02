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

/*!
 * \file scripting.h
 * \brief Entry points to bring the scripting system up an running.
 * 
 * This header and the scripting functions provide a script language
 * independent scripting interface (to some extent). The only currently
 * supported language is Python; the early development of Open Orbit used
 * Scheme (through Guile), so there are remains of this system around (and it
 * might be interesting to revive it in the future).
 * */

#ifndef SCRIPTING_H_
#define SCRIPTING_H_
#ifdef __cplusplus
extern "C" {
#endif 

#include <stdbool.h>
#include "error.h"

#ifdef PYTHON_FWORK
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#define SCR_INIT_SCRIPT_NAME "script/init.py"
#define SCR_POST_INIT_SCRIPT_NAME "script/postinit.py"


/*!
 * \brief Initialises the scripting system.
 * 
 * Brings any internal scripting modules online. E.g. it registers C-functions
 * whith the scripting subsystem.
 * */
void ooScriptingInit(void);
bool ooScriptingRunPostInit(void);
/*!
 * \brief Cleans up and shuts down the scripting system.
 * 
 * Call this at exit.
 * */
void ooScriptingFinalise(void);

/*!
 * \brief Loads and executes the init script.
 * 
 * The init script is executed before the graphics system is enabled and is
 * responsible for setting up items such as video modes, controls and so forth.
 * */
void ooScriptingRunInit(void);

bool ooScriptingRunFile(const char *fname);

#ifdef __cplusplus
}
#endif 

#endif /*SCRIPTING_H_*/
