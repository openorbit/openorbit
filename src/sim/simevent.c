/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

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


#include <stdlib.h>
#include "common/moduleinit.h"
#include "common/palloc.h"

#include "sim/simevent.h"
#include "sim/simtime.h"
#include <openorbit/log.h>
#include "palloc.h"

#define OO_EVENT_QUEUE_INIT_LEN 100

struct handler_param {
  sim_event_handler_fn_t handler;
  void *data;
};

static sim_event_queue_t *gQueue;
static pool_t *gTimerParamPool;

MODULE_INIT(simevent, NULL)
{
  log_trace("initialising 'simevent' module");
  gQueue = sim_new_event_queue();
  gTimerParamPool = pool_create(sizeof(struct handler_param));
}

// BUG: If working with events with fire time < 1970 as event time then is negative
int64_t EventRank(sim_event_t *ev)
{
  return ev->fireTime;
}

sim_event_queue_t*
sim_new_event_queue(void)
{
  sim_event_queue_t *queue = smalloc(sizeof(sim_event_queue_t));
  queue->freeEvents = smalloc(sizeof(sim_event_t) * OO_EVENT_QUEUE_INIT_LEN);
  queue->activeEventHeap = heap_new(8, (compute_rank_f)EventRank);

  for (int i = 0 ; i < OO_EVENT_QUEUE_INIT_LEN; i ++) {
    queue->freeEvents[i].next = &queue->freeEvents[i+1];
    queue->freeEvents[i].data = NULL;
    queue->freeEvents[i].handler = NULL;
    queue->freeEvents[i].fireTime = 0;
  }

  return queue;
}

sim_event_t*
sim_event_alloc(void)
{
  if (gQueue->freeEvents == NULL) {
    gQueue->freeEvents = smalloc(sizeof(sim_event_t) * OO_EVENT_QUEUE_INIT_LEN);
    if (gQueue->freeEvents == NULL) {
      log_fatal("out of memory %s:%d", __FILE__, __LINE__);
    }
    for (int i = 0 ; i < OO_EVENT_QUEUE_INIT_LEN; i ++) {
      gQueue->freeEvents[i].next = &gQueue->freeEvents[i+1];
      gQueue->freeEvents[i].data = NULL;
      gQueue->freeEvents[i].handler = NULL;
      gQueue->freeEvents[i].fireTime = 0;
    }
  }

  sim_event_t * ev = gQueue->freeEvents;
  gQueue->freeEvents = ev->next;
  ev->next = NULL;
  return ev;
}

void
sim_event_release(sim_event_t *ev)
{
  ev->next = gQueue->freeEvents;
  gQueue->freeEvents = ev;
}

void
sim_event_insert(sim_event_t *ev)
{
  heap_insert(gQueue->activeEventHeap, ev);
}

void
sim_event_stackpost(sim_event_handler_fn_t handler, void *data)
{
  sim_event_t *ev = sim_event_alloc();
  ev->fireTime = sim_time_get_time_stamp();
  ev->handler = handler;
  ev->data = data;
  sim_event_insert(ev);
}

void
sim_event_enqueue_absolute(double jd, sim_event_handler_fn_t handler, void *data)
{
  double currentJD = sim_time_get_time();
  if (jd < currentJD) {
    // cannot enqueue events in the future
    log_error("past events cannot be inserted in queue, current JD = %f, event JD = %f",
               currentJD, jd);
  }

  sim_event_t *ev = sim_event_alloc();
  ev->fireTime = sim_time_jd_to_time_stamp(jd);
  ev->handler = handler;
  ev->data = data;

  sim_event_insert(ev);
}


void
sim_event_enqueue_relative_ms(unsigned offset, sim_event_handler_fn_t handler, void *data)
{
  sim_event_t *ev = sim_event_alloc();
  ev->fireTime = sim_time_get_time_stamp() + offset;
  ev->handler = handler;
  ev->data = data;

  sim_event_insert(ev);
}

void
sim_event_enqueue_relative_s(double offset, sim_event_handler_fn_t handler, void *data)
{
  sim_event_t *ev = sim_event_alloc();
  ev->fireTime = sim_time_get_time_stamp() + offset*1000.0;
  ev->handler = handler;
  ev->data = data;

  sim_event_insert(ev);
}

// This is a rather messy thing. We want to be able to enqueue events on a fixed
// offset from the current time.
// In the future this needs to be fixed since the user should be able to change
// the sim frequency.

// FIXME: Must be synced with main.c
//#define SIM_WCT_TIMER 2
//#include <SDL/SDL.h>

// TODO: Factor out sdl stuff
#if 0
static uint32_t
sim_timer_event(uint32_t interval, void *param)
{
  struct handler_param *hp = param;
  SDL_Event event;
  SDL_UserEvent userevent;

  userevent.type = SDL_USEREVENT;
  userevent.code = SIM_WCT_TIMER;
  userevent.data1 = hp->handler;
  userevent.data2 = hp->data;

  event.type = SDL_USEREVENT;
  event.user = userevent;

  SDL_PushEvent(&event);
  pool_free(param);
  return 0;
}

void
sim_event_enqueue_relative_s_wct(double offset, sim_event_handler_fn_t handler, void *data)
{
  struct handler_param *param = pool_alloc(gTimerParamPool);
  param->handler = handler;
  param->data = data;
  SDL_AddTimer((Uint32)(offset*1000.0), sim_timer_event, param);
}
#endif

void
sim_event_dispatch_pending()
{
  sim_event_t *ev = heap_peek(gQueue->activeEventHeap);
  while (ev && ev->fireTime < sim_time_get_time_stamp()) {
    heap_remove(gQueue->activeEventHeap);
    ev->handler(ev->data);
    sim_event_release(ev);
    ev = heap_peek(gQueue->activeEventHeap);
  }
}
