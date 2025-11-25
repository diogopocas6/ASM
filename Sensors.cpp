#include "Sensors.h"

void sensors_begin() {
  pinMode(PIN_SENSOR_PECA, INPUT);      // ou INPUT_PULLUP consoante o sensor
  pinMode(PIN_SENSOR_GARRA, INPUT);
}

bool sensor_pieceDetected() {
  int v = digitalRead(PIN_SENSOR_PECA);
  // Ajustar HIGH/LOW dependendo do sensor (aqui assumo HIGH = peça presente)
  return (v == HIGH);
}

bool sensor_garraHasObject() {
  int v = digitalRead(PIN_SENSOR_GARRA);
  return (v == HIGH);  // HIGH = objeto dentro da garra
}
