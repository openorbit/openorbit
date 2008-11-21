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
    created by the Initial Developer are Copyright (C) 2006,2008 the
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


#ifndef __IO_MANAGER_H__
#define __IO_MANAGER_H__
#ifdef __cplusplus
extern "C" {
#endif
    

#include <stdbool.h>
#include <stdint.h>
#include <Python.h>
#include "SDL.h"


/* These use the same values as SDL, so for porting (to not use SDL) they must be
   translated in some way */
#define OO_IO_MOD_NONE   0x0000
#define OO_IO_MOD_LSHIFT 0x0001
#define OO_IO_MOD_RSHIFT 0x0002
#define OO_IO_MOD_LCTRL  0x0040
#define OO_IO_MOD_RCTRL  0x0080
#define OO_IO_MOD_LALT   0x0100
#define OO_IO_MOD_RALT   0x0200
#define OO_IO_MOD_LMETA  0x0400
#define OO_IO_MOD_RMETA  0x0800
#define OO_IO_MOD_NUM    0x1000
#define OO_IO_MOD_CAPS   0x2000
#define OO_IO_MOD_MODE   0x4000

/*! Initialises map between SDL key / button IDs and the internal hashtable keys

    \pre None
*/
void ooIoInitSdlStringMap(void);
typedef void (*OObuttonhandlerfunc)(bool buttonUp, void *data);
/*! Register key handler C function */
void ooIoRegCKeyHandler(const char *name, OObuttonhandlerfunc handlerFunc);
/*! Register key handler Python function */
void ooIoRegPyKeyHandler(const char *name, PyObject *handlerFunc);

/*! Bind keyboard with keyName to the keyAction key

    \param keyName String key for the keyboard button
    \param keyAction Key corresponding to the registered action
    \param up 0 if the handler should fire on button down, otherwise it will fire on
        button up.
    \param mask ored mask from the OO_IO_MOD* constants 
*/
void ooIoBindKeyHandler(const char *keyName, const char *keyAction, int up,
                        uint16_t mask);


void ooIoDispatchKeyUp(const char *name, uint16_t mask);
void ooIoDispatchKeyDown(const char *name, uint16_t mask);

const char *ooIoSdlKeyNameLookup(SDLKey keyId);
const char *ooIoSdlMouseButtonNameLookup(unsigned buttonId);

#ifdef __cplusplus
}
#endif
    
#endif /* ! __IO_MANAGER_H */
