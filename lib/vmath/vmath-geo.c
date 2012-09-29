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


#include <math.h>


float
el_circumference_fastf(float semiMajor, float semiMinor)
{
  return M_PI * (3.0f*(semiMajor + semiMinor) -
                 sqrtf((3.0f*semiMajor+semiMinor) *
                       (semiMajor+3.0f*semiMinor)));
}

double
el_circumference_fast(double semiMajor, double semiMinor)
{
  return M_PI * (3.0*(semiMajor + semiMinor) -
                 sqrt((3.0*semiMajor+semiMinor) *
                       (semiMajor+3.0*semiMinor)));
}


float
el_circumference_accuratef(float semiMajor, float semiMinor)
{
  float abOverAb = (semiMajor-semiMinor)/(semiMajor+semiMinor);
  float abOverAbSquare3 = 3.0f * abOverAb * abOverAb;

  return M_PI * (semiMajor + semiMinor) *
         (1.0f + abOverAbSquare3 / (10.0f + sqrtf(4.0f-abOverAbSquare3)));
}

double
el_circumference_accurate(double semiMajor, double semiMinor)
{
  double abOverAb = (semiMajor-semiMinor)/(semiMajor+semiMinor);
  double abOverAbSquare3 = 3.0 * abOverAb * abOverAb;

  return M_PI * (semiMajor + semiMinor) *
  (1.0 + abOverAbSquare3 / (10.0 + sqrt(4.0-abOverAbSquare3)));
}

float
el_areaf(float semiMajor, float semiMinor)
{
  return M_PI * semiMajor * semiMinor;
}

double
el_area(double semiMajor, double semiMinor)
{
  return M_PI * semiMajor * semiMinor;
}

float
el_true_anomalyf(float semiMajor, float ecc, float fi)
{
  return (semiMajor * (1.0f - ecc*ecc)) / (1.0f + ecc * cosf(fi));
}

double
el_true_anomaly(double semiMajor, double ecc, double fi)
{
  return (semiMajor * (1.0 - ecc*ecc)) / (1.0 + ecc * cos(fi));
}

float
el_segment_areaf(float semiMajor, float ecc,  float fi0, float fi1)
{
  float r0 = el_true_anomalyf(semiMajor, ecc, fi0);
  float r1 = el_true_anomalyf(semiMajor, ecc, fi1);

  float x0 = r0 * cosf(fi0);
  float y0 = r0 * sinf(fi0);
  float x1 = r1 * cosf(fi1);
  float y1 = r1 * sinf(fi1);

  float seg = 0.5f * fabsf(x0*y1-x1*y0);

  return seg;
}

double
el_segment_area(double semiMajor, double ecc, double fi0, double fi1)
{
  double r0 = el_true_anomaly(semiMajor, ecc, fi0);
  double r1 = el_true_anomaly(semiMajor, ecc, fi1);

  double x0 = r0 * cos(fi0);
  double y0 = r0 * sin(fi0);
  double x1 = r1 * cos(fi1);
  double y1 = r1 * sin(fi1);

  double seg = 0.5 * fabs(x0*y1-x1*y0);

  return seg;
}


float
el_eccf(float semiMajor, float semiMinor)
{
  return sqrtf(1.0f-(semiMinor/semiMajor)*(semiMinor/semiMajor));
}

double
el_ecc(double semiMajor, double semiMinor)
{
  return sqrt(1.0-(semiMinor/semiMajor)*(semiMinor/semiMajor));
}








