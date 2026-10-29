#pragma once
#include <Arduino.h>

// ==========================================
// CẤU HÌNH NGOẠI VI STM32 JUMPSCARE
// ==========================================
#define PIN_LED_PC13  PC13  // Đèn báo trên mạch STM32

// 3 chân Relay kích hoạt (Kéo nội INPUT_PULLUP, kích mức LOW)
#define PIN_RELAY_1   PA1   // Relay 1 -> Bài 0001
#define PIN_RELAY_2   PA2   // Relay 2 -> Bài 0002
#define PIN_RELAY_3   PA3   // Relay 3 -> Bài 0003

// 3 chân Đèn LED báo bài hát (Âm chung, HIGH là sáng, LOW là tắt qua trở 330R)
#define PIN_LED_TRACK1 PA7  // Đèn cho bài 0001
#define PIN_LED_TRACK2 PA6  // Đèn cho bài 0002
#define PIN_LED_TRACK3 PB4  // Đèn cho bài 0003

// Mảng chân Relay và chân LED
const uint8_t RELAY_PINS[3] = {PIN_RELAY_1, PIN_RELAY_2, PIN_RELAY_3};
const uint8_t LED_PINS[3]   = {PIN_LED_TRACK1, PIN_LED_TRACK2, PIN_LED_TRACK3};

// ==========================================
// CẤU HÌNH GIAO TIẾP DFPLAYER MINI (Hardware UART3)
// STM32: PB10 (TX) -> trở 1k -> DFPlayer RX
// STM32: PB11 (RX) <- DFPlayer TX
// ==========================================
#define DFPLAYER_TX_PIN PB10
#define DFPLAYER_RX_PIN PB11
#define DFPLAYER_BAUDRATE 9600
#define DFPLAYER_DEFAULT_VOLUME 25  // Mức âm lượng 25/30
