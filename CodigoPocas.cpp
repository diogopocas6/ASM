#include <Servo.h>
#define PinBase 10
#define PinShoulder 9
#define PinElbow 6
#define PinGarra 5 
#define PinVentoinha 12 //mudar depois
#define PinSensorPickup A0
#define PinSensorPlace 13 //mudar depois
#define PinSensorTemperatura A2 //Secalhar alterar estes números
#define activateValueSensorPlace 100   //MUDAR CONFORME TESTES!
#define activateValueSensorPick 100    //MUDAR CONFORME TESTES!

/*
Posições segundo código enzo:
Begin -> Base = 90 / Shoulder = 0 / Elbow = 160 / Garra = 140(fechada)
*/
// Ângulos
#define BASE_CENTRO 90
#define BASE_PICKUP 180
#define BASE_DROPOFF 0

#define ELBOW_UP 160
#define ELBOW_DOWN 110

#define SHOULDER_START 0
#define SHOULDER_END 80

#define GARRA_OPEN 30
#define GARRA_CLOSE 140


Servo ServoBase;
Servo ServoShoulder;
Servo ServoElbow;
Servo ServoGarra;

typedef struct {
  int value;       // leitura atual 
  int prev_value;  // leitura anterior
  bool re;          // rising edge: 0 -> 1, apenas acontece se ativo = 1 e prev_ativo = 0
  bool ativo;       // se value > activateValueSensor, ativo = 1 else ativo = 0
  bool prev_ativo;
} Sensor;

Sensor SensorPlace,SensorPickup;
int temperatura = 0;
int PWMventoinha = 0;
void setup() {
  pinMode(PinVentoinha,OUTPUT);
  pinMode(PinSensorPickup,INPUT);       // VER SE É PRECISO POR INPUT_PULLUP
  pinMode(PinSensorPlace,INPUT);        // VER SE É PRECISO POR INPUT_PULLUP
  pinMode(PinSensorTemperatura,INPUT);  // VER SE É PRECISO POR INPUT_PULLUP
  ServoBase.attach(PinBase);            //ServoBase.write(x) para mover para a posiçao x graus 
  ServoShoulder.attach(PinShoulder);    //ServoShoulder.write(x) para mover para a posiçao x graus
  ServoElbow.attach(PinElbow);
  ServoGarra.attach(PinGarra);
  
  //Posições iniciais
  ServoBase.write(90);
  ServoShoulder.write(0);
  ServoElbow.write(160);
  ServoGarra.write(140);
}

//Funções para controlo dos servos
//GARRA
void openGarra(){
  for (int pos = GARRA_CLOSE; pos >= GARRA_OPEN; pos--){
    ServoGarra.write(pos);
    delay(20);
  }
}

void closeGarra(){
  for (int pos = GARRA_OPEN; pos <= GARRA_CLOSE; pos++){
    ServoGarra.write(pos);
    delay(20);
  }
}

void updateSensors(){
// Sensor Temperatura
  temperatura = analogRead(PinSensorTemperatura);

  // SensorPlace
  SensorPlace.prev_ativo = SensorPlace.ativo;
  SensorPlace.prev_value = SensorPlace.value;
  SensorPlace.value = analogRead(PinSensorPlace);
  SensorPlace.ativo = (SensorPlace.value > activateValueSensorPlace);   // DEFINIR activateValueSensorPlace no define
  SensorPlace.re = (!SensorPlace.prev_ativo && SensorPlace.ativo);  // re ativa quando passamos de não ativo para ativo

  // SensorPickup
  SensorPickup.prev_ativo = SensorPickup.ativo;
  SensorPickup.prev_value = SensorPickup.value;
  SensorPickup.value = analogRead(PinSensorPickup);
  SensorPickup.ativo = (SensorPickup.value > activateValueSensorPick);   // DEFINIR activateValueSensorPick no define
  SensorPickup.re = (!SensorPickup.prev_ativo && SensorPickup.ativo); // re ativa quando passamos de não ativo para ativo
}

void controlaVentoinha(int temperatura){
  PWMventoinha = map(temperatura, 0, 1023, 0, 255);
  analogWrite(PinVentoinha,PWMventoinha);
}

void loop() {
  updateSensors();
  controlaVentoinha(temperatura);

}
