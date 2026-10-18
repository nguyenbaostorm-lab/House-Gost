#pragma once
#include <Arduino.h>

// ==============================================================================
// 1. CẤU HÌNH 7 CHÂN RELAY TRÊN BO (PA0 ĐẾN PA6)
// ==============================================================================
#define PIN_RELAY_1      PA0    // Relay số 1 -> Chân PA0
#define PIN_RELAY_2      PA1    // Relay số 2 -> Chân PA1
#define PIN_RELAY_3      PA2    // Relay số 3 -> Chân PA2
#define PIN_RELAY_4      PA3    // Relay số 4 -> Chân PA3
#define PIN_RELAY_5      PA4    // Relay số 5 -> Chân PA4
#define PIN_RELAY_6      PA5    // Relay số 6 -> Chân PA5
#define PIN_RELAY_7      PA6    // Relay số 7 -> Chân PA6
#define TOTAL_RELAYS     7      // Tổng số 7 Relay đang sử dụng trên bo

// Mức kích hoạt Relay (Chuẩn Active-HIGH theo thực tế phần cứng)
#define RELAY_ON         1      // Mức HIGH (3.3V): Đóng rơ-le (Bật)
#define RELAY_OFF        0      // Mức LOW (0V): Ngắt rơ-le (Tắt)

// ==============================================================================
// 2. CẤU HÌNH 6 CHÂN NÚT BẤM CƠ (CÁCH NỐI: 1 ĐẦU VÀO CHÂN STM32, 1 ĐẦU VÀO MASS GND)
// ==============================================================================
#define PIN_NUT_NHAN_1   PB0    // 👉 NÚT NHẤN 1: Chân PB0 (Bấm nút -> kéo xuống GND)
#define PIN_NUT_NHAN_2   PB5    // 👉 NÚT NHẤN 2: Chân PB5 (Bấm nút -> kéo xuống GND)
#define PIN_NUT_NHAN_3   PB6    // 👉 NÚT NHẤN 3: Chân PB6 (Bấm nút -> kéo xuống GND)
#define PIN_NUT_NHAN_4   PB7    // 👉 NÚT NHẤN 4: Chân PB7 (Bấm nút -> kéo xuống GND)
#define PIN_NUT_NHAN_5   PB8    // 👉 NÚT NHẤN 5: Chân PB8 (Bấm nút -> kéo xuống GND)
#define PIN_NUT_NHAN_6   PB9    // 👉 NÚT NHẤN 6: Chân PB9 (Bấm nút -> kéo xuống GND - Nút dừng khẩn cấp)

// Tên viết tắt ngắn gọn để sử dụng trong thư viện và code:
#define PIN_BTN_1        PIN_NUT_NHAN_1
#define PIN_BTN_2        PIN_NUT_NHAN_2
#define PIN_BTN_3        PIN_NUT_NHAN_3
#define PIN_BTN_4        PIN_NUT_NHAN_4
#define PIN_BTN_5        PIN_NUT_NHAN_5
#define PIN_BTN_6        PIN_NUT_NHAN_6

// ==============================================================================
// 3. CỔNG UART GIAO TIẾP VỚI BO STM32 RELAY (CHÂN 12 TX & 13 RX)
// ==============================================================================
#define PIN_UART_TX      PB12   // Chân 12 (PB12 - TX): Truyền lệnh sang bo Relay
#define PIN_UART_RX      PB13   // Chân 13 (PB13 - RX): Nhận dữ liệu từ bo Relay
#define UART_BAUDRATE    9600   // Tốc độ truyền thông nối tiếp 9600 baud

// ==============================================================================
// 4. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT)
// ==============================================================================
#define PIN_LED_STATUS   PC13   // Đèn LED xanh tích hợp trên bo STM32 Blue Pill
