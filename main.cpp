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

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Pins.h"
#include "Config.h"
#include "Braco.h"
#include "Sensors.h"

// ====== OLED 128x64 I2C ======
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====== BRAÇO ======
arm_t arm;

// ====== MÁQUINA DE ESTADOS ======
enum class State {
  IDLE,               // braço parado, à espera de peça
  TURN_TO_PICKUP,     // roda base para lado da coleta
  LOWER_TO_PICKUP,    // baixa braço na coleta
  CHECK_AND_CLOSE,    // verifica peça -> fecha garra
  LIFT_WITH_PIECE,    // levanta braço com peça
  TURN_TO_DROPOFF,    // roda base até local de entrega
  LOWER_TO_DROPOFF,   // baixa braço para largar
  OPEN_CLAW,          // abre garra
  LIFT_EMPTY,         // levanta braço vazio
  RETURN_HOME         // volta ao standby
};

State state = State::IDLE;
unsigned long stateEntryMs = 0;

// Helper para mudar de estado
void changeState(State newState) {
  state = newState;
  stateEntryMs = millis();
}

// Tempo passado no estado atual
unsigned long timeInState() {
  return millis() - stateEntryMs;
}

void drawOled(State st, const SensorReadings &sens) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Linha 0: estado
  display.setCursor(0, 0);
  display.print("Estado: ");
  switch (st) {
    case State::IDLE:             display.println("IDLE"); break;
    case State::TURN_TO_PICKUP:   display.println("TURN_PICK"); break;
    case State::LOWER_TO_PICKUP:  display.println("LOWER_PICK"); break;
    case State::CHECK_AND_CLOSE:  display.println("CHECK_CLOSE"); break;
    case State::LIFT_WITH_PIECE:  display.println("LIFT_PIECE"); break;
    case State::TURN_TO_DROPOFF:  display.println("TURN_DROP"); break;
    case State::LOWER_TO_DROPOFF: display.println("LOWER_DROP"); break;
    case State::OPEN_CLAW:        display.println("OPEN_CLAW"); break;
    case State::LIFT_EMPTY:       display.println("LIFT_EMPTY"); break;
    case State::RETURN_HOME:      display.println("RETURN_HOME"); break;
  }

  // Linha 1: sensor pickup
  display.setCursor(0, 12);
  display.print("Pickup: ");
  display.println(sens.pieceAtPickup ? "SIM" : "NAO");

  // Linha 2: sensor dropoff
  display.setCursor(0, 22);
  display.print("Dropoff: ");
  display.println(sens.pieceAtDropoff ? "SIM" : "NAO");

  display.display();
}

void loop() {
  // 1) Ler sensores
  SensorReadings sens = sensors_read();
  bool pieceDetected = sens.pieceAtPickup;
  bool delivered     = sens.pieceAtDropoff;

  // 2) Máquina de estados
  switch (state) {

    case State::IDLE:
      arm_pose_standby(arm);
      if (pieceDetected) {
        Serial.println("Peça detetada na coleta -> TURN_TO_PICKUP");
        changeState(State::TURN_TO_PICKUP);
      }
      break;

    case State::TURN_TO_PICKUP:
      arm_pose_turn_pickup(arm);          // base -> lado de coleta
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::LOWER_TO_PICKUP);
      }
      break;

    case State::LOWER_TO_PICKUP:
      arm_pose_pickup_down(arm);          // baixa braço
      if (timeInState() > SERVO_SETTLE_MS) {
        // opcional: confirmar que peça ainda está na zona de pickup
        if (pieceDetected) {
          changeState(State::CHECK_AND_CLOSE);
        } else {
          Serial.println("Peça desapareceu -> RETURN_HOME");
          changeState(State::RETURN_HOME);
        }
      }
      break;

    case State::CHECK_AND_CLOSE:
      // aqui poderias usar um sensor na garra, mas por enquanto:
      Serial.println("Fechar garra (pose_close_claw)");
      arm_pose_close_claw(arm);
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::LIFT_WITH_PIECE);
      }
      break;

    case State::LIFT_WITH_PIECE:
      arm_pose_carry_up(arm);             // levanta braço com peça
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::TURN_TO_DROPOFF);
      }
      break;

    case State::TURN_TO_DROPOFF:
      arm_pose_turn_dropoff(arm);         // base -> lado de entrega
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::LOWER_TO_DROPOFF);
      }
      break;

    case State::LOWER_TO_DROPOFF:
      arm_pose_carry_down(arm);           // baixa braço para largar
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::OPEN_CLAW);
      }
      break;

    case State::OPEN_CLAW:
      arm_pose_open_claw(arm);            // abre garra (solta peça)
      if (timeInState() > RELEASE_MS) {
        if (delivered) {
          Serial.println("Entrega confirmada (sensor dropoff)");
        } else {
          Serial.println("AVISO: sensor dropoff nao viu peça");
        }
        changeState(State::LIFT_EMPTY);
      }
      break;

    case State::LIFT_EMPTY:
      arm_pose_carry_up(arm);             // levanta sem peça
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::RETURN_HOME);
      }
      break;

    case State::RETURN_HOME:
      arm_pose_standby(arm);              // volta ao centro
      if (timeInState() > SERVO_SETTLE_MS) {
        changeState(State::IDLE);
      }
      break;
  }

  // 3) Atualizar OLED
  drawOled(state, sens);

  // Pequena pausa (FSM continua baseada em millis)
  delay(10);
}

