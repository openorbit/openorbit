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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "sim.h"
#include "physics/dynamics.h"

#include "rendering/planet.h"
// epoch = ???

sim_state_t gSIM_state;

Uint32
sim_step_event(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_UserEvent userevent;
    
    /* In this example, our callback pushes an SDL_USEREVENT event
        into the queue, and causes ourself to be called again at the
        same interval: */
    
    userevent.type = SDL_USEREVENT;
    userevent.code = SIM_STEP_EVENT;
    userevent.data1 = NULL;
    userevent.data2 = NULL;
    
    event.type = SDL_USEREVENT;
    event.user = userevent;
    
    SDL_PushEvent(&event);
    return interval;
}


void
sim_step(float dt)
{
    gSIM_state.time ++;
    // temporary to have something to look at...
    for (int i = 0 ; i < MAX_PLANETS ; i ++) {
        planet_rot_orig(&gPlanets[i], dt*0.1);
        planet_rot_ax(&gPlanets[i], dt*0.5);
    }
    
    // compute physics step
//    ph_step(gSIM_state.world, dt);
    
    // do system simulation
    // sys_step(gSIM_state->systems, dt);
    
    // check event queue
    // sim_event_check(gSIM_state->events, );
}
