// Bibliotecas
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED 128x64 (I2C)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const uint8_t OLED_ADDRESS = 0x3C; // usualmente 0x3C

// Configuração dos servos
Servo base;
Servo shoulder;
Servo elbow;
Servo wrist;

int BASE_CENTER_ANGLE  = 90;
int BASE_PICKUP_ANGLE  = 180;
int BASE_DROPOFF_ANGLE = 0;    // dropoff branco
int BASE_DROPOFF_RED   = 135;  // dropoff vermelho

#define PIN_SERVO_BASE      10
#define PIN_SERVO_SHOULDER   9
#define PIN_SERVO_ELBOW      6
#define PIN_SERVO_WRIST      5

// Sensores & IO
#define PIN_IR_DROPOFF      12

// Ventoinha (L9110)
#define INA_PIN 2   // PWM destination (IN1)
#define INB_PIN 3   // other input (keep LOW for fixed direction)

// Sensor temperatura
#define PIN_TEMP_AIN A0

// Sensor de cor TCS34725
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

bool pecaBrancaPresente   = false;
bool pecaVermelhaPresente = false;

// Parâmetros de leitura / ventoinha
const int ADC_MAX = 1023;
const int NUM_SAMPLES_TEMP = 8;
const int SAMPLE_DELAY_MS = 6;
const int PWM_MAX = 255;
const int MIN_PWM_TO_SPIN = 60; // evita trepidações; ajusta conforme necessário
int pwmAtual = 0;

// Controlo térmico proporcional
float tempBaseC = 0.0;      
bool tempBaseOk = false;

const int PWM_MIN_CTRL = 80;
const int PWM_MAX_CTRL = 255;

const float TEMP_WINDOW = 5.0;          // ±5 °C
const float PWM_PER_DEG = (PWM_MAX_CTRL - PWM_MIN_CTRL) / (2.0 * TEMP_WINDOW);    // = 17.5 PWM/°C

// Estados legíveis para o display
String stateText = "Standby";

// Protótipos Funções
bool ehBranco(float r, float g, float b);
bool ehVermelho(float r, float g, float b);
bool pecaNoDropoff();
bool dropoffLivre();
void executarCicloBraco(int baseDropoffAngle, const String &nomeCor);
int readTempAvg();
void updateDisplay(const String &state, int tempRaw, int pwm);

// Setup
void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  // I2C iniciada
  Wire.begin();

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 not found!");
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
  stateText = "Standby";
  updateDisplay(stateText, 0, 0);
  delay(500);
  
// Medição da temperatura base 
const int NUM_BASE_SAMPLES = 5;
float somaTemp = 0;
for (int i = 0; i < NUM_BASE_SAMPLES; i++) {
  int raw = analogRead(PIN_TEMP_AIN);
  float tempC = raw * 3.3 / 1023.0 * 100.0;
  somaTemp += tempC;
  delay(100);
}

tempBaseC = somaTemp / NUM_BASE_SAMPLES;
tempBaseOk = true;

Serial.print("Temperatura base (C): ");
Serial.println(tempBaseC);

Serial.println("Sistema iniciado.");
}



