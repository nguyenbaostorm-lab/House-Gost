#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// THƯ VIỆN ĐIỀU KHIỂN MODULE I2C PCF8575 & GIAO TIẾP UART
// ==============================================================================

// Biến lưu trạng thái 16 chân của PCF8575 (Mặc định 0xFFFF = toàn bộ mức HIGH = Tắt hết)
static uint16_t g_pcfData   = 0xFFFF;
static uint8_t  g_pcfAddr   = PCF8575_ADDR; // Mặc định 0x20
static uint32_t g_sclPin    = PIN_I2C_SCL;  // Mặc định PB12
static uint32_t g_sdaPin    = PIN_I2C_SDA;  // Mặc định PB13
static bool     g_pcfFound  = false;

// Sử dụng cổng phần cứng USART2: PA2 (TX), PA3 (RX) để kết nối bo Sound
#define SoundSerial Serial2

// ------------------------------------------------------------------------------
// GIAO TIẾP I2C PHẦN MỀM CHUẨN OPEN-DRAIN (TỐC ĐỘ 100KHZ, AN TOÀN TUYỆT ĐỐI)
// ------------------------------------------------------------------------------
static inline void i2c_delay() {
  delayMicroseconds(5); // 5us = tần số ~100kHz chuẩn I2C
}

static inline void sda_high() { digitalWrite(g_sdaPin, HIGH); }
static inline void sda_low()  { digitalWrite(g_sdaPin, LOW); }
static inline void scl_high() { digitalWrite(g_sclPin, HIGH); }
static inline void scl_low()  { digitalWrite(g_sclPin, LOW); }

static void i2c_start() {
  sda_high();
  scl_high();
  i2c_delay();
  sda_low();
  i2c_delay();
  scl_low();
  i2c_delay();
}

static void i2c_stop() {
  sda_low();
  i2c_delay();
  scl_high();
  i2c_delay();
  sda_high();
  i2c_delay();
}

static bool i2c_write_byte(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x80) sda_high();
    else sda_low();
    i2c_delay();
    scl_high();
    i2c_delay();
    scl_low();
    i2c_delay();
    data <<= 1;
  }
  // Đọc ACK: Nhả SDA lên mức HIGH và đọc về (chuẩn Open-Drain không cần đổi pinMode)
  sda_high();
  i2c_delay();
  scl_high();
  i2c_delay();
  bool ack = (digitalRead(g_sdaPin) == LOW);
  scl_low();
  i2c_delay();
  return ack;
}

// Kiểm tra xem địa chỉ I2C có phản hồi không
static bool i2c_ping(uint8_t addr) {
  i2c_start();
  bool ack = i2c_write_byte(addr << 1);
  i2c_stop();
  return ack;
}

// Cấu hình chân I2C ở chế độ Open-Drain
static void i2c_init_pins(uint32_t scl, uint32_t sda) {
  g_sclPin = scl;
  g_sdaPin = sda;
  pinMode(g_sclPin, OUTPUT_OPEN_DRAIN);
  pinMode(g_sdaPin, OUTPUT_OPEN_DRAIN);
  digitalWrite(g_sclPin, HIGH);
  digitalWrite(g_sdaPin, HIGH);
  delay(10);
}

// Quét toàn bộ dải địa chỉ PCF8575 (0x20..0x27) và PCF8575A (0x38..0x3F)
static bool pcf8575_scan_address() {
  const uint8_t possibleAddrs[] = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
  };
  for (uint8_t i = 0; i < sizeof(possibleAddrs); i++) {
    if (i2c_ping(possibleAddrs[i])) {
      g_pcfAddr = possibleAddrs[i];
      g_pcfFound = true;
      return true;
    }
  }
  return false;
}

