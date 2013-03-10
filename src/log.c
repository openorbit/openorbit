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
static log_level_t sLogLev = LOG_INFO;
static char* sLogNames[] = {"trace", "info", "warning", "error", "fatal", "abort"};

log_level_t log_get_lev_from_str(const char *str)
{
  for (int i = 0 ; i < sizeof(sLogNames)/sizeof(char*) ; ++ i) {
    if (!strcmp(sLogNames[i], str)) {
      return (log_level_t)i;
    }
  }

  log_error("loglev from string \"%s\" not recognised", str);
  return LOG_INFO;
}

static void
log_write_v(log_level_t lev, const char *msg, va_list vaList)
{
  if (!sLogFile) sLogFile = stderr;
  if (lev >= sLogLev) {
    fprintf(sLogFile, "oo: %s: ", sLogNames[lev]);
    vfprintf(sLogFile, msg, vaList);
    fprintf(sLogFile, "\n");
  }
}

void
log_set_level(log_level_t lev)
{
  sLogLev = lev;
}


void
log_trace(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_TRACE, msg, vaList);
  va_end(vaList);
}


void
log_info(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_INFO, msg, vaList);
  va_end(vaList);
}


void
log_warn(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_WARN, msg, vaList);
  va_end(vaList);
}


void
log_error(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_ERROR, msg, vaList);
  va_end(vaList);
}

void
log_fatal_if_null(const void *ptr, const char *msg, ...)
{
  if (ptr == NULL) {
    va_list vaList;
    va_start(vaList, msg);
    log_write_v(LOG_FATAL, msg, vaList);
    va_end(vaList);
    exit(EX_SOFTWARE);
  }
}

void
log_fatal(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_FATAL, msg, vaList);
  va_end(vaList);
  exit(EX_SOFTWARE);
}

void
log_abort(const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(LOG_FATAL, msg, vaList);
  va_end(vaList);
  abort();
}


void
log_msg(log_level_t lev, const char *msg, ...)
{
  va_list vaList;
  va_start(vaList, msg);
  log_write_v(lev, msg, vaList);
  va_end(vaList);
  if (lev == LOG_FATAL) exit(EX_SOFTWARE);
  if (lev == LOG_ABORT) abort();
}