void loop() {
  // 1) Controlo da ventoinha (sempre)
  int tempRaw = readTempAvg();
  float tempC = tempRaw * 3.3 / 1023.0 * 100.0;

  int pwm;

  // limites da janela
  float tempMin = tempBaseC - TEMP_WINDOW;
  float tempMax = tempBaseC + TEMP_WINDOW;

  if (tempC <= tempMin) {
    pwm = PWM_MIN_CTRL;
  }
  else if (tempC >= tempMax) {
    pwm = PWM_MAX_CTRL;
  }
  else {
    // zona linear
    float delta = tempC - tempMin;   // 0 → 10 °C
    pwm = PWM_MIN_CTRL + delta * PWM_PER_DEG;
  }

  // segurança extra
  if (pwm < PWM_MIN_CTRL) pwm = PWM_MIN_CTRL;
  if (pwm > PWM_MAX_CTRL) pwm = PWM_MAX_CTRL;

  pwmAtual = pwm;
  digitalWrite(INB_PIN, LOW);
  analogWrite(INA_PIN, pwmAtual);

  // 2) Mostrar estado / temp / pwm no display
  updateDisplay("Standby", tempRaw, pwmAtual);

  // 3) Lógica do braço: só começa quando dropoff livre (se já tiver peça, não inicia ciclo)
  if (!dropoffLivre()) {
    stateText = "Drop Ocupado";
    updateDisplay(stateText, tempRaw, pwmAtual);
    pecaBrancaPresente   = false;
    pecaVermelhaPresente = false;
    delay(300);
    return;
  }

  // 4) Ler sensor de cor
  float red, green, blue;
  tcs.setInterrupt(false);
  delay(60);
  tcs.getRGB(&red, &green, &blue);
  tcs.setInterrupt(true);

  bool brancaAgora   = ehBranco(red, green, blue);
  bool vermelhaAgora = ehVermelho(red, green, blue);

  // 5) Transições: peça detectada e dropoff livre
  // prioridade pra branco se as duas derem true por ruído
  if (brancaAgora && !pecaBrancaPresente && !vermelhaAgora) {
    executarCicloBraco(BASE_DROPOFF_ANGLE, "branco");
  } 
  else if (vermelhaAgora && !pecaVermelhaPresente && !brancaAgora) {
    executarCicloBraco(BASE_DROPOFF_RED, "vermelho");
  }

  pecaBrancaPresente   = brancaAgora;
  pecaVermelhaPresente = vermelhaAgora;

  delay(100);
}

// Funções utilitárias

bool ehBranco(float r, float g, float b) {
  const float limiarBrilho = 115;
  const float maxDesbalanceamento = 1.35;
  if (r < limiarBrilho || g < limiarBrilho || b < limiarBrilho) return false;
  float maxv = max(r, max(g, b));
  float minv = min(r, min(g, b));
  if (maxv / minv > maxDesbalanceamento) return false;
  return true;
}

// detecção de vermelho
bool ehVermelho(float r, float g, float b) {
  const float limiarBrilho = 80;
  const float fatorDominancia = 1.4;

  if (r < limiarBrilho) return false;
  if (r < g * fatorDominancia) return false;
  if (r < b * fatorDominancia) return false;

  float maxv = max(r, max(g, b));
  float minv = min(r, min(g, b));
  if (maxv / minv < 1.2) return false;

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
  float temperatura_graus = tempRaw * 3.3 / 1023.0 * 100.0;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Estado:");
  display.println(state);
  display.setCursor(0, 26);
  display.print("Estado Drop : ");
  display.print( pecaNoDropoff() ? "Ocupado" : "Livre" );
  display.setCursor(0, 40);
  display.print("PWM ventoinha: ");
  display.print(pwm);
  display.setCursor(0, 48);
  display.print("Temp Atual:   ");
  display.print(temperatura_graus,1);
  display.setCursor(0,56);
  display.print("Temp Inicial: ");
  display.print(tempBaseC,1);
  display.display();
}


