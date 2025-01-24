#include <cstdint>

typedef struct Pump {
  const char *name;
  uint8_t pin;
  uint8_t currentState;
  uint32_t dutyCycleSeconds;
  uint32_t dutyCycleCount;
} Pump;

void inc(Pump *pump);
void updateState(Pump *pump, uint8_t active);
