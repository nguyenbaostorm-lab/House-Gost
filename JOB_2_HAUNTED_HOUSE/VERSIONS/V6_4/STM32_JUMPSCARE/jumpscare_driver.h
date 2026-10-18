#pragma once
#include <Arduino.h>
#include "config.h"

// Cấu trúc dữ liệu quản lý từng kênh Relay Timer/Toggle
struct RelayChannel {
  int swPin;
  int relayPin;
  unsigned long duration;
  
  bool relayState;                // true: đang bật, false: đang tắt
  unsigned long startTime;        // Mốc thời gian bắt đầu kích
  
  int lastButtonReading;          // Giá trị đọc tức thời trước đó
  int buttonState;                // Trạng thái nút sau khi lọc rung (debounced)
  unsigned long lastDebounceTime; // Thời điểm phát hiện thay đổi
};

// Khai báo 3 kênh Relay
static RelayChannel channels[3] = {
  { PIN_SW1, PIN_RELAY1, DURATION_CH1, false, 0, HIGH, HIGH, 0 },
  { PIN_SW2, PIN_RELAY2, DURATION_CH2, false, 0, HIGH, HIGH, 0 },
  { PIN_SW3, PIN_RELAY3, DURATION_CH3, false, 0, HIGH, HIGH, 0 }
};

inline void delay_us_jump(uint32_t us) {
  delayMicroseconds(us);
}

// Khởi tạo toàn bộ chân GPIO và UART
inline void khoiTaoJumpscare(void) {
  // 1. Cấu hình 3 kênh công tắc (INPUT_PULLUP) và 3 kênh Relay (OUTPUT)
  for (int i = 0; i < 3; i++) {
    pinMode(channels[i].swPin, INPUT_PULLUP);
    pinMode(channels[i].relayPin, OUTPUT);
    digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
    channels[i].relayState = false;
  }

  // 2. Cấu hình LED PC13
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH); // LED ban đầu tắt

  // 3. Cấu hình UART Bit-Bang với Bo Sound: PB5 (TX) Output PP, PB6 (RX) Input Pull-Up
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitStruct.Pin = PIN_UART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(PIN_UART_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET); // Idle = HIGH

  GPIO_InitStruct.Pin = PIN_UART_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PIN_UART_PORT, &GPIO_InitStruct);
}

// Bật 1 kênh Relay cụ thể và bắt đầu đếm giờ
inline void batKenhRelay(uint8_t index, unsigned long now) {
  if (index < 3) {
    channels[index].relayState = true;
    channels[index].startTime = now;
    digitalWrite(channels[index].relayPin, RELAY_ACTIVE);
  }
}

// Tắt 1 kênh Relay cụ thể ngay lập tức
inline void tatKenhRelay(uint8_t index) {
  if (index < 3) {
    channels[index].relayState = false;
    digitalWrite(channels[index].relayPin, RELAY_INACTIVE);
  }
}

// Tắt toàn bộ 3 kênh Relay khẩn cấp
inline void tatTatCaRelay(void) {
  for (int i = 0; i < 3; i++) {
    channels[i].relayState = false;
    digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
  }
}

// Gửi lệnh ký tự sang bo STM32 Sound (qua PB5 TX với 9600 baud)
inline void guiLenhSound(char cmd) {
  noInterrupts();
  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_RESET); // Start bit
  delay_us_jump(104);

  uint8_t data = (uint8_t)cmd;
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x01) {
      HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_RESET);
    }
    delay_us_jump(104);
    data >>= 1;
  }

  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET); // Stop bit
  delay_us_jump(104);
  interrupts();
}

// Đọc lệnh từ bo STM32 Sound gửi sang (qua PB6 RX)
inline char docLenhSound(void) {
  if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) == GPIO_PIN_RESET) {
    delay_us_jump(52); // Lấy mẫu giữa Start bit
    if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) != GPIO_PIN_RESET) {
      return 0;
    }

    uint8_t receivedChar = 0;
    for (uint8_t i = 0; i < 8; i++) {
      delay_us_jump(104);
      if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) == GPIO_PIN_SET) {
        receivedChar |= (1 << i);
      }
    }

    delay_us_jump(104); // Stop bit
    return (char)receivedChar;
  }
  return 0;
}
