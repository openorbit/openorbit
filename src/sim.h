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
#include "SDL.h"

typedef enum {
	SIM_io_event,
	SIM_time_event
} sim_event_kind_t;

    
typedef void (*sim_event_handler_t)(sim_event_kind_t e, void *data);
    
typedef struct {
    sim_event_kind_t kind;
    sim_event_handler_t handler;
    void *data;
} sim_event_t;
    
    
typedef struct {
    uint64_t time;
    uint16_t step_size;
//    ph_world_t *world;
    // 
} sim_state_t;
    
/* Simulator SDL events */
#define SIM_STEP_EVENT 0
#define SIM_DEBUG_EVENT 1
    
// 25Hz
#define SIM_STEP_PERIOD 40

Uint32 sim_step_event(Uint32 interval, void *param);
void sim_step(float dt);

#ifdef __cplusplus
}
#endif

#endif /* ! SIM_H__ */
