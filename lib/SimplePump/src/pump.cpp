#include "pump.h"

// updates the state of a pump
void updateState(Pump *pump, uint8_t active) {
  if (active) {
    inc(pump);
  }
  pump->currentState = active;
}

// increments pump counters
void inc(Pump *pump) {
  // increment runtimeCount if pump was not previously active
  if (!pump->currentState) {
    pump->dutyCycleCount++;
  }

  // increment pump runtimeSeconds
  pump->dutyCycleSeconds++;
}
