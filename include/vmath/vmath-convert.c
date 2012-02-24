/*
 Copyright 2010 Mattias Holm <mattias.holm(at)openorbit.org>
 
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

#include <vmath/vmath-convert.h>

float3
equ2cart_f(float ra, float dec)
{
  float3 cart;
  float cosdec = cosf(dec);
  cart = vf3_set(cosdec * cosf(ra), cosdec * sinf(ra), sinf(dec));
  return cart;
}

double3
equ2cart_d(double ra, double dec)
{
  double3 cart;
  double cosdec = cos(dec);
  cart = vd3_set(cosdec * cos(ra), cosdec * sin(ra), sin(dec));
  return cart;
}

void
cart2geodetic_f(float3 p, float a, float e,
           float * restrict latitude,
           float * restrict longitude,
           float * restrict altitude)
{
  // Compute xyz coordinates from lat/long/alt
  float3 pxy = vf3_set(vf3_x(p), vf3_y(p), 0.0f);
  float b = a * sqrtf(1.0f-e*e); // semi minor axis for body
  float f = (a - b) / a; // flattening factor
  // Longitude is easy...
  *longitude = atan(vf3_y(p)/ vf3_x(p));

  // Solve latitude
  float C;
  float phi_i0, phi_i1;
  float tol = 0.01f; // 1 cm accuracy
  float R = vf3_abs(pxy);
  float z = vf3_z(p);
  // Initial estimate of latitude
  phi_i1 = atan(z/vf3_abs(pxy));

  // TODO: Add iteration limit
  do {
    phi_i0 = phi_i1;
    
    C = 1.0f / (1.0f-(2.0f*f-f*f)*sinf(phi_i0)*sinf(phi_i0));
    phi_i1 = atanf(z + a*C*(2*f-f*f)*sinf(phi_i0)/R);
  } while (fabsf(phi_i1-phi_i0) > tol);

  *altitude = R/cos(phi_i1) - a * C;
}

float3
geodetic2cart_f(float a, float e,
                float latitude,
                float longitude,
                float altitude)
{
  float xi = sqrt(1.0f-e*e*sinf(latitude)*sinf(latitude));
  float x = (a / xi + altitude) * cosf(latitude) * cosf(longitude);
  float y = (a / xi + altitude) * cosf(latitude) * sinf(longitude);
  float z = (a*(1.0f-e*e) / xi + altitude) * sinf(latitude);

  return vf3_set(x, y, z);
}
