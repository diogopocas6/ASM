#include "Braco.h"

static void arm_write_all(arm_t &arm, int base, int sh, int el, int wr) {
  arm.baseAngle = base;
  arm.base.write(base); //base
  arm.shoulder.write(sh); //shoulder
  arm.elbow.write(el); //elbow
  arm.wrist.write(wr); //wrist
}

void arm_begin(arm_t &arm) {
  arm.base.attach(PIN_SERVO_BASE);
  arm.shoulder.attach(PIN_SERVO_SHOULDER);
  arm.elbow.attach(PIN_SERVO_ELBOW);
  arm.wrist.attach(PIN_SERVO_WRIST);

  arm_pose_standby(arm);
}

void arm_pose_standby(arm_t &arm) {
  arm_write_all(arm, BASE_CENTER_ANGLE, 90, 90, 90); //ajustar valores testando
}

void arm_pose_turn_pickup(arm_t &arm) {
  arm_write_all(arm, BASE_PICKUP_ANGLE, 90, 90, 90);  // Base para o lado de pickup
}

void arm_pose_pickup_down(arm_t &arm) {
  arm_write_all(arm, BASE_PICKUP_ANGLE, 120, 60, 120);  // Braço para baixo
}

void arm_pose_close_claw(arm_t &arm) {
  arm_write_all(arm, BASE_PICKUP_ANGLE, 120, 60, 80);  // Pegar objeto
}

void arm_pose_carry_up(arm_t &arm) {
  arm_write_all(arm, arm.baseAngle, 80, 120, 80);  // Levantar o braço mantendo a base
}

void arm_pose_turn_dropoff(arm_t &arm) {
  arm_write_all(arm, BASE_DROPOFF_ANGLE, 80, 120, 80);   // Base para lado de dropoff
}

void arm_pose_carry_down(arm_t &arm) {
  arm_write_all(arm, BASE_DROPOFF_ANGLE, 120, 60, 80);   // Braço para baixo
}

void arm_pose_open_claw(arm_t &arm) {
  arm_write_all(arm, BASE_DROPOFF_ANGLE, 120, 60, 120);   // Soltar objeto
}

void arm_set_base(arm_t &arm, int deg) {
  arm.baseAngle = constrain(deg, 0, 180);
  arm.base.write(arm.baseAngle);
}

