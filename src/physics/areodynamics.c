/*
  Copyright 2009,2010,2011 Mattias Holm <mattias.holm(at)openorbit.org>

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

#include "physics.h"
#include "areodynamics.h"
#include <vmath/vmath.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <gencds/hashtable.h>
#include "common/palloc.h"
#include "common/moduleinit.h"
#include <openorbit/log.h>
// Note that these are molar weights for all elements, this does not take into
// account that some elements appear in molecules (i.e. H2)
// TODO: Set all elements that are -0.0 to NAN, but this cannot be done until
//       there is a new official release of clang for OS X
static double elementMolarWeights[PL_ELEM_Last] = {
  [PL_ELEM_n] = -0.0, // Neutron, but this is mosly here to be able to index by
                      // proton count
  [PL_ELEM_H] = 0.00100794, [PL_ELEM_He] = 0.004002602,

  // Period 2
  [PL_ELEM_Li] = 0.006941,     [PL_ELEM_Be] = 0.009012,
  [PL_ELEM_B]  = 0.010811,     [PL_ELEM_C]  = 0.0120107,
  [PL_ELEM_N]  = 0.0140067,    [PL_ELEM_O]  = 0.0159994,
  [PL_ELEM_F]  = 0.0189984032, [PL_ELEM_Ne] = 0.0201797,

  // Period 3
  [PL_ELEM_Na] = 0.02298977,  [PL_ELEM_Mg] = 0.0243050,
  [PL_ELEM_Al] = 0.026981538, [PL_ELEM_Si] = 0.0280855,
  [PL_ELEM_P]  = 0.030973761, [PL_ELEM_S]  = 0.032066,
  [PL_ELEM_Cl] = 0.0354527,   [PL_ELEM_Ar] = 0.039948,

  // Period 4
  [PL_ELEM_K]  = 0.0390983,   [PL_ELEM_Ca] = 0.040078,
  [PL_ELEM_Sc] = 0.044955910, [PL_ELEM_Ti] = 0.047867,
  [PL_ELEM_V]  = 0.0509415,   [PL_ELEM_Cr] = 0.0519961,
  [PL_ELEM_Mn] = 0.054938049, [PL_ELEM_Fe] = 0.055845,
  [PL_ELEM_Co] = 0.058933200, [PL_ELEM_Ni] = 0.0586934,
  [PL_ELEM_Cu] = 0.063546,    [PL_ELEM_Zn] = 0.06539,
  [PL_ELEM_Ga] = 0.069723,    [PL_ELEM_Ge] = 0.07261,
  [PL_ELEM_As] = 0.07492160,  [PL_ELEM_Se] = 0.07896,
  [PL_ELEM_Br] = 0.079904,    [PL_ELEM_Kr] = 0.08380,

  // Period 5
  [PL_ELEM_Rb] = 0.0854678,  [PL_ELEM_Sr] = 0.08762,
  [PL_ELEM_Y]  = 0.088905,   [PL_ELEM_Zr] = 0.091224,
  [PL_ELEM_Nb] = 0.09290638, [PL_ELEM_Mo] = 0.09594,
  [PL_ELEM_Tc] = -0.0,       [PL_ELEM_Ru] = 0.10107,
  [PL_ELEM_Rh] = 0.10290550, [PL_ELEM_Pd] = 0.10642,
  [PL_ELEM_Ag] = 0.1078682,  [PL_ELEM_Cd] = 0.112411,
  [PL_ELEM_In] = 0.114818,   [PL_ELEM_Sn] = 0.118710,
  [PL_ELEM_Sb] = 0.121760,   [PL_ELEM_Te] = 0.12760,
  [PL_ELEM_I]  = 0.12690447, [PL_ELEM_Xe] = 0.13129,

  // Period 6
  [PL_ELEM_Cs] = 0.13290545, [PL_ELEM_Ba] = 0.137327,

  // Lanthanoids
  [PL_ELEM_La] = 0.1389055,  [PL_ELEM_Ce] = 0.140116,
  [PL_ELEM_Pr] = 0.14090765, [PL_ELEM_Nd] = 0.14424,
  [PL_ELEM_Pm] = -0.0,       [PL_ELEM_Sm] = 0.15036,
  [PL_ELEM_Eu] = 0.151964,   [PL_ELEM_Gd] = 0.15725,
  [PL_ELEM_Tb] = 0.15892534, [PL_ELEM_Dy] = 0.16250,
  [PL_ELEM_Ho] = 0.16493032, [PL_ELEM_Er] = 0.1676,
  [PL_ELEM_Tm] = 0.16893421, [PL_ELEM_Yb] = 0.17304,
  [PL_ELEM_Lu] = 0.174967,

  [PL_ELEM_Hf] = 0.17849,  [PL_ELEM_Ta] = 0.1809479,
  [PL_ELEM_W]  = 0.18384,  [PL_ELEM_Re] = 0.186207,
  [PL_ELEM_Os] = 0.19023,  [PL_ELEM_Ir] = 0.192217,
  [PL_ELEM_Pt] = 0.195078, [PL_ELEM_Au] = 0.19696655,
  [PL_ELEM_Hg] = 0.20059,  [PL_ELEM_Tl] = 0.2043833,
  [PL_ELEM_Pb] = 0.2072,   [PL_ELEM_Bi] = 0.20898038,
  [PL_ELEM_Po] = -0.0,     [PL_ELEM_At] = -0.0,
  [PL_ELEM_Rn] = -0.0,

  // Period 7
  [PL_ELEM_Fr] = -0.0, [PL_ELEM_Ra] = -0.0,

  // Actinoids
  [PL_ELEM_Ac] = -0.0,       [PL_ELEM_Th] = 0.2320381,
  [PL_ELEM_Pa] = 0.23103588, [PL_ELEM_U]  = 0.2380289,
  [PL_ELEM_Np] = -0.0,       [PL_ELEM_Pu] = -0.0,
  [PL_ELEM_Am] = -0.0,       [PL_ELEM_Cm] = -0.0,
  [PL_ELEM_Bk] = -0.0,       [PL_ELEM_Cf] = -0.0,
  [PL_ELEM_Es] = -0.0,       [PL_ELEM_Fm] = -0.0,
  [PL_ELEM_Md] = -0.0,       [PL_ELEM_No] = -0.0,
  [PL_ELEM_Lr] = -0.0,

  [PL_ELEM_Rf]  = -0.0, [PL_ELEM_Db]  = -0.0,
  [PL_ELEM_Sg]  = -0.0, [PL_ELEM_Bh]  = -0.0,
  [PL_ELEM_Hs]  = -0.0, [PL_ELEM_Mt]  = -0.0,
  [PL_ELEM_Ds]  = -0.0, [PL_ELEM_Rg]  = -0.0,
  [PL_ELEM_Cn]  = -0.0, [PL_ELEM_Uut] = -0.0,
  [PL_ELEM_Uuq] = -0.0, [PL_ELEM_Uup] = -0.0,
  [PL_ELEM_Uuh] = -0.0, [PL_ELEM_Uus] = -0.0,
  [PL_ELEM_Uuo] = -0.0,
};

static char *elemNames[PL_ELEM_Last] = {
  [PL_ELEM_n] = "neutron",
  [PL_ELEM_H] = "hydrogen", [PL_ELEM_He] = "helium",

  // Period 2
  [PL_ELEM_Li] = "lithium",     [PL_ELEM_Be] = "beryllium",
  [PL_ELEM_B]  = "boron",     [PL_ELEM_C]  = "carbon",
  [PL_ELEM_N]  = "nitrogen",    [PL_ELEM_O]  = "oxygen",
  [PL_ELEM_F]  = "flourine", [PL_ELEM_Ne] = "neon",

  // Period 3
  [PL_ELEM_Na] = "sodium",  [PL_ELEM_Mg] = "magnesium",
  [PL_ELEM_Al] = "aluminium", [PL_ELEM_Si] = "silicon",
  [PL_ELEM_P]  = "phosphorus", [PL_ELEM_S]  = "sulfur",
  [PL_ELEM_Cl] = "chlorine",   [PL_ELEM_Ar] = "argon",

  // Period 4
  [PL_ELEM_K]  = "potassium",   [PL_ELEM_Ca] = "calcium",
  [PL_ELEM_Sc] = "scandium", [PL_ELEM_Ti] = "titanium",
  [PL_ELEM_V]  = "vanadium",   [PL_ELEM_Cr] = "chromium",
  [PL_ELEM_Mn] = "manganese", [PL_ELEM_Fe] = "iron",
  [PL_ELEM_Co] = "cobalt", [PL_ELEM_Ni] = "nickel",
  [PL_ELEM_Cu] = "copper",    [PL_ELEM_Zn] = "zinc",
  [PL_ELEM_Ga] = "gallium",    [PL_ELEM_Ge] = "germanium",
  [PL_ELEM_As] = "arsenic",  [PL_ELEM_Se] = "selenium",
  [PL_ELEM_Br] = "bromine",    [PL_ELEM_Kr] = "krypton",

  // Period 5
  [PL_ELEM_Rb] = "rubidium",  [PL_ELEM_Sr] = "strontium",
  [PL_ELEM_Y]  = "yttrium",   [PL_ELEM_Zr] = "zirconium",
  [PL_ELEM_Nb] = "niobium", [PL_ELEM_Mo] = "molybdenum",
  [PL_ELEM_Tc] = "technetium",       [PL_ELEM_Ru] = "ruthenium",
  [PL_ELEM_Rh] = "rhodium", [PL_ELEM_Pd] = "palladium",
  [PL_ELEM_Ag] = "silver",  [PL_ELEM_Cd] = "cadmium",
  [PL_ELEM_In] = "indium",   [PL_ELEM_Sn] = "tin",
  [PL_ELEM_Sb] = "antimony",   [PL_ELEM_Te] = "tellurium",
  [PL_ELEM_I]  = "iodine", [PL_ELEM_Xe] = "xenon",

  // Period 6
  [PL_ELEM_Cs] = "cesium", [PL_ELEM_Ba] = "barium",

  // Lanthanoids
  [PL_ELEM_La] = "lanthanum",  [PL_ELEM_Ce] = "cerium",
  [PL_ELEM_Pr] = "praseodymium", [PL_ELEM_Nd] = "neodymium",
  [PL_ELEM_Pm] = "promethium",       [PL_ELEM_Sm] = "samarium",
  [PL_ELEM_Eu] = "europium",   [PL_ELEM_Gd] = "gadolinium",
  [PL_ELEM_Tb] = "terbium", [PL_ELEM_Dy] = "dysprosium",
  [PL_ELEM_Ho] = "holmium", [PL_ELEM_Er] = "erbium",
  [PL_ELEM_Tm] = "thulium", [PL_ELEM_Yb] = "ytterbium",
  [PL_ELEM_Lu] = "lutecium",

  [PL_ELEM_Hf] = "hafnium",  [PL_ELEM_Ta] = "tantalum",
  [PL_ELEM_W]  = "tungsten",  [PL_ELEM_Re] = "rhenium",
  [PL_ELEM_Os] = "osmium",  [PL_ELEM_Ir] = "iridium",
  [PL_ELEM_Pt] = "platinum", [PL_ELEM_Au] = "gold",
  [PL_ELEM_Hg] = "mercury",  [PL_ELEM_Tl] = "thallium",
  [PL_ELEM_Pb] = "lead",   [PL_ELEM_Bi] = "bismuth",
  [PL_ELEM_Po] = "polonium",     [PL_ELEM_At] = "astatine",
  [PL_ELEM_Rn] = "radon",

  // Period 7
  [PL_ELEM_Fr] = "francium", [PL_ELEM_Ra] = "radium",

  // Actinoids
  [PL_ELEM_Ac] = "actinium",       [PL_ELEM_Th] = "thorium",
  [PL_ELEM_Pa] = "protactinium", [PL_ELEM_U]  = "uranium",
  [PL_ELEM_Np] = "neptunium",       [PL_ELEM_Pu] = "plutonium",
  [PL_ELEM_Am] = "americium",       [PL_ELEM_Cm] = "curium",
  [PL_ELEM_Bk] = "berkelium",       [PL_ELEM_Cf] = "californium",
  [PL_ELEM_Es] = "einsteinium",       [PL_ELEM_Fm] = "fermium",
  [PL_ELEM_Md] = "mendelevium",       [PL_ELEM_No] = "nobelium",
  [PL_ELEM_Lr] = "lawrencium",

  [PL_ELEM_Rf]  = "rutherfordium", [PL_ELEM_Db]  = "dubnium",
  [PL_ELEM_Sg]  = "seaborgium", [PL_ELEM_Bh]  = "bohrium",
  [PL_ELEM_Hs]  = "hassium", [PL_ELEM_Mt]  = "meitnerium",
  [PL_ELEM_Ds]  = "darmstadtium", [PL_ELEM_Rg]  = "aoentgenium",
  [PL_ELEM_Cn]  = "copernicium", [PL_ELEM_Uut] = "ununtrium",
  [PL_ELEM_Uuq] = "ununquadium", [PL_ELEM_Uup] = "ununpentium",
  [PL_ELEM_Uuh] = "ununhexium", [PL_ELEM_Uus] = "ununseptium",
  [PL_ELEM_Uuo] = "ununoctium",
};

static hashtable_t *ht;

MODULE_INIT(areodynamics, NULL)
{
  log_trace("initialising 'areodynamics' module");
  ht = hashtable_new_with_str_keys(128);

  for (pl_element_t i = PL_ELEM_First ; i < PL_ELEM_Last ; ++ i) {
    hashtable_insert(ht, elemNames[i], (void*)i);
  }
}

pl_element_t
pl_element_id_from_name(const char *name)
{
  char nameCopy[strlen(name)+1];

  for (int i = 0; i < strlen(name)+1 ; ++ i) {
    nameCopy[i] = tolower(name[i]); // Will pass null marker unchanged
  }

  pl_element_t i = (pl_element_t) hashtable_lookup(ht, nameCopy);
  return i;
}

const char*
pl_element_name_from_id(pl_element_t elem)
{
  assert(elem >= PL_ELEM_First);
  assert(elem < PL_ELEM_Last);

  return elemNames[elem];
}

double
pl_molecule_weight(unsigned nAtoms, pl_element_t elem)
{
  return (double)nAtoms * elementMolarWeights[elem];
}


double
pl_compute_individual_gas_const(double M)
{
  return PL_UGC / M;
}

double
pl_compute_density(double molWeight, double P, double T)
{
  return (molWeight*P) / (PL_UGC * T);
}

double
pl_estimate_temp(double luminocity, double albedo, double R)
{
  // Quad root
  double t = pow((luminocity*(1.0 - albedo))/(4.0 * M_PI * PL_ST*R*R),
                 1.0/4.0);
  return t;
}

// Computes the current airspeed of the object, the airspeed is the
// current speed of the object with the speed of the planet and the current
// rotation tangent subtracted

float3
pl_compute_airvelocity(pl_object_t *obj)
{
  // TODO: Adjust for planet rotation and wind
  // TOOD: Replace deprecated system with other mechanism
  return -obj->v; //(obj->v - obj->sys->orbitalBody->obj.v);
}

double
pl_object_compute_airspeed(pl_object_t *obj)
{
  return vf3_abs(pl_compute_airvelocity(obj));
}
// Compute drag force from
// \param v Velocity of object of fluid
// \param p Density of fluid
// \param Cd Drag coeficient for object
// \param A Area of object (i.e. the orthographic projection area)
// \result A vector with the drag force (oposite direction of velocity)
float3
pl_compute_drag(float3 v, double p, double Cd, double A)
{
  double v_mag = vf3_abs(vf3_neg(v));
  double fd = 0.5 * p * v_mag * v_mag * Cd * A;
  float3 vf_dir = vf3_normalise(v);
  return vf3_s_mul(vf_dir, fd);
}

// Computing lift of airfoils using thin airfoil theory
// Symetric airfoil: CL = 2 * M_PI * alpha
// Chambered airfoil: CL = Ct0 + 2 * M_PI * alpha
// Lift: 0.5 * density * velocity ** 2 * A * CL
//  where A is the planform area, i.e. area of the wings
float3
pl_atmosphere_compute_lift(PLatmosphere *atm, pl_object_t *obj, PLairfoil *foil)
{
  //float3 vel = plComputeAirvelocity(obj);
  //float speed = plComputeAirspeed(obj);

  // TODO: Compute alpha from rotation and velocity, obviously this should
  //       somehow be adjusted for beta and gamma, but we skip that for now.
  //float alpha = 0.0f;
  //float CL = foil->Ct0 + 2.0f * M_PI * alpha;

  //float lift = 0.5f * plComputeAirdensity(obj) * speed * speed * foil->area * CL;

  return vf3_set(0.0f, 0.0f, 0.0f);
}


void
pl_atmosphere_init(PLatmosphere *atm, float groundPressure, float h0)
{
  atm->P0 = groundPressure;
  atm->T0 = NAN;
  atm->M = NAN;
  atm->p0 = NAN;
  atm->h0 = h0;
}


/*! Compute drag for a given object
 *  We need to compute parasitic drag (form, profile, interference and skin
 *  friction drag), and the drag from any airfoils.
 */
