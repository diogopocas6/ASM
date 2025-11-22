#pragma once
#include <Arduino.h>
#include <Servo.h>
#include "Pins.h"
#include "Config.h"

// Guardar os servos
typedef struct {
  Servo base;
  Servo shoulder;
  Servo elbow;
  Servo wrist;
  int baseAngle;
} arm_t;

void arm_begin(arm_t &arm);

// Posições pré-definidas (poses)
void arm_pose_standby(arm_t &arm);
void arm_pose_turn_pickup(arm_t &arm);
void arm_pose_pickup_down(arm_t &arm);
void arm_pose_close_claw(arm_t &arm);
void arm_pose_carry_up(arm_t &arm);
void arm_pose_turn_dropoff(arm_t &arm);
void arm_pose_carry_down(arm_t &arm);
void arm_pose_open_claw(arm_t &arm);

// Se precisares mexer só a base
void arm_set_base(arm_t &arm, int deg);

void fecharGarraSeObjeto() {
  int sensorValor = digitalRead(sensor_garraPin); // acho que n precida disto aqui

  // HIGH significa que o objeto está entre os dedos da garra
  if (sensorValor == HIGH) {
    // fecha suavemente
    for (int pos = servo_pos_4, pos >= 20, pos--) {   // 20 = posição fechada (exemplo)
      servo_pos_4 = pos;   
      myservo_4.write(pos);
      delay(10);  // velocidade da garra
    }

   
  }
void goToPose(const Pose& p, int pause_ms = 5) {  // funçao que faz o braço mudar de posiçao mudar de posiçao (alterar isto) 
  ptp_move(p.s1, p.s2, p.s3, p.s4, pause_ms);  
}