// Tự động dò chân và địa chỉ I2C thông minh
static void pcf8575_auto_detect() {
  // Thử nghiệm 1: Chuẩn PB12 (SCL) và PB13 (SDA)
  i2c_init_pins(PB12, PB13);
  if (pcf8575_scan_address()) return;

  // Thử nghiệm 2: Đảo chân PB13 (SCL) và PB12 (SDA) (phòng trường hợp cắm chéo dây)
  i2c_init_pins(PB13, PB12);
  if (pcf8575_scan_address()) return;

  // Thử nghiệm 3: Chân I2C1 chuẩn PB6 (SCL) và PB7 (SDA)
  i2c_init_pins(PB6, PB7);
  if (pcf8575_scan_address()) return;

  // Thử nghiệm 4: Chân I2C1 Remap PB8 (SCL) và PB9 (SDA)
  i2c_init_pins(PB8, PB9);
  if (pcf8575_scan_address()) return;

  // Thử nghiệm 5: Chân I2C2 chuẩn PB10 (SCL) và PB11 (SDA)
  i2c_init_pins(PB10, PB11);
  if (pcf8575_scan_address()) return;

  // Nếu không dò thấy, quay về mặc định PB12 & PB13
  i2c_init_pins(PIN_I2C_SCL, PIN_I2C_SDA);
  g_pcfAddr = PCF8575_ADDR;
}

// Gửi toàn bộ 16-bit dữ liệu tới PCF8575
static void pcf8575_update() {
  i2c_start();
  i2c_write_byte(g_pcfAddr << 1);               // Gửi địa chỉ ghi (Write)
  i2c_write_byte((uint8_t)(g_pcfData & 0xFF));  // Byte thấp (P0 -> P7)
  i2c_write_byte((uint8_t)((g_pcfData >> 8) & 0xFF)); // Byte cao (P8 -> P15)
  i2c_stop();
}

// ==============================================================================
// CÁC HÀM ĐIỀU KHIỂN CÔNG CỤ TIỆN ÍCH DÀNH CHO NGƯỜI DÙNG
// ==============================================================================

// 1. Khởi tạo toàn bộ hệ thống
inline void khoiTaoHeThong() {
  // Cấu hình LED báo trạng thái bo PC13
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  // Mở UART kết nối sang bo Sound
  SoundSerial.begin(UART_BAUDRATE);

  // Tự động dò chân I2C và địa chỉ PCF8575
  pcf8575_auto_detect();

  // Mặc định ngắt toàn bộ 13 Relay (mức HIGH = Active-LOW ngắt)
  g_pcfData = 0xFFFF;
  pcf8575_update();

  // Báo hiệu trạng thái tìm kiếm I2C bằng đèn PC13:
  if (g_pcfFound) {
    // Tìm thấy module PCF8575 -> Nháy nhanh 4 nhịp xác nhận thành công
    for (int i = 0; i < 4; i++) {
      digitalWrite(PIN_LED_STATUS, LOW);
      delay(70);
      digitalWrite(PIN_LED_STATUS, HIGH);
      delay(70);
    }
  }
}

// 2. BẬT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 13)
inline void batRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    uint8_t pinBit = relayNum - 1; // Relay 1 tương ứng chân P0
    g_pcfData &= ~(1 << pinBit);   // Kéo bit về 0 (Active-LOW: Bật)
    pcf8575_update();
  }
}

// 3. TẮT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 13)
inline void tatRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    uint8_t pinBit = relayNum - 1;
    g_pcfData |= (1 << pinBit);    // Kéo bit lên 1 (Active-LOW: Tắt)
    pcf8575_update();
  }
}

// 4. BẬT TẤT CẢ các rơ-le đang dùng (1 đến 13)
inline void batTatCaRelay() {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    g_pcfData &= ~(1 << i);
  }
  pcf8575_update();
}

// 5. TẮT TẤT CẢ các rơ-le (Ngắt an toàn)
inline void tatTatCaRelay() {
  g_pcfData = 0xFFFF;
  pcf8575_update();
}

// 6. Gửi lệnh ký tự sang bo STM32 SOUND qua UART (PA2 TX)
inline void guiLenhSound(char cmd) {
  SoundSerial.print(cmd);
  SoundSerial.flush();
}

// 7. Nhấp nháy đèn LED trạng thái bo liên tục
inline void nhapNhayLed() {
  static unsigned long lastTime = 0;
  if (millis() - lastTime >= 100) {
    lastTime = millis();
    digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
  }
}

// 8. Đọc lệnh gửi từ bo STM32 Sound sang
inline char docLenhSound() {
  if (SoundSerial.available() > 0) {
    return SoundSerial.read();
  }
  return 0;
}
