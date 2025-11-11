#include <Servo.h>

Servo myServo;  // create servo object

void setup() {
  myServo.attach(9);  // servo signal pin connected to D10
}

void loop() {
  myServo.write(0);    // move to 0 degrees
  delay(1000);
  myServo.write(45);   // move to 90 degrees
  delay(1000);
  myServo.write(90);  // move to 180 degrees
  delay(1000);
  myservo.write(60);
  delay(500);
}
