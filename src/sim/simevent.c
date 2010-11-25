/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include "common/moduleinit.h"
#include "sim/simevent.h"
#include "sim/simtime.h"
#include "log.h"

#define OO_EVENT_QUEUE_INIT_LEN 100

static OOeventqueue *gQueue;

INIT_PRIMARY_MODULE
{
  gQueue = simNewEventQueue();
}

// BUG: If working with events with fire time < 1970 as event time then is negative
int64_t EventRank(OOevent *ev)
{
  return ev->fireTime;
}

OOeventqueue*
simNewEventQueue(void)
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

OOevent*
simAllocEvent(void)
{
  if (gQueue->freeEvents == NULL) {
    gQueue->freeEvents = malloc(sizeof(OOevent) * OO_EVENT_QUEUE_INIT_LEN);
    if (gQueue->freeEvents == NULL) {
      ooLogFatal("out of memory %s:%d", __FILE__, __LINE__);
    }
    for (int i = 0 ; i < OO_EVENT_QUEUE_INIT_LEN; i ++) {
      gQueue->freeEvents[i].next = &gQueue->freeEvents[i+1];
      gQueue->freeEvents[i].data = NULL;
      gQueue->freeEvents[i].handler = NULL;
      gQueue->freeEvents[i].fireTime = 0;
    }
  }

  OOevent * ev = gQueue->freeEvents;
  gQueue->freeEvents = ev->next;
  ev->next = NULL;
  return ev;
}

void
simReleaseEvent(OOevent *ev)
{
  ev->next = gQueue->freeEvents;
  gQueue->freeEvents = ev;
}

void
simInsertEvent(OOevent *ev)
{
  heap_insert(gQueue->activeEventHeap, ev);
}

void
simStackEvent(OOeventhandler handler, void *data)
{
  OOevent *ev = simAllocEvent();
  ev->fireTime = simTimeGetTimeStamp();
  ev->handler = handler;
  ev->data = data;
  simInsertEvent(ev);
}

void
simEnqueueAbsoluteEvent(double jd, OOeventhandler handler, void *data)
{
  double currentJD = simTimeGetTime();
  if (jd < currentJD) {
    // cannot enqueue events in the future
    ooLogError("past events cannot be inserted in queue, current JD = %f, event JD = %f",
               currentJD, jd);
  }

  OOevent *ev = simAllocEvent();
  ev->fireTime = simTimeJDToTimeStamp(jd);
  ev->handler = handler;
  ev->data = data;

  simInsertEvent(ev);
}


void
simEnqueueDelta_ms(unsigned offset, OOeventhandler handler, void *data)
{
  OOevent *ev = simAllocEvent();
  ev->fireTime = simTimeGetTimeStamp() + offset;
  ev->handler = handler;
  ev->data = data;

  simInsertEvent(ev);
}

void
simEnqueueDelta_s(double offset, OOeventhandler handler, void *data)
{
  OOevent *ev = simAllocEvent();
  ev->fireTime = simTimeGetTimeStamp() + offset*1000.0;
  ev->handler = handler;
  ev->data = data;

  simInsertEvent(ev);
}


void
simDispatchPendingEvents()
{
  OOevent *ev = heap_peek(gQueue->activeEventHeap);
  while (ev && ev->fireTime < simTimeGetTimeStamp()) {
    heap_remove(gQueue->activeEventHeap);
    ev->handler(ev->data);
    simReleaseEvent(ev);
    ev = heap_peek(gQueue->activeEventHeap);
  }
}
