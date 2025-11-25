#pragma once
#include <Arduino.h>
#include "Pins.h"

// Estrutura com as leituras de ambos os sensores
typedef struct {
  bool pieceAtPickup;   // há peça na zona de coleta?
  bool pieceAtDropoff;  // há peça na zona de entrega?
} sensors_t;

// Inicializa os pinos dos sensores
void sensors_begin();

// Atualiza a struct de leituras
void sensors_read(sensors_t &s);

