#pragma once

// Ângulos típicos do braço - falta ajustar
constexpr int BASE_CENTER_ANGLE   = 90;
constexpr int BASE_PICKUP_ANGLE   = 30;
constexpr int BASE_DROPOFF_ANGLE  = 150;

// Tempos em ms para a máquina de estados
//constexpr unsigned SERVO_SETTLE_MS = 1000;   // tempo para o servo chegar à posição -> precisa?
constexpr unsigned HOLD_MS         = 700;   // pegar objeto
constexpr unsigned RELEASE_MS      = 2000;  // soltar objeto

// Loop principal (tipo 50 Hz)
constexpr float MAIN_INTERVAL_S    = 0.02f; // 20 ms

// OLED
constexpr bool OLED_ENABLED        = true;
