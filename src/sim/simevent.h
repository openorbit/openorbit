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
 created by the Initial Developer are Copyright (C) 2009 the
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
 provisions above and replace them with the notice and other provisions
 required by the GPL or the LGPL. If you do not delete the provisions
 above, a recipient may use your version of this file under either the MPL,
 the GPL or the LGPL."
 */

#ifndef SIMEVENT_H_KHYQLKNG
#define SIMEVENT_H_KHYQLKNG

#include <gencds/heap.h>


/*!
  Event handler function
 */
 typedef void (*OOeventhandler)(void *data);

/*!
  Internal event structure. The structure is not for direct access.

  \todo Move to C-file if possible
 */
 typedef struct _OOevent {
     uint64_t fireTime;
     OOeventhandler handler;
     void *data;
     struct _OOevent *next;
 } OOevent;


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
   OOevent *freeEvents;
 } OOeventqueue;

 OOeventqueue* ooSimNewEventQueue(void);
 int ooSimInsertEvent(OOeventqueue *q, int offset, OOeventhandler handler, void *data);
 int ooSimHandleNextEvent(OOeventqueue *q);

#endif /* end of include guard: SIMEVENT_H_KHYQLKNG */
