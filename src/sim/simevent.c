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
 