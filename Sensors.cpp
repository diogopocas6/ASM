#include "Sensors.h"

void sensors_begin() {
  pinMode(PIN_SENSOR_PECA, INPUT);    
  pinMode(PIN_SENSOR_GARRA, INPUT);
}

SensorReadings sensors_read() {
  SensorReadings s;

  // Exemplo para sensores analógicos tipo TCRT5000
  int vPickup  = analogRead(PIN_SENSOR_PICKUP);
  int vDropoff = analogRead(PIN_SENSOR_DROPOFF);

  const int THRESH = 400;  // < 400 ≈ objeto detetado --> testar com sensor e modificar

  s.pieceAtPickup  = (vPickup  < THRESH);
  s.pieceAtDropoff = (vDropoff < THRESH);

  // Se forem digitais (saída 0/1), era só:
  // s.pieceAtPickup  = (digitalRead(PIN_SENSOR_PICKUP)  == HIGH);
  // s.pieceAtDropoff = (digitalRead(PIN_SENSOR_DROPOFF) == HIGH);

  return s;
}
