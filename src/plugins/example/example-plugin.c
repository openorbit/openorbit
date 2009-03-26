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
