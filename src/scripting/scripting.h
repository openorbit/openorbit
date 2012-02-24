/*
  Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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
