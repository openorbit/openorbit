/*
 Copyright 2012 Mattias Holm <mattias.holm(at)openorbit.org>

 This file is part of Open Orbit. Open Orbit is free software: you can
 redistribute it and/or modify it under the terms of the GNU General Public
 License as published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 You should have received a copy of the GNU General Public License
 along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.

 Some files of Open Orbit have relaxed licensing conditions. This file is
 licenced under the 2-clause BSD licence.

 Redistribution and use of this file in source and binary forms, with or
 without modification, are permitted provided that the following conditions are
 met:

 - Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#ifndef orbit_vmath_geo_h
#define orbit_vmath_geo_h


float   el_circumference_fastf(float semiMajor, float semiMinor);
double  el_circumference_fast(double semiMajor, double semiMinor);

float   el_circumference_accuratef(float semiMajor, float semiMinor);
double  el_circumference_accurate(double semiMajor, double semiMinor);

float   el_areaf(float semiMajor, float semiMinor);
double  el_area(double semiMajor, double semiMinor);

float   el_true_anomalyf(float semiMajor, float ecc, float fi);
double  el_true_anomaly(double semiMajor, double ecc, double fi);

float   el_segment_areaf(float semiMajor, float ecc,  float fi0, float fi1);
double  el_segment_area(double semiMajor, double ecc, double fi0, double fi1);

float   el_eccf(float semiMajor, float semiMinor);
double  el_ecc(double semiMajor, double semiMinor);


#endif
