// Integração: Braço com TCS34725 + OLED SSD1306 + Controlo ventoinha via L9110 (A0 -> PWM)
// Mantém a rotina do braço praticamente igual ao original, e mostra estado + temp + pwm no OLED.

// Bibliotecas
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =======================
// OLED 128x64 (I2C)
// =======================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const uint8_t OLED_ADDRESS = 0x3C; // usualmente 0x3C

// =======================
// Configuração dos servos
// =======================

Servo base;
Servo shoulder;
Servo elbow;
Servo wrist;

int BASE_CENTER_ANGLE  = 90;
int BASE_PICKUP_ANGLE  = 180;
int BASE_DROPOFF_ANGLE = 0;

#define PIN_SERVO_BASE      10
#define PIN_SERVO_SHOULDER   9
#define PIN_SERVO_ELBOW      6
#define PIN_SERVO_WRIST      5

// =======================
// Sensores & IO
// =======================

#define PIN_IR_DROPOFF      12

// Ventoinha (L9110)
#define INA_PIN 2   // PWM destination (IN1)
#define INB_PIN 3   // other input (keep LOW for fixed direction)

// Sensor temperatura
#define PIN_TEMP_AIN A0

// =======================
// Sensor de cor TCS34725
// =======================
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

bool pecaBrancaPresente = false;

// =======================
// Parâmetros de leitura / ventoinha
// =======================
const int ADC_MAX = 1023;
const int NUM_SAMPLES_TEMP = 8;
const int SAMPLE_DELAY_MS = 6;
const int PWM_MAX = 255;
const int MIN_PWM_TO_SPIN = 60; // evita trepidações; ajusta conforme necessário

// Estados legíveis para o display
String stateText = "INIT";

// Protótipos
bool ehBranco(float r, float g, float b);
bool pecaNoDropoff();
bool dropoffLivre();
void executarCicloBraco();
int readTempAvg();
void updateDisplay(const String &state, int tempRaw, int pwm);

// =======================
// Setup
// =======================
void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  // I2C iniciada
  Wire.begin();

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 not found!");
    // Se falhar, travar para não executar o resto sem display
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Inicializando...");
  display.display();

  // TCS34725
  if (!tcs.begin()) {
    Serial.println("ERRO: TCS34725 NAO encontrado!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Erro: TCS34725 nao");
    display.println("encontrado!");
    display.display();
    while (1) { }
  }

  // Servos
  elbow.attach(PIN_SERVO_ELBOW);
  base.attach(PIN_SERVO_BASE);
  shoulder.attach(PIN_SERVO_SHOULDER);
  wrist.attach(PIN_SERVO_WRIST);

  base.write(BASE_CENTER_ANGLE);
  shoulder.write(0);
  elbow.write(160);
  wrist.write(140);

  // IR dropoff
  pinMode(PIN_IR_DROPOFF, INPUT);

  // Ventoinha pins
  pinMode(INA_PIN, OUTPUT);
  pinMode(INB_PIN, OUTPUT);
  digitalWrite(INA_PIN, LOW);
  digitalWrite(INB_PIN, LOW);

  // Estado inicial no display
  stateText = "STANDBY";
  updateDisplay(stateText, 0, 0);
  delay(500);
  Serial.println("Sistema iniciado.");
}

// =======================
// Loop principal
// =======================
void loop() {
  // 1) Controlo da ventoinha (sempre)
  int tempRaw = readTempAvg();
  int pwm = map(tempRaw, 0, ADC_MAX, 0, PWM_MAX);
  if (pwm < MIN_PWM_TO_SPIN) pwm = MIN_PWM_TO_SPIN;
  // direção fixa: PWM em INA_PIN, INB LOW (ou ao contrário se preferires)
  digitalWrite(INB_PIN, LOW);
  analogWrite(INA_PIN, pwm);

  // 2) Mostrar estado / temp / pwm no display
  // Se estivermos no ciclo do braço, executarCicloBraco irá actualizar o estado também
  updateDisplay("STANDBY", tempRaw, pwm);

  // 3) Lógica do braço: só começa quando dropoff livre
  if (!dropoffLivre()) {
    // Se ocupado, mostra no ecrã e aguarda
    stateText = "DROPOFF OCUPADO";
    updateDisplay(stateText, tempRaw, pwm);
    pecaBrancaPresente = false;
    delay(300);
    return;
  }

  // 4) Ler sensor de cor
  float red, green, blue;
  tcs.setInterrupt(false);
  delay(60);
  tcs.getRGB(&red, &green, &blue);
  tcs.setInterrupt(true);

  bool brancaAgora = ehBranco(red, green, blue);

  // Actualizar display com informação rápida
  if (brancaAgora && !pecaBrancaPresente) {
    stateText = "PECA BRANCA DETECTADA";
    updateDisplay(stateText, tempRaw, pwm);
  } else if (!brancaAgora && pecaBrancaPresente) {
    stateText = "PECA REMOVIDA";
    updateDisplay(stateText, tempRaw, pwm);
  }

  // 5) Transição: peça branca apareceu e dropoff livre
  if (brancaAgora && !pecaBrancaPresente) {
    executarCicloBraco();
  }

  pecaBrancaPresente = brancaAgora;
  delay(100);
}

// =======================
// Funções utilitárias
// =======================

bool ehBranco(float r, float g, float b) {
  const float limiarBrilho = 115;
  const float maxDesbalanceamento = 1.35;
  if (r < limiarBrilho || g < limiarBrilho || b < limiarBrilho) return false;
  float maxv = max(r, max(g, b));
  float minv = min(r, min(g, b));
  if (maxv / minv > maxDesbalanceamento) return false;
  return true;
}

