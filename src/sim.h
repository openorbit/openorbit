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
    created by the Initial Developer are Copyright (C) 2006 the
    Initial Developer. All Rights Reserved.

    Contributor(s):
        Mattias Holm <mattias.holm(at)contra.nu>.

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

 
#ifndef SIM_H__
#define SIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "physics/orbit.h"
#include "rendering/scenegraph.h"
#include "sim/spacecraft.h"

    
typedef void (*OOeventhandler)(void *data);
    
typedef struct _OOevent {
    signed fireOffset;
    OOeventhandler handler;
    void *data;
    struct _OOevent *next;
} OOevent;

typedef struct {
    OOevent *first;
    OOevent *freeEvents;
} OOeventqueue;

OOeventqueue* ooSimNewEventQueue(void);
int ooSimInsertEvent(OOeventqueue *q, int offset, OOeventhandler handler, void *data);
int ooSimHandleNextEvent(OOeventqueue *q);
    
typedef struct {  
  double currentTime; //!< Current time in earth days relative to epoch
  uint64_t timeStamp; //!< Time stamp (ticking up one every step)
  float stepSize;     //!< Step size for simulation in seconds
  OOspacecraft *currentSc; //!< Current active spacecraft
  OOorbsys *orbSys;   //!< Root orbit system, this will be the sun initially
  //OOcam *cam;         //!< Current camera
  OOscenegraph *sg;   //!< Scenegraph of the world
} SIMstate;

void ooSimSetSg(OOscenegraph *sg);

void ooSimSetOrbSys(OOorbsys *osys);

void ooSimStep(float dt);

#ifdef __cplusplus
}
#endif

#endif /* ! SIM_H__ */
