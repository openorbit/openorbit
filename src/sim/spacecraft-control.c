#include <stdbool.h>
#include "io-manager.h"
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
    
  }
}
