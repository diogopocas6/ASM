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