float3
pl_object_compute_drag(pl_object_t *obj)
{
  double p = pl_object_compute_airdensity(obj);
  float3 vel = pl_compute_airvelocity(obj); // TODO: Adjust for planet rotation

  float3 drag = pl_compute_drag(vel, p, obj->dragCoef, obj->area);
  return drag;//vf3_set(0.0, 0.0, 0.0);
}

double
pl_object_compute_airpressure(pl_object_t *obj)
{
  return 0.0;
  // TODO: Update to new physics system.
#if 0
  pl_system_t *sys = obj->sys;
  PLatmosphere *atm = obj->sys->orbitalBody->atm;
  float3 dist = lwc_dist(&obj->p, &sys->orbitalBody->obj.p);
  //float g0 = sys->orbitalBody->GM / (sys->orbitalBody->eqRad * sys->orbitalBody->eqRad); // TODO: Cache g0
  float h = vf3_abs(dist) - sys->orbitalBody->eqRad; // TODO: adjust for oblateness
  double pressure = pl_atmosphere_pressure(atm, h);//plComputeSimpleAirpressure(atm, h);
  //plPressureAtAltitude(atm->P0, atm->T0, g0, atm->M, h, 0.0);
  //log_info("airpressure for %s = %f Pa", obj->name, pressure);

  return pressure;
#endif
}

