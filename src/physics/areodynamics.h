/*
 Copyright 2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#ifndef PL_ATMOSPHERE_H
#define PL_ATMOSPHERE_H

#include "physics/reftypes.h"
#include <vmath/vmath.h>
#include <gencds/array.h>

typedef enum pl_element_t {
  PL_ELEM_First = 0, PL_ELEM_n = 0,
  // Period 1
  PL_ELEM_H = 1, PL_ELEM_He,

  // Period 2
  PL_ELEM_Li, PL_ELEM_Be, PL_ELEM_B, PL_ELEM_C,
  PL_ELEM_N, PL_ELEM_O, PL_ELEM_F, PL_ELEM_Ne,

  // Period 3
  PL_ELEM_Na, PL_ELEM_Mg, PL_ELEM_Al, PL_ELEM_Si,
  PL_ELEM_P, PL_ELEM_S, PL_ELEM_Cl, PL_ELEM_Ar,

  // Period 4
  PL_ELEM_K, PL_ELEM_Ca, PL_ELEM_Sc, PL_ELEM_Ti,
  PL_ELEM_V, PL_ELEM_Cr, PL_ELEM_Mn, PL_ELEM_Fe,
  PL_ELEM_Co, PL_ELEM_Ni, PL_ELEM_Cu, PL_ELEM_Zn,
  PL_ELEM_Ga, PL_ELEM_Ge, PL_ELEM_As, PL_ELEM_Se,
  PL_ELEM_Br, PL_ELEM_Kr,

  // Period 5
  PL_ELEM_Rb, PL_ELEM_Sr, PL_ELEM_Y, PL_ELEM_Zr,
  PL_ELEM_Nb, PL_ELEM_Mo, PL_ELEM_Tc, PL_ELEM_Ru,
  PL_ELEM_Rh, PL_ELEM_Pd, PL_ELEM_Ag, PL_ELEM_Cd,
  PL_ELEM_In, PL_ELEM_Sn, PL_ELEM_Sb, PL_ELEM_Te,
  PL_ELEM_I, PL_ELEM_Xe,

  // Period 6
  PL_ELEM_Cs, PL_ELEM_Ba,

  // Lanthanoids
  PL_ELEM_La, PL_ELEM_Ce, PL_ELEM_Pr, PL_ELEM_Nd,
  PL_ELEM_Pm, PL_ELEM_Sm, PL_ELEM_Eu, PL_ELEM_Gd,
  PL_ELEM_Tb, PL_ELEM_Dy, PL_ELEM_Ho, PL_ELEM_Er,
  PL_ELEM_Tm, PL_ELEM_Yb, PL_ELEM_Lu,

  PL_ELEM_Hf, PL_ELEM_Ta, PL_ELEM_W, PL_ELEM_Re,
  PL_ELEM_Os, PL_ELEM_Ir, PL_ELEM_Pt, PL_ELEM_Au,
  PL_ELEM_Hg, PL_ELEM_Tl, PL_ELEM_Pb, PL_ELEM_Bi,
  PL_ELEM_Po, PL_ELEM_At, PL_ELEM_Rn,

  // Period 7
  PL_ELEM_Fr, PL_ELEM_Ra,

  // Actinoids
  PL_ELEM_Ac, PL_ELEM_Th, PL_ELEM_Pa, PL_ELEM_U,
  PL_ELEM_Np, PL_ELEM_Pu, PL_ELEM_Am, PL_ELEM_Cm,
  PL_ELEM_Bk, PL_ELEM_Cf, PL_ELEM_Es, PL_ELEM_Fm,
  PL_ELEM_Md, PL_ELEM_No, PL_ELEM_Lr,

  PL_ELEM_Rf, PL_ELEM_Db, PL_ELEM_Sg, PL_ELEM_Bh,
  PL_ELEM_Hs, PL_ELEM_Mt, PL_ELEM_Ds, PL_ELEM_Rg,
  PL_ELEM_Cn, PL_ELEM_Uut, PL_ELEM_Uuq, PL_ELEM_Uup,
  PL_ELEM_Uuh, PL_ELEM_Uus, PL_ELEM_Uuo,

  PL_ELEM_Last,
} pl_element_t;

typedef struct pl_molecule_t {
  unsigned elems;

  struct {
    unsigned count;
    pl_element_t elem;
  } atoms[];
} pl_molecule_t;

// Atmospheres are divided into troposphere, stratosphere, mesosphere,
// thermosphere and exosphere. The boundaries between these are the
// tropopause, stratopause, mesopause, thermopause.
// In order to have a reasonable model of a planets atmosphere, we need to take
// all these levels into account. However, we generalize this as follows:
// an arbitrary number of layers can be specified, each layer having different
// properties. A template object is then created, which in turn is used to build
// an atmosphere object which contain a number of sample points of fixed
// distance. The values are then interpolated between the sample points.

typedef struct pl_atm_layer_t pl_atm_layer_t;
typedef struct pl_atm_template_t pl_atm_template_t;

typedef float (*pl_atm_comp_fn_t)(pl_atm_layer_t *,float);

struct pl_atm_layer_t {
  float g0;
  float P_b;
  float T_b;
  float M; // Molar mass (kg / mol)
  float p_b;
  float h_b;
  float L_b;
  pl_atm_comp_fn_t compPressure;
  pl_atm_comp_fn_t compDensity;
};

struct pl_atm_template_t {
  size_t layer_count;
  pl_atm_layer_t layer[];
};


typedef struct PLatmosphere {
  float P0; // Standard pressure at "ground"-level
  float T0; // Standard temperature at "ground"-level
  float M; // Molar mass (kg / mol)
  float p0; // Standard density at "ground"-level
  float h0; // Scale height of atmosphere

  float sample_distance;
  float_array_t P; // Pressure points
  float_array_t p; // Density points
} PLatmosphere;

double pl_object_compute_altitude(PLobject *obj);
float3 pl_compute_airvelocity(PLobject *obj);
double pl_object_compute_airspeed(PLobject *obj);
double pl_object_compute_airpressure(PLobject *obj);
double pl_object_compute_airdensity(PLobject *obj);
float3 pl_object_compute_drag(PLobject *obj);
double pl_object_compute_airdensity_with_current_pressure(PLobject *obj);
void pl_atmosphere_init(PLatmosphere *atm, float groundPressure, float h0);

/*!
 Computes the air preasure at a given altitude, note that for simplicitys sake,
 you can call the methods with 0 for the hb value and use standard ground level
 parameters for Pb and Tb.
 \param Pb Static preasure (reference)
 \param Tb Standard temperature
 \param g0 Gravitational accelleration
 \param M Molar mass of atmosphere
 \param h Height above sea level
 \param hb height of reference point (for Pb / Tb)
 \param Lb Lapse rate in kelvin per metre
 */
double
pl_pressure_at_altitude_with_lapse(double Pb, double Tb, double g0, double M,
                              double h, double hb, double Lb);

double
pl_pressure_at_altitude(double Pb, double Tb, double g0, double M, double h,
                     double hb);

// Simplified atmospheric model
float pl_atmosphere_simple_compute_airpressure(const PLatmosphere *atm, float h);

float pl_atm_template_compute_airpressure(pl_atm_template_t *atm, float h);
float pl_atm_template_compute_airdensity(pl_atm_template_t *atm, float h);

pl_atm_template_t*
pl_new_atmosphere_template(size_t layers, double g0, double M,
                           const double *p_b, const double *P_b,
                           const double *T_b, const double *h_b,
                           const double *L_b);
PLatmosphere* pl_new_atmosphere(float sample_dist, float h,
                                pl_atm_template_t *t);
float pl_atmosphere_density(const PLatmosphere *atm, float h);
float pl_atmosphere_pressure(const PLatmosphere *atm, float h);


typedef struct PLairfoil {
  float area;
  float Ct0;
} PLairfoil;

float3 pl_atmosphere_compute_lift(PLatmosphere *atm, PLobject *obj, PLairfoil *foil);


#endif /* ! PL_ATMOSPHERE_H */
