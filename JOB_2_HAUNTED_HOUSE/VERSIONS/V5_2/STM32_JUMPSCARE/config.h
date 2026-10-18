#pragma once
#include <Arduino.h>
#include "stm32f1xx_hal.h"

// ==============================================================================
// 1. CỔNG GIAO TIẾP UART VỚI BO STM32_SOUND (PB5 TX sang B11 & PB6 RX tu B10)
// ==============================================================================
#define PIN_UART_PORT GPIOB
#define PIN_UART_TX_PIN GPIO_PIN_5 // Chân B5 (TX bo Jump): Nối sang chân B11 (RX bo Sound)
#define PIN_UART_RX_PIN GPIO_PIN_6 // Chân B6 (RX bo Jump): Nối tu chan B10 (TX bo Sound)
#define UART_BAUDRATE 9600         // Tốc độ truyền thông 9600 baud

// ==============================================================================
// 2. CẤU HÌNH MỨC KÍCH RELAY
// ==============================================================================
#define RELAY_ACTIVE    HIGH  // Mức kích hoạt rơ-le (BẬT)
#define RELAY_INACTIVE  LOW   // Mức ngắt rơ-le (TẮT)

// ==============================================================================
// 3. SƠ ĐỒ CHÂN: 3 CÔNG TẮC & 3 RELAY TIMER/TOGGLE
// ==============================================================================
// Kênh 1: Công tắc PA6 -> Relay 1 PA3 (1 phút 11 giây = 71 giây)
const int PIN_SW1 = PA6;
const int PIN_RELAY1 = PA3;
const unsigned long DURATION_CH1 = 71000UL;

// Kênh 2: Công tắc PA7 -> Relay 2 PA4 (57 giây)
const int PIN_SW2 = PA7;
const int PIN_RELAY2 = PA4;
const unsigned long DURATION_CH2 = 57000UL;

// Kênh 3: Công tắc PB12 -> Relay 3 PB13 (1 phút 26 giây = 86 giây)
const int PIN_SW3 = PB12;
const int PIN_RELAY3 = PB13;
const unsigned long DURATION_CH3 = 86000UL;

// Chống rung phím 50ms
const unsigned long DEBOUNCE_DELAY = 50;

// ==============================================================================
// 4. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT) PC13
// ==============================================================================
#define PIN_LED_PORT GPIOC
#define PIN_LED_PIN  GPIO_PIN_13 // LED xanh tích hợp PC13
#define LED_ON  GPIO_PIN_RESET
#define LED_OFF GPIO_PIN_SET
