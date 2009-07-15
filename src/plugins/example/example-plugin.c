/*
  Copyright 2009 Mattias Holm <mattias.holm(at)openorbit.org>

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
#include <openorbit/openorbit.h>
#include <openorbit/ifaces/spacecraft-if.h>
#include <openorbit/ifaces/control-if.h>

OOpluginversion oopluginversion = OO_Plugin_Ver_1_00;
static OOplugin gEx_plug_desc;

#define EXAMPLE_PLUG_REV 0

static char gEx_plug_name[] = "Example Plugin";
static char gEx_plug_key[] = "EX_PLUG";
static char gEx_plug_doc[] =
    "Demonstrates how to write a simple plugin for Open Orbit";

typedef struct OOexamplesc {

} OOexamplesc;


void
register_example_class(OMcontext *ctxt)
{
  OMclass *cls = omNewClass(ctxt, "examplesc", sizeof(OOexamplesc));
  
}

OOplugin*
ooplugininit(OOplugincontext_v1 *ctxt)
{
    gEx_plug_desc.name = gEx_plug_name;
    gEx_plug_desc.rev = EXAMPLE_PLUG_REV;
    gEx_plug_desc.key = gEx_plug_key; 
    gEx_plug_desc.description = gEx_plug_doc;
    
    register_example_class(ctxt->objectManager);
    
    return &gEx_plug_desc;
}

void
oopluginfinalise(OOplugincontext_v1 *ctxt)
{
  
}

void
scStep(OOexamplesc *sc, float dt)
{
  
}
// These functions handle the enabling of actuators

void
aftThrusterFire(OOexamplesc *sc)
{
  
}

void
fwdThrusterFire(OOexamplesc *sc)
{
  
}

void
alphaRCSFire(OOexamplesc *sc)
{
  
}

void
betaRCSFire(OOexamplesc *sc)
{
  
}

void
gammaRCSFire(OOexamplesc *sc)
{
  
}
