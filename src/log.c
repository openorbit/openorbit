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
    created by the Initial Developer are Copyright (C) 2008 the
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sysexits.h>

#include "log.h"

static FILE *sLogFile;
static OOloglev sLogLev;
static char* sLogNames[] = {"trace", "info", "warning", "error", "fatal"};

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
    if (lev >= sLogLev) {
        fprintf(sLogFile, "oo: %s: ", sLogNames[lev]);
        vfprintf(sLogFile, msg, vaList);
        fprintf(sLogFile, "\n");
    }
}

void
ooLogInit(FILE *logFile)
{
    sLogFile = logFile;
    sLogLev = OOLog_Info;
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
ooLogMsg(OOloglev lev, const char *msg, ...)
{
    va_list vaList;
    va_start(vaList, msg);
    ooLogWriteV(lev, msg, vaList);
    va_end(vaList);
    if (lev = OOLog_Fatal) exit(EX_SOFTWARE);
}
