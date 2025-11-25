#pragma once

// Servos pins
#define PIN_SERVO_BASE   10
#define PIN_SERVO_SHOULDER 9
#define PIN_SERVO_ELBOW    6
#define PIN_SERVO_WRIST    5

//Sensors pins
#define PIN_SENSOR_PICKUP   A0  // sensor de objeto na zona de recolha
#define PIN_SENSOR_DROPOFF  A1  // sensor de objeto na zona de entrega

// Sensor de temperatura ambiente (analógico)
#define PIN_TEMP_SENSOR   A2
// Ventoinha controlada por PWM (ligada a um MOSFET/transístor)
#define PIN_FAN_PWM       3   //precisa ser pino PWM
