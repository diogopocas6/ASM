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
  int pieceDetected = digitalRead(sensorPin); // detetor das peças
  int sensorValor = digitalRead(sensor_garraPin); // sensor da garra

  switch (state) {

    case State::IDLE:
      if (pieceDetected) {
        Serial.println("Peça detetada! A mover para PICK...");
        state = State::MOVE_TO_PIECE; //vai se mover para onde a peça está
      }
      break;

    case State::MOVE_TO_PIECE:
        goToPose(PICK, 5);     // funçao que move o braço(tem de ser alterada)
        state = State::FECHAR_GARRA; // vai verificar se a peça esta mesmo la 
        break;
    case State::FECHAR_GARRA:
         if (sensorValor == HIGH) { 
            Serial.println("A fechar garra...");
            fecharGarraSeObjeto();
            Serial.println("Garra fechada. Programa continua...");
            state = State::POS_DESCARGA;     // vai para a posiçao de descarga
        }
        else {
            Serial.println("Peça desapareceu! Voltar HOME.");
            state = State::VOLTAR_HOME; // vai voltar para a posiçao inicial
        }
        break;

    case State::VOLTAR_HOME:
        goToPose(HOME, 5);
        Serial.println("De volta ao HOME.");
        state = State::IDLE;
        break;
 
 }
  }
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
