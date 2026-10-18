#pragma once
#include <Arduino.h>

// ==============================================================================
// 1. CẤU HÌNH GIAO TIẾP I2C (PHẦN MỀM) ĐIỀU KHIỂN MODULE PCF8575
// ==============================================================================
#define PIN_I2C_SCL PB12  // Chân xung nhịp clock I2C
#define PIN_I2C_SDA PB13  // Chân dữ liệu data I2C
#define PCF8575_ADDR 0x20 // Địa chỉ I2C mặc định của PCF8575

// ==============================================================================
// 2. SỐ LƯỢNG RELAY SỬ DỤNG TRÊN PCF8575 (P0 ĐẾN P14)
// ==============================================================================
#define TOTAL_RELAYS 15   // Sử dụng 15 Relay: P0 -> P14 (P15 & INT: dự trù)

// Mức kích hoạt Relay (Chuẩn Active-LOW)
#define RELAY_ON 0  // Mức LOW: Đóng tiếp điểm Relay
#define RELAY_OFF 1 // Mức HIGH: Ngắt tiếp điểm Relay

// ==============================================================================
// 3. CỔNG GIAO TIẾP UART (USART2 PHẦN CỨNG) VỚI MẠCH ÂM THANH
// ==============================================================================
#define PIN_UART_TX PA2    // Chân TX gửi lệnh sang STM32 Sound
#define PIN_UART_RX PA3    // Chân RX nhận dữ liệu từ STM32 Sound
#define UART_BAUDRATE 9600 // Tốc độ truyền thông

// ==============================================================================
// 4. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT)
// ==============================================================================
#define PIN_LED_STATUS PC13 // Đèn LED xanh tích hợp trên bo STM32 Blue Pill
