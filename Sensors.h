#pragma once
#include <Arduino.h>
#include "Pins.h"

void sensors_begin();

// devolve true se há peça na posição de recolha
bool sensor_pieceDetected();

// devolve true se o sensor da garra deteta objeto
bool sensor_garraHasObject();


