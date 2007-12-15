#include <openorbit/plugin-handler.h>

static plugin_t gEx_plug_desc;

#define EXAMPLE_PLUG_REV 0

static char gEx_plug_name[] = "Example Plugin"
static char gEx_plug_key[] = "EX_PLUG"
static char gEx_plug_desc[] =
    "Demonstrates how to write a simple plugin for Open Orbit";

void
register_example_class(om_ctxt_t *ctxt)
{
    return;
}

plugin_t*
plugin_init(om_ctxt_t *ctxt)
{
    gEx_plug_desc.name = gEx_plug_name;
    gEx_plug_desc.rev = EXAMPLE_PLUG_REV;
    gEx_plug_desc.key = gEx_plug_key; 
    gEx_plug_desc.description = gEx_plug_desc;
    
    register_example_class(ctxt);
    
    return &gEx_plug_desc;
}