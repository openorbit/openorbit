#include <stdbool.h>
#include "io-manager.h"
#include "sim/spacecraft.h"
#include "log.h"

void ooSimScCtrlMainEngineToggle(bool buttonUp, void *data);

void
ooSimScCtrlInit()
{
  ooIoRegCKeyHandler("sc-main-engine-toggle", ooSimScCtrlMainEngineToggle);
}

void
ooSimScCtrlMainEngineToggle(bool buttonUp, void *data)
{
  if (buttonUp) {
    OOspacecraft *sc = ooScGetCurrent();
    
    if (sc->mainEngine) {
      switch (sc->mainEngine->state) {
      case OO_Engine_Disabled:
        ooLogInfo("firing main engine");
        sc->mainEngine->state = OO_Engine_Enabled;
        break;
      case OO_Engine_Enabled:
        ooLogInfo("cutting main engine");
        sc->mainEngine->state = OO_Engine_Disabled;
        break;
      case OO_Engine_Fault:
        ooLogInfo("engine toggle on faulty engine");
        break;
      default:
        assert(0 && "invalid case");
      }
    }
  }
}
