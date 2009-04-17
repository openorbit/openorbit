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
 
#ifndef LOG_H_
#define LOG_H_
#include <stdio.h>

typedef enum {
    OOLog_Trace = 0,
    OOLog_Info,
    OOLog_Warn,
    OOLog_Error,
    OOLog_Fatal
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
void ooLogMsg(OOloglev lev, const char *msg, ...);

#endif /* _LOG_H_ */
