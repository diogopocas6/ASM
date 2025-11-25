#pragma once
#include <Arduino.h>
#include "Pins.h"

// Struct com o estado dos dois sensores
struct SensorReadings {
  bool pieceAtPickup;   // há peça na zona de coleta?
  bool pieceAtDropoff;  // há peça na zona de entrega?
};

void sensors_begin();
SensorReadings sensors_read();

