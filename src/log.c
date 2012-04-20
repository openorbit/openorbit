/*
  Copyright 2008,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sysexits.h>
#include "common/moduleinit.h"
#include <openorbit/log.h>

static FILE *sLogFile;
static OOloglev sLogLev = OOLog_Info;
static char* sLogNames[] = {"trace", "info", "warning", "error", "fatal", "abort"};

OOloglev ooLogGetLevFromStr(const char *str)
{
  for (int i = 0 ; i < sizeof(sLogNames)/sizeof(char*) ; ++ i) {
    if (!strcmp(sLogNames[i], str)) {
      return (OOloglev)i;
    }
  }

  ooLogError("loglev from string \"%s\" not recognised", str);
  return OOLog_Info;
}

static void
ooLogWriteV(OOloglev lev, const char *msg, va_list vaList)
{
  if (!sLogFile) sLogFile = stderr;
  if (lev >= sLogLev) {
    fprintf(sLogFile, "oo: %s: ", sLogNames[lev]);
    vfprintf(sLogFile, msg, vaList);
    fprintf(sLogFile, "\n");
  }
}

void
ooLogSetLevel(OOloglev lev)
{
  sLogLev = lev;
}


void
ooLogTrace(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Trace, msg, vaList);
  va_end(vaList);
}


void
ooLogInfo(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Info, msg, vaList);
  va_end(vaList);
}


void
ooLogWarn(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Warn, msg, vaList);
  va_end(vaList);
}


void
ooLogError(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Error, msg, vaList);
  va_end(vaList);
}

void
ooLogFatalIfNull(const void *ptr, const char *msg, ...)
{
  if (ptr == NULL) {
    va_list vaList;
    va_start(vaList, msg);
    ooLogWriteV(OOLog_Fatal, msg, vaList);
    va_end(vaList);
    exit(EX_SOFTWARE);
  }
}

void
ooLogFatal(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Fatal, msg, vaList);
  va_end(vaList);
  exit(EX_SOFTWARE);
}

void
ooLogAbort(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(OOLog_Fatal, msg, vaList);
  va_end(vaList);
  abort();
}


void
ooLogMsg(OOloglev lev, const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  ooLogWriteV(lev, msg, vaList);
  va_end(vaList);
  if (lev == OOLog_Fatal) exit(EX_SOFTWARE);
  if (lev == OOLog_Abort) abort();
}
