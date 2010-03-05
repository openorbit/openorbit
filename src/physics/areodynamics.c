/*
  Copyright 2009,2010 Mattias Holm <mattias.holm(at)openorbit.org>

  This file is part of Open Orbit.

  Open Orbit is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Open Orbit is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "physics.h"
#include "areodynamics.h"
#include <vmath/vmath.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <gencds/hashtable.h>
// Note that these are molar weights for all elements, this does not take into
// account that some elements appear in molecules (i.e. H2)
// TODO: Set all elements that are -0.0 to NAN, but this cannot be done until
//       there is a new official release of clang for OS X
static double elementMolarWeights[PL_Elem_Last] = {
  [PL_Elem_n] = -0.0, // Neutron, but this is mosly here to be able to index by
                      // proton count
  [PL_Elem_H] = 0.00100794, [PL_Elem_He] = 0.004002602,

  // Period 2
  [PL_Elem_Li] = 0.006941,     [PL_Elem_Be] = 0.009012,
  [PL_Elem_B]  = 0.010811,     [PL_Elem_C]  = 0.0120107,
  [PL_Elem_N]  = 0.0140067,    [PL_Elem_O]  = 0.0159994,
  [PL_Elem_F]  = 0.0189984032, [PL_Elem_Ne] = 0.0201797,

  // Period 3
  [PL_Elem_Na] = 0.02298977,  [PL_Elem_Mg] = 0.0243050,
  [PL_Elem_Al] = 0.026981538, [PL_Elem_Si] = 0.0280855,
  [PL_Elem_P]  = 0.030973761, [PL_Elem_S]  = 0.032066,
  [PL_Elem_Cl] = 0.0354527,   [PL_Elem_Ar] = 0.039948,

  // Period 4
  [PL_Elem_K]  = 0.0390983,   [PL_Elem_Ca] = 0.040078,
  [PL_Elem_Sc] = 0.044955910, [PL_Elem_Ti] = 0.047867,
  [PL_Elem_V]  = 0.0509415,   [PL_Elem_Cr] = 0.0519961,
  [PL_Elem_Mn] = 0.054938049, [PL_Elem_Fe] = 0.055845,
  [PL_Elem_Co] = 0.058933200, [PL_Elem_Ni] = 0.0586934,
  [PL_Elem_Cu] = 0.063546,    [PL_Elem_Zn] = 0.06539,
  [PL_Elem_Ga] = 0.069723,    [PL_Elem_Ge] = 0.07261,
  [PL_Elem_As] = 0.07492160,  [PL_Elem_Se] = 0.07896,
  [PL_Elem_Br] = 0.079904,    [PL_Elem_Kr] = 0.08380,

  // Period 5
  [PL_Elem_Rb] = 0.0854678,  [PL_Elem_Sr] = 0.08762,
  [PL_Elem_Y]  = 0.088905,   [PL_Elem_Zr] = 0.091224,
  [PL_Elem_Nb] = 0.09290638, [PL_Elem_Mo] = 0.09594,
  [PL_Elem_Tc] = -0.0,       [PL_Elem_Ru] = 0.10107,
  [PL_Elem_Rh] = 0.10290550, [PL_Elem_Pd] = 0.10642,
  [PL_Elem_Ag] = 0.1078682,  [PL_Elem_Cd] = 0.112411,
  [PL_Elem_In] = 0.114818,   [PL_Elem_Sn] = 0.118710,
  [PL_Elem_Sb] = 0.121760,   [PL_Elem_Te] = 0.12760,
  [PL_Elem_I]  = 0.12690447, [PL_Elem_Xe] = 0.13129,

  // Period 6
  [PL_Elem_Cs] = 0.13290545, [PL_Elem_Ba] = 0.137327,

  // Lanthanoids
  [PL_Elem_La] = 0.1389055,  [PL_Elem_Ce] = 0.140116,
  [PL_Elem_Pr] = 0.14090765, [PL_Elem_Nd] = 0.14424,
  [PL_Elem_Pm] = -0.0,       [PL_Elem_Sm] = 0.15036,
  [PL_Elem_Eu] = 0.151964,   [PL_Elem_Gd] = 0.15725,
  [PL_Elem_Tb] = 0.15892534, [PL_Elem_Dy] = 0.16250,
  [PL_Elem_Ho] = 0.16493032, [PL_Elem_Er] = 0.1676,
  [PL_Elem_Tm] = 0.16893421, [PL_Elem_Yb] = 0.17304,
  [PL_Elem_Lu] = 0.174967,

  [PL_Elem_Hf] = 0.17849,  [PL_Elem_Ta] = 0.1809479,
  [PL_Elem_W]  = 0.18384,  [PL_Elem_Re] = 0.186207,
  [PL_Elem_Os] = 0.19023,  [PL_Elem_Ir] = 0.192217,
  [PL_Elem_Pt] = 0.195078, [PL_Elem_Au] = 0.19696655,
  [PL_Elem_Hg] = 0.20059,  [PL_Elem_Tl] = 0.2043833,
  [PL_Elem_Pb] = 0.2072,   [PL_Elem_Bi] = 0.20898038,
  [PL_Elem_Po] = -0.0,     [PL_Elem_At] = -0.0,
  [PL_Elem_Rn] = -0.0,

  // Period 7
  [PL_Elem_Fr] = -0.0, [PL_Elem_Ra] = -0.0,

  // Actinoids
  [PL_Elem_Ac] = -0.0,       [PL_Elem_Th] = 0.2320381,
  [PL_Elem_Pa] = 0.23103588, [PL_Elem_U]  = 0.2380289,
  [PL_Elem_Np] = -0.0,       [PL_Elem_Pu] = -0.0,
  [PL_Elem_Am] = -0.0,       [PL_Elem_Cm] = -0.0,
  [PL_Elem_Bk] = -0.0,       [PL_Elem_Cf] = -0.0,
  [PL_Elem_Es] = -0.0,       [PL_Elem_Fm] = -0.0,
  [PL_Elem_Md] = -0.0,       [PL_Elem_No] = -0.0,
  [PL_Elem_Lr] = -0.0,

  [PL_Elem_Rf]  = -0.0, [PL_Elem_Db]  = -0.0,
  [PL_Elem_Sg]  = -0.0, [PL_Elem_Bh]  = -0.0,
  [PL_Elem_Hs]  = -0.0, [PL_Elem_Mt]  = -0.0,
  [PL_Elem_Ds]  = -0.0, [PL_Elem_Rg]  = -0.0,
  [PL_Elem_Cn]  = -0.0, [PL_Elem_Uut] = -0.0,
  [PL_Elem_Uuq] = -0.0, [PL_Elem_Uup] = -0.0,
  [PL_Elem_Uuh] = -0.0, [PL_Elem_Uus] = -0.0,
  [PL_Elem_Uuo] = -0.0,
};

static char *elemNames[PL_Elem_Last] = {
  [PL_Elem_n] = "neutron",
  [PL_Elem_H] = "hydrogen", [PL_Elem_He] = "helium",

  // Period 2
  [PL_Elem_Li] = "lithium",     [PL_Elem_Be] = "beryllium",
  [PL_Elem_B]  = "boron",     [PL_Elem_C]  = "carbon",
  [PL_Elem_N]  = "nitrogen",    [PL_Elem_O]  = "oxygen",
  [PL_Elem_F]  = "flourine", [PL_Elem_Ne] = "neon",

  // Period 3
  [PL_Elem_Na] = "sodium",  [PL_Elem_Mg] = "magnesium",
  [PL_Elem_Al] = "aluminium", [PL_Elem_Si] = "silicon",
  [PL_Elem_P]  = "phosphorus", [PL_Elem_S]  = "sulfur",
  [PL_Elem_Cl] = "chlorine",   [PL_Elem_Ar] = "argon",

  // Period 4
  [PL_Elem_K]  = "potassium",   [PL_Elem_Ca] = "calcium",
  [PL_Elem_Sc] = "scandium", [PL_Elem_Ti] = "titanium",
  [PL_Elem_V]  = "vanadium",   [PL_Elem_Cr] = "chromium",
  [PL_Elem_Mn] = "manganese", [PL_Elem_Fe] = "iron",
  [PL_Elem_Co] = "cobalt", [PL_Elem_Ni] = "nickel",
  [PL_Elem_Cu] = "copper",    [PL_Elem_Zn] = "zinc",
  [PL_Elem_Ga] = "gallium",    [PL_Elem_Ge] = "germanium",
  [PL_Elem_As] = "arsenic",  [PL_Elem_Se] = "selenium",
  [PL_Elem_Br] = "bromine",    [PL_Elem_Kr] = "krypton",

  // Period 5
  [PL_Elem_Rb] = "rubidium",  [PL_Elem_Sr] = "strontium",
  [PL_Elem_Y]  = "yttrium",   [PL_Elem_Zr] = "zirconium",
  [PL_Elem_Nb] = "niobium", [PL_Elem_Mo] = "molybdenum",
  [PL_Elem_Tc] = "technetium",       [PL_Elem_Ru] = "ruthenium",
  [PL_Elem_Rh] = "rhodium", [PL_Elem_Pd] = "palladium",
  [PL_Elem_Ag] = "silver",  [PL_Elem_Cd] = "cadmium",
  [PL_Elem_In] = "indium",   [PL_Elem_Sn] = "tin",
  [PL_Elem_Sb] = "antimony",   [PL_Elem_Te] = "tellurium",
  [PL_Elem_I]  = "iodine", [PL_Elem_Xe] = "xenon",

  // Period 6
  [PL_Elem_Cs] = "cesium", [PL_Elem_Ba] = "barium",

  // Lanthanoids
  [PL_Elem_La] = "lanthanum",  [PL_Elem_Ce] = "cerium",
  [PL_Elem_Pr] = "praseodymium", [PL_Elem_Nd] = "neodymium",
  [PL_Elem_Pm] = "promethium",       [PL_Elem_Sm] = "samarium",
  [PL_Elem_Eu] = "europium",   [PL_Elem_Gd] = "gadolinium",
  [PL_Elem_Tb] = "terbium", [PL_Elem_Dy] = "dysprosium",
  [PL_Elem_Ho] = "holmium", [PL_Elem_Er] = "erbium",
  [PL_Elem_Tm] = "thulium", [PL_Elem_Yb] = "ytterbium",
  [PL_Elem_Lu] = "lutecium",

  [PL_Elem_Hf] = "hafnium",  [PL_Elem_Ta] = "tantalum",
  [PL_Elem_W]  = "tungsten",  [PL_Elem_Re] = "rhenium",
  [PL_Elem_Os] = "osmium",  [PL_Elem_Ir] = "iridium",
  [PL_Elem_Pt] = "platinum", [PL_Elem_Au] = "gold",
  [PL_Elem_Hg] = "mercury",  [PL_Elem_Tl] = "thallium",
  [PL_Elem_Pb] = "lead",   [PL_Elem_Bi] = "bismuth",
  [PL_Elem_Po] = "polonium",     [PL_Elem_At] = "astatine",
  [PL_Elem_Rn] = "radon",

  // Period 7
  [PL_Elem_Fr] = "francium", [PL_Elem_Ra] = "radium",

  // Actinoids
  [PL_Elem_Ac] = "actinium",       [PL_Elem_Th] = "thorium",
  [PL_Elem_Pa] = "protactinium", [PL_Elem_U]  = "uranium",
  [PL_Elem_Np] = "neptunium",       [PL_Elem_Pu] = "plutonium",
  [PL_Elem_Am] = "americium",       [PL_Elem_Cm] = "curium",
  [PL_Elem_Bk] = "berkelium",       [PL_Elem_Cf] = "californium",
  [PL_Elem_Es] = "einsteinium",       [PL_Elem_Fm] = "fermium",
  [PL_Elem_Md] = "mendelevium",       [PL_Elem_No] = "nobelium",
  [PL_Elem_Lr] = "lawrencium",

  [PL_Elem_Rf]  = "rutherfordium", [PL_Elem_Db]  = "dubnium",
  [PL_Elem_Sg]  = "seaborgium", [PL_Elem_Bh]  = "bohrium",
  [PL_Elem_Hs]  = "hassium", [PL_Elem_Mt]  = "meitnerium",
  [PL_Elem_Ds]  = "darmstadtium", [PL_Elem_Rg]  = "aoentgenium",
  [PL_Elem_Cn]  = "copernicium", [PL_Elem_Uut] = "ununtrium",
  [PL_Elem_Uuq] = "ununquadium", [PL_Elem_Uup] = "ununpentium",
  [PL_Elem_Uuh] = "ununhexium", [PL_Elem_Uus] = "ununseptium",
  [PL_Elem_Uuo] = "ununoctium",
};

static hashtable_t *ht;

void
plElementInit(void)
{
  ht = hashtable_new_with_str_keys(128);

  for (PLelement i = PL_Elem_First ; i < PL_Elem_Last ; ++ i) {
    hashtable_insert(ht, elemNames[i], (void*)i);
  }
}

void
plInit(void)
{
  plElementInit();
}

PLelement
plElementIdFromName(const char *name)
{
  char nameCopy[strlen(name)+1];

  for (int i = 0; i < strlen(name)+1 ; ++ i) {
    nameCopy[i] = tolower(name[i]); // Will pass null marker unchanged
  }

  PLelement i = (PLelement) hashtable_lookup(ht, nameCopy);
  return i;
}

const char*
plElementNameFromId(PLelement elem)
{
  assert(elem >= PL_Elem_First);
  assert(elem < PL_Elem_Last);

  return elemNames[elem];
}

double
plMoleculeWeight(unsigned nAtoms, PLelement elem)
{
  return (double)nAtoms * elementMolarWeights[elem];
}


double
plCompIndividualGasConst(double M)
{
  return PL_UGC / M;
}

double
plComputeDensity(double molWeight, double P, double T)
{
  return (molWeight*P) / (PL_UGC * T);
}
double
plEstimateTemp(double luminocity, double albedo, double R)
{
  // Quad root
  double t = pow((luminocity*(1.0 - albedo))/(4.0 * M_PI * PL_ST*R*R), 1.0/4.0);
  return t;
}
// Computes the current airspeed of the object
double
plComputeAirspeed(PLobject *obj)
{
  
}
// Compute drag force from
// \param v Velocity of object relative to the fluid
// \param p Density of fluid
// \param Cd Drag coeficient for object
// \param A Area of object (i.e. the orthographic projection area)
// \result A vector with the drag force (oposite direction of velocity)
float3
plComputeDrag(float3 v, double p, double Cd, double A)
{
  double v_mag = vf3_abs(v);
  double fd = 0.5 * p * v_mag * v_mag * Cd * A;
  float3 vf_dir = vf3_neg(vf3_normalise(v));
  return vf3_s_mul(vf_dir, fd);
}

float3
plComputeDragForObject(PLobject *obj)
{
  double p = plComputeAirdensity(obj);
  float3 vel = obj->v; // TODO: Adjust for planet rotation

  float3 drag = plComputeDrag(vel, p, obj->dragCoef, obj->area);
  return drag;
}

double
plComputeAirpressure(PLobject *obj)
{
  PLsystem *sys = obj->sys;
  PLatmosphere *atm = &obj->sys->orbitalBody->atm;
  float3 dist = ooLwcDist(&obj->p, &sys->orbitalBody->obj.p);
  float g0 = sys->orbitalBody->GM / (sys->orbitalBody->eqRad * sys->orbitalBody->eqRad); // TODO: Cache g0
  float h = vf3_abs(dist) - sys->orbitalBody->eqRad; // TODO: adjust for oblateness 
  double pressure = plPressureAtAltitude(atm->Pb, atm->Tb, g0, atm->M, h, 0.0);
  return pressure;
}

double
plComputeAirdensity(PLobject *obj)
{
  PLsystem *sys = obj->sys;
  PLatmosphere *atm = &obj->sys->orbitalBody->atm;
  float3 dist = ooLwcDist(&obj->p, &sys->orbitalBody->obj.p);
  float g0 = sys->orbitalBody->GM / (sys->orbitalBody->eqRad * sys->orbitalBody->eqRad); // TODO: Cache g0
  float h = vf3_abs(dist) - sys->orbitalBody->eqRad; // TODO: adjust for oblateness 
  double density = plPressureAtAltitude(atm->pb, atm->Tb, g0, atm->M, h, 0.0);
  return density;
}



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
plPressureAtAltitudeWithLapse(double Pb, double Tb, double g0, double M,
                              double h, double hb, double Lb)
{
  return Pb * pow(Tb/(Tb+Lb*(h-hb)), g0*M / (PL_UGC*Lb));
}


double
plPressureAtAltitude(double Pb, double Tb, double g0, double M, double h,
                     double hb)
{
  return Pb * exp((-g0 * M * (h-hb)) / (PL_UGC * Tb));
}
