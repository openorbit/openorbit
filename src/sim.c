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

SIMstate gSIM_state;

#define OO_EVENT_QUEUE_INIT_LEN 100
OOeventqueue*
ooSimNewEventQueue(void)
{
    OOeventqueue *queue = malloc(sizeof(OOeventqueue));
    queue->freeEvents = malloc(sizeof(OOevent) * OO_EVENT_QUEUE_INIT_LEN);
    
    for (int i = 0 ; i < OO_EVENT_QUEUE_INIT_LEN; i ++) {
        queue->freeEvents[i].next = &queue->freeEvents[i+1];
        queue->freeEvents[i].data = NULL;
        queue->freeEvents[i].handler = NULL;
        queue->freeEvents[i].fireOffset = 0;
    }
    
    return queue;
}

int
ooSimInsertEvent(OOeventqueue *q, int offset, OOeventhandler handler, void *data)
{
    OOevent *e = q->first;
    int tsCnt = e->fireOffset;
    while (tsCnt < offset) {
        e = e->next;
        tsCnt += e->fireOffset;
    }
}

int
ooSimHandleNextEvent(OOeventqueue *q)
{
    if (q->first && (-- q->first->fireOffset <= 0)) {
        OOevent *ev = q->first;
        q->first = q->first->next;
        ev->handler(ev->data);
        
        ev->next = q->freeEvents;
        q->freeEvents = ev;
        
        return 0;
    }
    
    if (q->first == NULL) return -1;
    return q->first->fireOffset;
}


void
ooSimSetSg(OOnode *sg)
{
    gSIM_state.sg = sg;
}
void
ooSimSetOrbSys(orb_sys_t *osys)
{
    gSIM_state.orbSys = osys;
}


#include "physics/orbit.h"
#include <sys/time.h>
void
ooSimStep(float dt)
{
    gSIM_state.timeStamp ++;
    // temporary to have something to look at...
    for (int i = 0 ; i < MAX_PLANETS ; i ++) {
        planet_rot_orig(&gPlanets[i], dt*0.1);
        planet_rot_ax(&gPlanets[i], dt*0.5);
    }
    
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);
    
    extern orb_sys_t *gOrb_root_system;
    orbit_clear(gOrb_root_system);
    orbit_step(gOrb_root_system, dt);
    
    gettimeofday(&end, NULL);
    
    printf("\tsimstep time: %lu\n",  (end.tv_usec - start.tv_usec) % 1000000L);
    // compute physics step
//    ph_step(gSIM_state.world, dt);
    
    // do system simulation
    // sys_step(gSIM_state->systems, dt);
    
    // check event queue
    // sim_event_check(gSIM_state->events, );
}
