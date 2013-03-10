/*
 Copyright 2009,2013 Mattias Holm <lorrden(at)openorbit.org>
 
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


#ifndef SIMENVIRONMENT_H__
#define SIMENVIRONMENT_H__

typedef struct sim_environment_t sim_environment_t;

struct sim_environment_t {
  float airPressure; // Pa
  float airDensity;
  float aoa; // angle of attack
};

void sim_compute_env(sim_environment_t *env, void *sc);

#endif // SIMENVIRONMENT_H__