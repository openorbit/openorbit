/*
  Copyright 2008 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>

typedef enum {
  LOG_TRACE = 0,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL,
  LOG_ABORT,
} log_level_t;

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

void log_set_level(log_level_t lev);
log_level_t log_get_lev_from_str(const char *str);
void log_trace(const char *msg, ...);
void log_info(const char *msg, ...);
void log_warn(const char *msg, ...);
void log_error(const char *msg, ...);
void log_fatal_if_null(const void *ptr, const char *msg, ...);
void log_fatal(const char *msg, ...) __attribute__((__noreturn__));
void log_abort(const char *msg, ...) __attribute__((__noreturn__));
void log_msg(log_level_t lev, const char *msg, ...);

#endif /* _LOG_H_ */