double
pl_object_compute_altitude(pl_object_t *obj)
{
  return 0.0;
  // TODO: Fix for new physics system
#if 0
  pl_system_t *sys = obj->sys;
  float3 dist = lwc_dist(&obj->p, &sys->orbitalBody->obj.p);
  float h = vf3_abs(dist) - sys->orbitalBody->eqRad;
  return h;
#endif
}

double
pl_object_compute_airdensity(pl_object_t *obj)
{
  return 0.0;
  // TODO: Fix for new physics system

#if 0
  pl_system_t *sys = obj->sys;
  PLatmosphere *atm = obj->sys->orbitalBody->atm;
  float3 dist = lwc_dist(&obj->p, &sys->orbitalBody->obj.p);
  //float g0 = sys->orbitalBody->GM / (sys->orbitalBody->eqRad * sys->orbitalBody->eqRad); // TODO: Cache g0
  float h = vf3_abs(dist) - sys->orbitalBody->eqRad; // TODO: adjust for oblateness
  //double pressure = plComputeSimpleAirpressure(atm, h);

  double density = pl_atmosphere_density(atm, h);
  //log_info("altitude for %s = %f m above surface of %s", obj->name, h,
  //          obj->sys->orbitalBody->name);
  //log_info("atmosphere: P = %f Pa, h0 = %f m", atm->P0, atm->h0);
  //log_info("airdensity for %s = %f kg/m**3", obj->name, density);
  //plPressureAtAltitude(atm->p0, atm->T0, g0, atm->M, h, 0.0);
  //return pressure * atm->M / (PL_UGC * atm->T0);
  return density;
#endif
}


