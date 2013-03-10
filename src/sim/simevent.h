/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef SIMEVENT_H_KHYQLKNG
#define SIMEVENT_H_KHYQLKNG

#include <gencds/heap.h>
#include <gencds/list.h>

/*!
  Event handler function
 */
 typedef void (*sim_event_handler_fn_t)(void *data);

/*!
  Internal event structure. The structure is not for direct access.

  \todo Move to C-file if possible
 */
typedef struct _sim_event_t {
  int64_t fireTime;
  sim_event_handler_fn_t handler;
//  void (^handler_block)(void)
  void *data;
  struct _sim_event_t *next;
} sim_event_t;


/*!
  Event queue structure

  An event queue works as follows: The programmer ask for either a timed event
  which may be relative to the current simtime or absolute, it may also be at
  time = now, meaning it will be stack posted.

  There are two event queues running parallel. One is a heap of events, that all
  have an absolute time for triggering. Thus, inserting events in this queue is
  log(n) in time, and extracting them is also log(n) in time.

  The second queue is the event stack. This stack is for use when an event is
  posted by a function in order to get a callback from a different calling
  context. The stack posted events are guaranteed to be dispatched before the
  next physics simulation step. Inserting and removing these events are O(1)
  in complexity.

  In order to speed things up, we allocate a pool of event structures and place
  these in a free event pool. Whenever an event is inserted, a structure is
  taken from the free event pool and inserted either on the event stack or in
  the event heap. Whenever an event is popped, the structure is moved back into
  the free event pool.
*/
 typedef struct {
   heap_t *activeEventHeap;
   sim_event_t *freeEvents;
 } sim_event_queue_t;

sim_event_queue_t* sim_new_event_queue(void);

void sim_event_stackpost(sim_event_handler_fn_t handler, void *data);
void sim_event_enqueue_absolute(double jd, sim_event_handler_fn_t handler, void *data);
void sim_event_enqueue_relative_ms(unsigned offset, sim_event_handler_fn_t handler, void *data);
void sim_event_enqueue_relative_s(double offset, sim_event_handler_fn_t handler, void *data);
void sim_event_enqueue_relative_s_wct(double offset, sim_event_handler_fn_t handler, void *data);

void sim_event_dispatch_pending(void);

#endif /* end of include guard: SIMEVENT_H_KHYQLKNG */
