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
void arm_pose_standby(arm_t &arm);        // posição "home" / espera
void arm_pose_turn_pickup(arm_t &arm);    // base vira para lado onde está a peça
void arm_pose_pickup_down(arm_t &arm);    // braço baixa até à peça
void arm_pose_close_claw(arm_t &arm);     // fecha a "garra"
void arm_pose_carry_up(arm_t &arm);       // levanta com peça
void arm_pose_turn_dropoff(arm_t &arm);   // base vira para zona de descarga
void arm_pose_carry_down(arm_t &arm);     // baixa para largar
void arm_pose_open_claw(arm_t &arm);      // abre garra (solta peça)

// Se precisar mexer só a base
void arm_set_base(arm_t &arm, int deg);