//double
//plComputeAirdensityWithCurrentPressure(pl_object_t *obj)
//{
//  PLatmosphere *atm = obj->sys->orbitalBody->atm;
//  return obj->airPressure * atm->M / (PL_UGC * atm->T0);
//}


/*!
 Computes the air pressure at a given altitude, note that for simplicitys sake,
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
                              double h, double hb, double Lb)
{
  return Pb * pow(Tb/(Tb+Lb*(h-hb)), g0*M / (PL_UGC*Lb));
}


double
pl_pressure_at_altitude(double Pb, double Tb, double g0, double M, double h,
                     double hb)
{
  return Pb * exp((-g0 * M * (h-hb)) / (PL_UGC * Tb));
}

float
pl_atmosphere_simple_compute_airpressure(const PLatmosphere *atm, float h)
{
  double e = exp(-h/atm->h0);
  double p = atm->P0 * e;
  return p;
}

float
comp_pressure_lapse(pl_atm_layer_t *atm, float h)
{
  return atm->P_b * pow(atm->T_b/(atm->T_b+atm->L_b*(h-atm->h_b)),
                        atm->g0 * atm->M / (PL_UGC*atm->L_b));
}


float
comp_pressure_no_lapse(pl_atm_layer_t *atm, float h)
{
  return atm->P_b * exp((-atm->g0 * atm->M * (h - atm->h_b)) / (PL_UGC * atm->T_b));
}


float
comp_density_lapse(pl_atm_layer_t *atm, float h)
{
  return atm->p_b * pow((atm->T_b+atm->L_b*(h-atm->h_b))/atm->T_b,
                        (-atm->g0 * atm->M / (PL_UGC*atm->L_b)) - 1.0);
}


float
comp_density_no_lapse(pl_atm_layer_t *atm, float h)
{
  return atm->p_b * exp((-atm->g0 * atm->M * (h - atm->h_b)) / (PL_UGC * atm->T_b));
}



float
pl_atm_template_compute_airpressure(pl_atm_template_t *atm, float h)
{
  int i = 0;
  for (i = 0 ; i < atm->layer_count - 1 ; i++) {
    if (atm->layer[i+1].h_b > h) break;
  }
  return atm->layer[i].compPressure(&atm->layer[i], h);
}

float
pl_atm_template_compute_airdensity(pl_atm_template_t *atm, float h)
{
  int i = 0;
  for (i = 0 ; i < atm->layer_count - 1 ; i++) {
    if (atm->layer[i+1].h_b > h) break;
  }
  return atm->layer[i].compDensity(&atm->layer[i], h);
}

pl_atm_template_t*
pl_new_atmospheref(size_t layers, float g0, float M, float *p_b, float *P_b,
             float *T_b, float *h_b, float *L_b)
{
  pl_atm_template_t *atm = smalloc(sizeof(pl_atm_template_t) +
                                     sizeof(pl_atm_layer_t) * layers);
  atm->layer_count = layers;
  for (int i = 0 ; i < layers ; i++) {
    atm->layer[i].g0 = g0;
    atm->layer[i].M = M;
    atm->layer[i].h_b = h_b[i];
    atm->layer[i].L_b = L_b[i];
    atm->layer[i].P_b = P_b[i];
    atm->layer[i].p_b = p_b[i];
    atm->layer[i].T_b = T_b[i];
    if (L_b[i] == 0.0) {
      atm->layer[i].compPressure = comp_pressure_no_lapse;
      atm->layer[i].compDensity = comp_density_no_lapse;
    } else {
      atm->layer[i].compPressure = comp_pressure_lapse;
      atm->layer[i].compDensity = comp_density_lapse;
    }
  }

  return atm;
}


pl_atm_template_t*
pl_new_atmosphere_template(size_t layers, double g0, double M, const double *p_b,
              const double *P_b, const double *T_b, const double *h_b,
              const double *L_b)
{
  pl_atm_template_t *atm = smalloc(sizeof(pl_atm_template_t) +
                                     sizeof(pl_atm_layer_t) * layers);
  atm->layer_count = layers;
  for (int i = 0 ; i < layers ; i++) {
    atm->layer[i].g0 = g0;
    atm->layer[i].M = M;
    atm->layer[i].h_b = h_b[i];
    atm->layer[i].L_b = L_b[i];
    atm->layer[i].P_b = P_b[i];
    atm->layer[i].p_b = p_b[i];
    atm->layer[i].T_b = T_b[i];
    if (L_b[i] == 0.0) {
      atm->layer[i].compPressure = comp_pressure_no_lapse;
      atm->layer[i].compDensity = comp_density_no_lapse;
    } else {
      atm->layer[i].compPressure = comp_pressure_lapse;
      atm->layer[i].compDensity = comp_density_lapse;
    }
  }

  return atm;
}

PLatmosphere*
pl_new_atmosphere(float sample_dist, float h, pl_atm_template_t *t)
{
  PLatmosphere *atm = smalloc(sizeof(PLatmosphere));
  float_array_init(&atm->P);
  float_array_init(&atm->p);

  size_t samples = (size_t) h / sample_dist;
  atm->sample_distance = sample_dist;
  for (size_t i = 0 ; i < samples ; i ++) {
    float P = pl_atm_template_compute_airpressure(t, i*sample_dist);
    float p = pl_atm_template_compute_airdensity(t, i*sample_dist);
    float_array_push(&atm->P, P);
    float_array_push(&atm->p, p);
  }
  return atm;
}

float
pl_atmosphere_pressure(const PLatmosphere *atm, float h)
{
  if (atm == NULL) return 0.0;
  if (h < 0.0) {
    float P0 = ARRAY_ELEM(atm->P, 0);
    float P1 = ARRAY_ELEM(atm->P, 1);
    float P = P0 - (P0-P1)/atm->sample_distance * fabsf(h);
    return P;
  } else if (h > atm->sample_distance * atm->P.length-1) {
    float P0 = ARRAY_ELEM(atm->P, atm->P.length-2);
    float P1 = ARRAY_ELEM(atm->P, atm->P.length-1);
    float P = fmaxf(P1+((P0-P1)/atm->sample_distance)*(h-atm->sample_distance *
                                                       atm->P.length-1), 0.0f);
    return P;
  } else {
    size_t idx = h / atm->sample_distance;
    float f = fmodf(h, atm->sample_distance) / atm->sample_distance;
    float P = ARRAY_ELEM(atm->P, idx) * (1.0f-f) +
              ARRAY_ELEM(atm->P, idx+1) * f;
    return P;
  }
}

float
pl_atmosphere_density(const PLatmosphere *atm, float h)
{
  if (atm == NULL) return 0.0;
  if (h < 0.0) {
    float p0 = ARRAY_ELEM(atm->p, 0);
    float p1 = ARRAY_ELEM(atm->p, 1);
    float p = p0 - (p0-p1)/atm->sample_distance * fabsf(h);
    return p;
  } else if (h > atm->sample_distance * atm->p.length-1) {
    float p0 = ARRAY_ELEM(atm->p, atm->p.length-2);
    float p1 = ARRAY_ELEM(atm->p, atm->p.length-1);
    float p = fmaxf(p1+((p0-p1)/atm->sample_distance)*(h-atm->sample_distance *
                                                       atm->p.length-1), 0.0f);
    return p;
  } else {
    size_t idx = h / atm->sample_distance;
    float f = fmodf(h, atm->sample_distance) / atm->sample_distance;
    float p = ARRAY_ELEM(atm->p, idx) * (1.0f-f) +
              ARRAY_ELEM(atm->p, idx+1) * f;
    return p;
  }
}
