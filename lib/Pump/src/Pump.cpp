#include "Pump.h"

void Pump::updateState(int state) {
  if (state) {
    inc(state);
    active = 1;
    return;
  }

  active = 0;
}


void Pump::inc(int sec) {
  usageSeconds += sec;

  if (!active) {
    usageCount++;
  }
}
