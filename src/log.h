/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>

typedef enum {
  OOLog_Trace = 0,
  OOLog_Info,
  OOLog_Warn,
  OOLog_Error,
  OOLog_Fatal,
  OOLog_Abort,
} OOloglev;

typedef enum {
  OOErr_None = 0,
  OOErr_Script,
  OOErr_FileIO,
  OOErr_FileOpen,
  OOErr_Mem,
  OOErr_InvalidPar,
  OOErr_NetIO,
  OOErr_NetAddr
} OOerr;

void ooLogInit(FILE *logPath);
void ooLogSetLevel(OOloglev lev);
OOloglev ooLogGetLevFromStr(const char *str);
void ooLogTrace(const char *msg, ...);
void ooLogInfo(const char *msg, ...);
void ooLogWarn(const char *msg, ...);
void ooLogError(const char *msg, ...);
void ooLogFatalIfNull(const void *ptr, const char *msg, ...);
void ooLogFatal(const char *msg, ...) __attribute__((__noreturn__));
void ooLogAbort(const char *msg, ...) __attribute__((__noreturn__));
void ooLogMsg(OOloglev lev, const char *msg, ...);

#endif /* _LOG_H_ */
