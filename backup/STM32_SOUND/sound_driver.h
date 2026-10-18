#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"

// ==============================================================================
// THƯ VIỆN ĐIỀU KHIỂN 7 RELAY, 6 NÚT BẤM & GIAO TIẾP UART QUA LẠI VỚI BO RELAY
// ==============================================================================

// Mảng chân của 7 Relay
static const uint32_t RELAY_PINS[TOTAL_RELAYS] = {
  PIN_RELAY_1, PIN_RELAY_2, PIN_RELAY_3, PIN_RELAY_4,
  PIN_RELAY_5, PIN_RELAY_6, PIN_RELAY_7
};

// Khởi tạo cổng UART mềm trên chân 13 (RX = PB13) và chân 12 (TX = PB12)
static SoftwareSerial RelaySerial(PIN_UART_RX, PIN_UART_TX);

// ==============================================================================
// CÁC HÀM TIỆN ÍCH DÀNH CHO NGƯỜI DÙNG
// ==============================================================================

// 1. Khởi tạo toàn bộ hệ thống (Relay, nút bấm, UART)
inline void khoiTaoHeThong() {
  // Giải phóng triệt để USART2 mặc định trên PA2/PA3 để chân PA2 hoàn toàn là chân GPIO của Relay 3
  Serial.end();

  // Cấu hình LED trạng thái bo PC13
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  // Cấu hình 7 chân Relay (Đảm bảo xuất mức OFF ngay từ đầu để không bị chớp nháy)
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }

  // Cấu hình 6 chân nút nhấn chế độ kéo lên nội bộ INPUT_PULLUP
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);
  pinMode(PIN_BTN_3, INPUT_PULLUP);
  pinMode(PIN_BTN_4, INPUT_PULLUP);
  pinMode(PIN_BTN_5, INPUT_PULLUP);
  pinMode(PIN_BTN_6, INPUT_PULLUP);

  // Mở cổng UART phần cứng USART1 (PA9 TX, PA10 RX) kết nối sang bo Relay
  RelaySerial.begin(UART_BAUDRATE);
}

// 2. BẬT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 7)
inline void batRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    digitalWrite(RELAY_PINS[relayNum - 1], RELAY_ON);
  }
}

// 3. TẮT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 7)
inline void tatRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    digitalWrite(RELAY_PINS[relayNum - 1], RELAY_OFF);
  }
}

// 4. BẬT toàn bộ 7 rơ-le cùng lúc
inline void batTatCaRelay() {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_ON);
  }
}

// 5. TẮT toàn bộ 7 rơ-le cùng lúc
inline void tatTatCaRelay() {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }
}

// 6. Gửi lệnh ký tự sang bo STM32 Relay (qua PA9 TX)
inline void guiLenhRelay(char cmd) {
  RelaySerial.print(cmd);
  RelaySerial.flush();
}

// 7. Đọc lệnh từ bo STM32 Relay gửi sang (qua PA10 RX, trả về 0 nếu chưa có lệnh)
inline char docLenhRelay() {
  if (RelaySerial.available() > 0) {
    return RelaySerial.read();
  }
  return 0;
}

// 8. Kiểm tra xem nút nào đang được bấm (Trả về 1..6, hoặc 0 nếu không bấm)
inline uint8_t kiemTraNutNhan() {
  if (digitalRead(PIN_BTN_1) == LOW) return 1;
  if (digitalRead(PIN_BTN_2) == LOW) return 2;
  if (digitalRead(PIN_BTN_3) == LOW) return 3;
  if (digitalRead(PIN_BTN_4) == LOW) return 4;
  if (digitalRead(PIN_BTN_5) == LOW) return 5;
  if (digitalRead(PIN_BTN_6) == LOW) return 6;
  return 0;
}

// 9. Nhấp nháy đèn LED trạng thái bo PC13 liên tục (100ms)
inline void nhapNhayLed() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 100) {
    lastTime = millis();
    digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
  }
}
