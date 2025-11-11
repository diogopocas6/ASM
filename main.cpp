#include <Arduino.h>
#include <Wire.h>
#include "Pins.h"
#include "Types.h"
#include "Config.h"

#include "Braço.h"
#include "Controlo_Base.h"
#include "Sensors.h"
#include "Display.h"
#include "Maq_Estados.h"


// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