void executarCicloBraco(int baseDropoffAngle, const String &nomeCor) {
  updateDisplay("Ciclo " + nomeCor + "  Roda -> Pickup", readTempAvg(), pwmAtual);
  delay(200);

  // girar pra pickup
  for (int pos = BASE_CENTER_ANGLE; pos <= BASE_PICKUP_ANGLE; pos++) {
    base.write(pos);
    delay(10);
  }
  updateDisplay("Ciclo " + nomeCor + "  Desce Braco ", readTempAvg(), pwmAtual);
  delay(500);

  // descer garra
  for (int pos = 160; pos >= 110; pos--) {
    elbow.write(pos);
    delay(10);
  }
  updateDisplay("Ciclo " + nomeCor + "  A abrir Garra", readTempAvg(), pwmAtual);
  delay(500);

  // abrir garra
  for (int pos = 150; pos >= 70; pos--) {
    wrist.write(pos);
    delay(10);
  }
  delay(2000);
  updateDisplay("Ciclo " + nomeCor + "  A estender -> Pick", readTempAvg(), pwmAtual);
  // estende braço
  for (int pos = 0; pos <= 80; pos++) {
    shoulder.write(pos);
    delay(10);
  }
  delay(500);
  updateDisplay("Ciclo " + nomeCor + "  A agarrar peca", readTempAvg(), pwmAtual);
  // fechar garra (pega peça)
  for (int pos = 70; pos <= 150; pos++) {
    wrist.write(pos);
    delay(10);
  }
  delay(2000);
  updateDisplay("Ciclo " + nomeCor + "  A recolher peca", readTempAvg(), pwmAtual);
  // volta braço (com peça)
  for (int pos = 80; pos >= 0; pos--) {
    shoulder.write(pos);
    delay(10);
  }
  updateDisplay("Ciclo " + nomeCor + "  Roda -> Drop", readTempAvg(), pwmAtual);
  delay(5);

  
  // GIRA BASE PARA DROPOFF
  // (só o BRANCO respeita sensor Drop e pausa se tiver objeto)
  bool waitingDrop = false;

  for (int pos = BASE_PICKUP_ANGLE; pos >= baseDropoffAngle; pos--) {
    base.write(pos);

    if (nomeCor == "branco") {
      // se aparecer objeto no IR, mostrar e esperar liberar
      if (pecaNoDropoff() && !waitingDrop) {
        updateDisplay("Ciclo " + nomeCor + "  Drop ocupado", readTempAvg(), pwmAtual);
        waitingDrop = true;
      }

      while (pecaNoDropoff()) {
        // fica parado até o dropoff ficar livre de novo
        delay(5);
      }

      if (waitingDrop && !pecaNoDropoff()) {
        updateDisplay("Ciclo " + nomeCor + "  Drop Livre", readTempAvg(), pwmAtual);
        waitingDrop = false;
      }
    }

    delay(10);
  }

  updateDisplay("Ciclo " + nomeCor + "  A estender -> Drop" , readTempAvg(), pwmAtual);
  delay(5);

  // estende garra no dropoff
  for (int pos = 0; pos <= 90; pos++) {
    shoulder.write(pos);
    delay(10);
  }
  delay(500);
  
  updateDisplay("Ciclo " + nomeCor + "  A soltar peca", readTempAvg(), pwmAtual);
  // abre garra (largando peça)
  for (int pos = 150; pos >= 70; pos--) {
    wrist.write(pos);
    delay(10);
  }
  delay(2000);

  // CONFIRMAÇÃO 
  if (nomeCor == "branco") {
    updateDisplay("Ciclo " + nomeCor + "  A confirmar entrega", readTempAvg(), pwmAtual);
    unsigned long inicioEspera = millis();
    const unsigned long tempoMaximo = 3000;
    bool detectado = false;
    while (millis() - inicioEspera < tempoMaximo) {
      if (pecaNoDropoff()) { 
        detectado = true; 
        break; 
      }
      delay(50);
    }

    if (detectado) {
      updateDisplay("Ciclo " + nomeCor + "  Peca entregue!", readTempAvg(), pwmAtual);
      delay(1500);
    } else {
      updateDisplay("Ciclo " + nomeCor + "  Erro na entrega!", readTempAvg(), pwmAtual);
      delay(1500);
    }
  } else {
    // VERMELHO: assume que entregou
    updateDisplay("Ciclo " + nomeCor + "  Peca entregue", readTempAvg(), pwmAtual);
    delay(1500);
  }

  // recolhe garra
  updateDisplay("Ciclo " + nomeCor + "  A recolher braco", readTempAvg(), pwmAtual);
  for (int pos = 90; pos >= 0; pos--) {
    shoulder.write(pos);
    delay(10);
  }
  updateDisplay("Ciclo " + nomeCor + "  A fechar garra", readTempAvg(), pwmAtual);
  delay(500);
  // fecha garra
  for (int pos = 70; pos <= 150; pos++) {
    wrist.write(pos);
    delay(10);
  }
  delay(2000);

  updateDisplay("Ciclo " + nomeCor + "  A subir braco", readTempAvg(), pwmAtual);
  // sobe garra
  for (int pos = 110; pos <= 160; pos++) {
    elbow.write(pos);
    delay(10);
  }
  updateDisplay("Ciclo " + nomeCor + "  Roda -> Standby", readTempAvg(), pwmAtual);
  delay(500);
  // gira base de volta ao centro (funciona para branco e vermelho)
  if (baseDropoffAngle < BASE_CENTER_ANGLE) {
    for (int pos = baseDropoffAngle; pos <= BASE_CENTER_ANGLE; pos++) {
      base.write(pos);
      delay(10);
    }
  } else {
    for (int pos = baseDropoffAngle; pos >= BASE_CENTER_ANGLE; pos--) {
      base.write(pos);
      delay(10);
    }
  }

  updateDisplay("Standby", readTempAvg(), pwmAtual);
  delay(500);
}