bool pecaNoDropoff() {
  int estado = digitalRead(PIN_IR_DROPOFF);
  return (estado == LOW);
}

bool dropoffLivre() {
  return !pecaNoDropoff();
}

// Leitura média do sensor de temperatura (A0)
int readTempAvg() {
  unsigned long sum = 0;
  for (int i = 0; i < NUM_SAMPLES_TEMP; i++) {
    sum += analogRead(PIN_TEMP_AIN);
    delay(SAMPLE_DELAY_MS);
  }
  return (int)(sum / NUM_SAMPLES_TEMP);
}

// Atualiza OLED com estado, temp raw e pwm
void updateDisplay(const String &state, int tempRaw, int pwm) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("ROBO: BrasArmSys");
  display.setCursor(0,12);
  display.print("Estado: ");
  display.println(state);
  display.setCursor(0, 26);
  display.print("TempRaw: ");
  display.print(tempRaw);
  display.setCursor(0, 36);
  display.print("PWM: ");
  display.print(pwm);
  // opcional: mostra cor lida (valores inteiros)
  display.setCursor(0, 48);
  display.print("IR Drop: ");
  display.print( pecaNoDropoff() ? "PRES" : "LIVRE" );
  display.display();
}

// =======================
// Ciclo completo do braço (bloqueante como no teu código original)
// =======================
void executarCicloBraco() {
  // Indicar inicio
  bool waitingDrop = false; 
  updateDisplay("CICLO: GIRAR->PICKUP", readTempAvg(), 0);
  delay(200);

  // girar pra pickup
  for (int pos = BASE_CENTER_ANGLE; pos <= BASE_PICKUP_ANGLE; pos++) {
    base.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: PICKUP", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // descer garra
  for (int pos = 160; pos >= 110; pos--) {
    elbow.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: DOWN", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // abrir garra
  for (int pos = 150; pos >= 70; pos--) {
    wrist.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: ABRIR GARRA", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(2000);

  // estende braço
  for (int pos = 0; pos <= 80; pos++) {
    shoulder.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: ESTENDE", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // fechar garra (pega peça)
  for (int pos = 70; pos <= 150; pos++) {
    wrist.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: FECHAR GARRA", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(2000);

  // volta braço (com peça)
  for (int pos = 80; pos >= 0; pos--) {
    shoulder.write(pos);
    delay(10);
  }
  updateDisplay("CICLO: RECOLHE", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(5);

  // gira base pra dropoff
  for (int pos = BASE_PICKUP_ANGLE; pos >= BASE_DROPOFF_ANGLE; pos--) {
    base.write(pos);
    if (pecaNoDropoff() && !waitingDrop) {
      updateDisplay("AGUARDAR DROP LIVRE", readTempAvg(), analogRead(PIN_TEMP_AIN));
      waitingDrop = true;
    } 
    while (pecaNoDropoff()){
      //updateDisplay("AGUARDAR DROP LIVRE", readTempAvg(), analogRead(PIN_TEMP_AIN));
      delay(5);
    }
    if (waitingDrop) {
    updateDisplay("DROP LIVRE", readTempAvg(), analogRead(PIN_TEMP_AIN));
    waitingDrop = false;
    }
    delay(10);
  }


  updateDisplay("A largar", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(5);
  // estende garra no dropoff
  for (int pos = 0; pos <= 90; pos++) {
    shoulder.write(pos);
    if (pecaNoDropoff() && !waitingDrop) {
    updateDisplay("Clear object!", readTempAvg(), analogRead(PIN_TEMP_AIN));
    waitingDrop = true;
    }
    while (pecaNoDropoff()){
      //updateDisplay("AGUARDAR DROP LIVRE", readTempAvg(), analogRead(PIN_TEMP_AIN));
      delay(5);
    }
    if (waitingDrop) {
    updateDisplay("A largar", readTempAvg(), analogRead(PIN_TEMP_AIN));
    waitingDrop = false;
  }

  delay(10);
  }
  updateDisplay("A descer", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // abre garra (largando peça)
  for (int pos = 150; pos >= 70; pos--) {
    wrist.write(pos);
    delay(10);
  }
  updateDisplay("A soltar", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(2000);

  // CONFIRMAÇÃO IR
  updateDisplay("CONFIRMA DROP", readTempAvg(), analogRead(PIN_TEMP_AIN));
  unsigned long inicioEspera = millis();
  const unsigned long tempoMaximo = 3000;
  bool detectado = false;
  while (millis() - inicioEspera < tempoMaximo) {
    if (pecaNoDropoff()) { detectado = true; break; }
    delay(50);
  }

  if (detectado) {
    updateDisplay("Entregue!", readTempAvg(), analogRead(PIN_TEMP_AIN));
    delay(600);
  } else {
    updateDisplay("Erro!", readTempAvg(), analogRead(PIN_TEMP_AIN));
    delay(1000);
  }

  // recolhe garra
  for (int pos = 90; pos >= 0; pos--) {
    shoulder.write(pos);
    delay(10);
  }
  updateDisplay("A Recolher", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // fecha garra
  for (int pos = 70; pos <= 150; pos++) {
    wrist.write(pos);
    delay(10);
  }
  updateDisplay("A Fechar", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(2000);

  // sobe garra
  for (int pos = 110; pos <= 160; pos++) {
    elbow.write(pos);
    delay(10);
  }
  updateDisplay("A SUBIR", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);

  // gira base centro
  for (int pos = BASE_DROPOFF_ANGLE; pos <= BASE_CENTER_ANGLE; pos++) {
    base.write(pos);
    delay(10);
  }
  updateDisplay("STANDBY", readTempAvg(), analogRead(PIN_TEMP_AIN));
  delay(500);
}
