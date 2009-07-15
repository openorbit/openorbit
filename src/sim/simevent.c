/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdlib.h>
#include "sim/simevent.h"

#define OO_EVENT_QUEUE_INIT_LEN 100

 uint64_t EventRank(OOevent *ev)
 {
   return ev->fireTime;
 }

 OOeventqueue*
 ooSimNewEventQueue(void)
 {
     OOeventqueue *queue = malloc(sizeof(OOeventqueue));
     queue->freeEvents = malloc(sizeof(OOevent) * OO_EVENT_QUEUE_INIT_LEN);
     queue->activeEventHeap = heap_new(8, (compute_rank_f)EventRank);

     for (int i = 0 ; i < OO_EVENT_QUEUE_INIT_LEN; i ++) {
         queue->freeEvents[i].next = &queue->freeEvents[i+1];
         queue->freeEvents[i].data = NULL;
         queue->freeEvents[i].handler = NULL;
         queue->freeEvents[i].fireTime = 0;
     }

     return queue;
 }

 int
 ooSimStackEvent(OOeventqueue *q, OOeventhandler handler, void *data)
 {

 }

 int
 ooSimJDEvent(OOeventqueue *q, double jd, OOeventhandler handler, void *data)
 {

 }


 int
 ooSimInsertEvent(OOeventqueue *q, int offset, OOeventhandler handler, void *data)
 {
     //OOevent *e = q->first;
     //int tsCnt = e->fireTime = offset;
     //while (tsCnt < offset) {
     //    e = e->next;
     //    tsCnt += e->fireTime;
     //}
 }

 int
 ooSimHandleNextEvent(OOeventqueue *q)
 {
     OOevent *ev = heap_peek(q->activeEventHeap);
     ev = heap_remove(q->activeEventHeap);

     return ev->fireTime;
 }
 