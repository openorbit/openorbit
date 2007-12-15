/* Open Orbit - An interactive spacecraft simulator.
 * Copyright (C) 2006  Mattias Holm (mattias.holm@contra.nu)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

#ifdef WITH_GUILE
#include <libguile.h>
    typedef SCM scr_f;
    typedef SCM scr_bool;
    typedef SCM scr_int;
    typedef SCM scr_str;
    typedef SCM scr_float;
    #define SCR_INIT_SCRIPT_NAME "init.scm"
#elif defined(WITH_PYTHON) /* WITH_GUILE */
#include <Python.h>
    typedef PyObject* scr_func_t;
    typedef PyObject* scr_bool_t;
    typedef PyObject* scr_int_t;
    typedef PyObject* scr_str_t;
    typedef PyObject* scr_float_t;
    #define SCR_INIT_SCRIPT_NAME "init.py"
#else /* WITH_PYTHON */
    #error "Need to have either Guile or Python enabled"
#endif /* ! WITH_PYTHON */

/*!
 * \brief Initialises the scripting system.
 * 
 * Brings any internal scripting modules online. E.g. it registers C-functions
 * whith the scripting subsystem.
 * */
bool init_scripting(void);

/*!
 * \brief Cleans up and shuts down the scripting system.
 * 
 * Call this at exit.
 * */
void finilise_scripting(void);

/*!
 * \brief Loads and executes the init script.
 * 
 * The init script is executed before the graphics system is enabled and is
 * responsible for setting up items such as video modes, controls and so forth.
 * */
oo_error_t load_setup_script(void);


#ifdef __cplusplus
}
#endif 

#endif /*SCRIPTING_H_*/
