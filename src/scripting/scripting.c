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


#include <stdarg.h>
#include <stdint.h>

#include "res-manager.h"
#include "log.h"

#include <Python.h>

extern void init_config(void);
extern void init_event(void);
#include "scripting.h"

// TODO: move to configure or something like that
#define OO_PATH_SEP ":"

void
ooScriptingInit(void)
{

  Py_InitializeEx(0); // note that ex(0) prevents python from stealing sighandlers

  init_config();
  init_event();


  // insert app-specific python path
  char *ooPyPath = ooResGetPath("python/");
  if (! ooPyPath) ooLogFatal("cannot generate python path");

  const char *pyPath = Py_GetPath();

  char *newOoPyPath;
  asprintf(&newOoPyPath, "%s" OO_PATH_SEP "%s", ooPyPath, pyPath);

  PySys_SetPath(newOoPyPath);

  free(ooPyPath);
  free(newOoPyPath);

//    ooScriptingRunInit();
}

void
ooScriptingFinalise(void)
{
  Py_Finalize();
}

void
ooScriptingRunInit(void)
{
  ooScriptingRunFile(SCR_INIT_SCRIPT_NAME);
}

bool
ooScriptingRunPostInit(void)
{
  return ooScriptingRunFile(SCR_POST_INIT_SCRIPT_NAME);
}

bool
ooScriptingRunFile(const char *fname)
{
  FILE *fp = ooResGetFile(fname);

  if (! fp) {
    ooLogWarn("could not open %s", fname);
    fprintf(stderr, "could not open %s\n", fname);
    return false;
  }
  if (PyRun_SimpleFile(fp, fname)) {
    fclose(fp);
    ooLogFatal("execution of %s failed", fname);
    //return false;
  }

  fclose(fp);
  return true;
}
