/*
  Copyright 2009,2010,2013 Mattias Holm <lorrden(at)openorbit.org>

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
#ifndef SIMTIME_H_STPF70AM
#define SIMTIME_H_STPF70AM

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

void sim_time_tick(double dt);
void sim_time_tick_ms(int64_t dms);

double sim_time_get_jd(void);
time_t sim_time_get_time(void);
int64_t sim_time_jd_to_time_stamp(double jd);
time_t sim_time_get_time(void);
int64_t sim_time_get_time_stamp(void);

#endif /* end of include guard: SIMTIME_H_STPF70AM */

