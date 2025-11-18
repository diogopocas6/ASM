#include "Sensors.h"

void sensors_begin() {
  pinMode(PIN_OBJ_SENSOR, INPUT);
  pinMode(PIN_DROP_SENSOR, INPUT);
}

void sensors_update(sensors_t &s) {
  // Sensores infrared tipo X
  int o = analogRead(PIN_OBJ_SENSOR);
  int d = analogRead(PIN_DROP_SENSOR);

  // Thresholds para "1 = detectado" --> ajustar pro sensor infrared usado
  s.obj_detected  = (o < 400);
  s.drop_detected = (d < 400);
}
