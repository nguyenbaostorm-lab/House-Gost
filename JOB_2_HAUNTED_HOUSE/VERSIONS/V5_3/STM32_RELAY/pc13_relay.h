#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// MODULE ĐIỀU KHIỂN ĐÈN LED PC13 TRÊN BO STM32_RELAY
// Chuẩn theo FLOWCHART_LED_PC13.md:
// - Khởi động 3s: nhấp nháy nhanh (80ms)
// - Khi ổn định / chờ S: nhấp nháy chậm (1000ms)
// - Khi có lệnh S: SÁNG LIÊN TỤC 3 GIÂY, sau 3s thì tắt
// - Sau 10s: quay lại nhấp nháy chậm
// ==============================================================================

enum LedRelayState {
  LED_CHOP_NHANH,     // 1. Khởi động 3s: nhấp nháy nhanh 80ms
  LED_CHOP_CHAM,      // 2. Ổn định chờ lệnh S / Hoạt động: nhấp nháy chậm 1000ms
  LED_SANG_3S_ROI_TAT // 3. Khi nhận lệnh S: sáng liên tục 3s rồi tắt
};

static LedRelayState g_ledRelayState = LED_CHOP_NHANH;
static uint32_t g_thoiGianBatDauSangLed = 0;
static uint32_t g_thoiGianChopLedCuoi = 0;

// Khởi tạo chân PC13
inline void pc13_relay_init() {
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH); // Mặc định tắt (Active-LOW: HIGH = TẮT)
  g_ledRelayState = LED_CHOP_NHANH;
}

// Bật sáng đèn PC13
inline void pc13_relay_on() {
  digitalWrite(PIN_LED_STATUS, LOW); // Mức LOW = SÁNG
}

// Tắt đèn PC13
inline void pc13_relay_off() {
  digitalWrite(PIN_LED_STATUS, HIGH); // Mức HIGH = TẮT
}

// Đảo trạng thái đèn PC13
inline void pc13_relay_toggle() {
  digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
}

// Thiết lập chế độ hoạt động cho đèn PC13
inline void pc13_relay_set_state(LedRelayState state) {
  g_ledRelayState = state;
  if (state == LED_SANG_3S_ROI_TAT) {
    g_thoiGianBatDauSangLed = millis();
    pc13_relay_on(); // Bật sáng ngay lập tức
  }
}

// Cập nhật hoạt động đèn PC13 liên tục trong vòng lặp loop()
inline void pc13_relay_update() {
  uint32_t now = millis();

  switch (g_ledRelayState) {
    case LED_CHOP_NHANH:
      if (now - g_thoiGianChopLedCuoi >= TOC_DO_NHAY_NHANH_MS) {
        g_thoiGianChopLedCuoi = now;
        pc13_relay_toggle();
      }
      break;

    case LED_CHOP_CHAM:
      if (now - g_thoiGianChopLedCuoi >= TOC_DO_NHAY_CHAM_MS) {
        g_thoiGianChopLedCuoi = now;
        pc13_relay_toggle();
      }
      break;

    case LED_SANG_3S_ROI_TAT:
      // Trong 3 giây: Giữ sáng liên tục
      if (now - g_thoiGianBatDauSangLed < THOI_GIAN_LED_SANG_3S_MS) {
        pc13_relay_on();
      } else {
        pc13_relay_off(); // Sau 3s: Tắt đèn
      }
      break;
  }
}
