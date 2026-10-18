#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// THƯ VIỆN ĐIỀU KHIỂN PCF8575 I2C OPEN-DRAIN CHUẨN XÁC NGUYÊN BẢN V1
// ==============================================================================

static uint16_t g_pcfData = 0x0000;
static bool g_p6DaBat = false;

#define SoundSerial Serial2

// ------------------------------------------------------------------------------
// GIAO TIẾP I2C OPEN-DRAIN CHUẨN (CHUẨN V1 ỔN ĐỊNH 100%)
// ------------------------------------------------------------------------------
static inline void i2c_delay() { delayMicroseconds(6); }
static inline void sda_high()  { digitalWrite(PIN_I2C_SDA, HIGH); }
static inline void sda_low()   { digitalWrite(PIN_I2C_SDA, LOW); }
static inline void scl_high()  { digitalWrite(PIN_I2C_SCL, HIGH); }
static inline void scl_low()   { digitalWrite(PIN_I2C_SCL, LOW); }

static void i2c_start() {
  sda_high(); scl_high(); i2c_delay();
  sda_low();  i2c_delay();
  scl_low();  i2c_delay();
}

static void i2c_stop() {
  sda_low();  i2c_delay();
  scl_high(); i2c_delay();
  sda_high(); i2c_delay();
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
  sda_high(); i2c_delay();
  scl_high(); i2c_delay();
  bool ack = (digitalRead(PIN_I2C_SDA) == LOW);
  scl_low();  i2c_delay();
  return ack;
}

// Hàm ghi 16-bit dữ liệu tới PCF8575 (Địa chỉ 0x27) - Chuẩn V1 không chèn xung SCL
inline bool writePCF8575(uint16_t data) {
  data |= MASK_RF_INPUT_PINS;
  if (g_p6DaBat) {
    data |= (1 << 6);
    g_pcfData |= (1 << 6);
  }

  bool thanhCong = false;
  for (uint8_t retry = 0; retry < 3; retry++) {
    noInterrupts();
    i2c_start();
    bool ack1 = i2c_write_byte(PCF8575_ADDR << 1);            // 0x4E
    bool ack2 = i2c_write_byte((uint8_t)(data & 0xFF));         // P00 - P07
    bool ack3 = i2c_write_byte((uint8_t)((data >> 8) & 0xFF)); // P10 - P17
    i2c_stop();
    interrupts();

    if (ack1 && ack2 && ack3) {
      thanhCong = true;
      break;
    }
    delayMicroseconds(50);
  }
  return thanhCong;
}

// Hàm đọc 16-bit dữ liệu từ PCF8575 (Địa chỉ 0x27) - Chuẩn V1
inline uint16_t readPCF8575() {
  noInterrupts();
  i2c_start();
  if (!i2c_write_byte((PCF8575_ADDR << 1) | 0x01)) { // 0x4F
    i2c_stop();
    interrupts();
    return 0xFFFF;
  }

  // 1. Đọc Byte thấp (P00 - P07)
  uint8_t lowByte = 0;
  sda_high();
  for (uint8_t i = 0; i < 8; i++) {
    lowByte <<= 1;
    scl_high();
    i2c_delay();
    if (digitalRead(PIN_I2C_SDA) == HIGH) lowByte |= 1;
    scl_low();
    i2c_delay();
  }
  // Gửi Master ACK để đọc tiếp Byte cao
  sda_low();
  i2c_delay();
  scl_high();
  i2c_delay();
  scl_low();
  i2c_delay();
  sda_high();

  // 2. Đọc Byte cao (P10 - P17)
  uint8_t highByte = 0;
  sda_high();
  for (uint8_t i = 0; i < 8; i++) {
    highByte <<= 1;
    scl_high();
    i2c_delay();
    if (digitalRead(PIN_I2C_SDA) == HIGH) highByte |= 1;
    scl_low();
    i2c_delay();
  }
  // Gửi Master NACK để kết thúc
  sda_high();
  i2c_delay();
  scl_high();
  i2c_delay();
  scl_low();
  i2c_delay();

  i2c_stop();
  interrupts();
  return (uint16_t)lowByte | ((uint16_t)highByte << 8);
}

// ==============================================================================
// CÁC HÀM XỬ LÝ RF433
// ==============================================================================
inline bool docTinHieuRF_P15(uint16_t raw) {
  bool bitState = (raw & (1 << 13)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState;
#else
  return bitState;
#endif
}

inline bool docTinHieuRF_P16(uint16_t raw) {
  bool bitState = (raw & (1 << 14)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState;
#else
  return bitState;
#endif
}

inline bool docTinHieuRF_P17(uint16_t raw) {
  bool bitState = (raw & (1 << 15)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState;
#else
  return bitState;
#endif
}

// ==============================================================================
// ĐIỀU KHIỂN BỘ GÕ TRỰC TIẾP QUA CHÂN PA7 CỦA STM32 BLUE PILL
// ⚡ CHUYỂN HOÀN TOÀN BỘ GÕ SANG PA7 (DIRECT GPIO, ZERO I2C TRAFFIC!)
// ==============================================================================
inline void batBoGo() { digitalWrite(PIN_BO_GO_PA7, HIGH); }
inline void tatBoGo() { digitalWrite(PIN_BO_GO_PA7, LOW); }
inline void batChanA7() { batBoGo(); }
inline void tatChanA7() { tatBoGo(); }
inline void batBoGoPA7() { batBoGo(); }
inline void tatBoGoPA7() { tatBoGo(); }

static uint32_t g_thoiGianKhoaNhietRelay = 0;
inline void datKhoaNhietRelay(uint32_t ms = 300) {
  g_thoiGianKhoaNhietRelay = millis() + ms;
}

// 1. TẮT TẤT CẢ RELAY TỪ P4 ĐẾN P14 (Kênh R2 - P15)
inline void tatTatCaRelayP4DenP14() {
  uint16_t maskXoa = 0x1FF0; // Bits 4..12 (P4..P14)
  g_pcfData &= ~maskXoa;
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// 2. CHỚP TẮT RELAY 13, RELAY 7 VÀ RELAY 10 (Kênh R3 - P16)
#define MASK_RELAY_CHOP ((1 << 7) | (1 << 8) | (1 << 11))

inline void batRelayChopRF() {
  g_pcfData |= MASK_RELAY_CHOP;
  writePCF8575(g_pcfData);
}

inline void tatRelayChopRF() {
  g_pcfData &= ~MASK_RELAY_CHOP;
  writePCF8575(g_pcfData);
}

// 3. RELAY 13 (P13: ĐÈN ĐỎ)
#define MASK_RELAY_DEN_DO (1 << 11)
inline void batRelay13() {
  g_pcfData |= MASK_RELAY_DEN_DO;
  writePCF8575(g_pcfData);
}
inline void tatRelay13() {
  g_pcfData &= ~MASK_RELAY_DEN_DO;
  writePCF8575(g_pcfData);
}

inline void pcf8575_update() {
  writePCF8575(g_pcfData);
}

// ==============================================================================
// CÁC HÀM CƠ BẢN ĐIỀU KHIỂN RELAY (CHUẨN V1)
// ==============================================================================

// Tắt tất cả các rơ-le (nếu P6 đã bật thì P6 vẫn giữ nguyên)
inline void tatTatCaRelay() {
  g_pcfData = g_p6DaBat ? (1 << 6) : 0x0000;
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// Khởi tạo bật P0 và P1, các relay khác tắt
inline void batP0_P1_TatTatCaConLai() {
  g_pcfData = (1 << RELAY_LOA_P0) | (1 << RELAY_LOA_P1);
  if (g_p6DaBat) g_pcfData |= (1 << 6);
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// Chế độ chiến thắng 'V'
inline void batCheDoChienThangV() {
  uint16_t maskV = (1 << 0) | (1 << 1) | (1 << 10); // P0, P1, P12
  if (g_p6DaBat) {
    maskV |= (1 << 6);
  }
  g_pcfData = maskV;
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// Bật 1 chân relay (Chuẩn V1)
inline void batChanPCF(uint8_t pin) {
  if (pin == 6) {
    g_p6DaBat = true;
  }

  if (pin == 14) {
    batBoGo(); // ⚡ BỘ GÕ CHẠY QUA PA7 (Không ghi I2C, không làm giật relay khác!)
    return;
  }
  else if (pin == 13) {
    g_pcfData |= (1 << 11); // P13: Bit 11
  }
  else if (pin == 12) {
    g_pcfData |= (1 << 10); // P12: Bit 10
  }
  else if (pin == 11) {
    g_pcfData |= (1 << 9);  // P11: Bit 9
  }
  else if (pin == 10) {
    g_pcfData |= (1 << 8);  // P10: Bit 8
  }
  else if (pin <= 7) {
    g_pcfData |= (1 << pin); // P0..P7: Bit 0..7
  }
  writePCF8575(g_pcfData);
}

// Tắt 1 chân relay (Chuẩn V1)
inline void tatChanPCF(uint8_t pin) {
  if (pin == 6 && g_p6DaBat) {
    return;
  }

  if (pin == 14) {
    tatBoGo(); // ⚡ TẮT BỘ GÕ QUA PA7
    return;
  }
  else if (pin == 13) {
    g_pcfData &= ~(1 << 11);
  }
  else if (pin == 12) {
    g_pcfData &= ~(1 << 10);
  }
  else if (pin == 11) {
    g_pcfData &= ~(1 << 9);
  }
  else if (pin == 10) {
    g_pcfData &= ~(1 << 8);
  }
  else if (pin <= 7) {
    g_pcfData &= ~(1 << pin);
  }
  writePCF8575(g_pcfData);
}

// Chuyển pha nguyên khối (Atomic) sau bộ gõ: TẮT TẤT CẢ RELAY, CHỈ BẬT P2
inline void chuyenPha_SauBoGo_BatDuyNhatP2() {
  uint16_t maskMoi = (1 << RELAY_LOA_P2);
  if (g_p6DaBat) {
    maskMoi |= (1 << 6);
  }
  g_pcfData = maskMoi;
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// Chuyển pha nguyên khối (Atomic): Tắt P2, Bật P0 và P1
inline void chuyenPha_TatP2_BatP0P1() {
  g_pcfData &= ~(1 << RELAY_LOA_P2);
  g_pcfData |= (1 << RELAY_LOA_P0);
  g_pcfData |= (1 << RELAY_LOA_P1);
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  datKhoaNhietRelay(300);
}

// Chuyển pha khi nhận lệnh K: Bật P11, P13, P0, P1
inline void chuyenPha_NhanLenhK_BatP11_P13_P0_P1() {
  g_pcfData |= (1 << RELAY_LOA_P0);
  g_pcfData |= (1 << RELAY_LOA_P1);
  g_pcfData |= (1 << 9);  // P11
  g_pcfData |= (1 << 11); // P13
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  datKhoaNhietRelay(1000);
}

inline void chuyenPha_Sau10sK_TatP13() {
  g_pcfData &= ~(1 << 11);
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  datKhoaNhietRelay(300);
}

inline void chuyenPha_Sau30sK_TatP13_BatP3() {
  g_pcfData &= ~(1 << 11);
  g_pcfData |= (1 << 3); // P3
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  datKhoaNhietRelay(500);
}

// ==============================================================================
// BỘ LỌC CẢM BIẾN NPN 5V (NHẤC CHAI RA): HỖ TRỢ CẢ CHÂN PA5 (V1) LẪN PB0
// ==============================================================================
enum TrangThaiLocA5 {
  A5_CHO_DAT_VAT = 0,
  A5_DA_CO_VAT = 1,
  A5_DA_KHOA_VINH_VIEN = 2
};

static TrangThaiLocA5 g_trangThaiLocA5 = A5_CHO_DAT_VAT;
static uint32_t g_thoiGianBatDauLowA5 = 0;
static uint32_t g_thoiGianBatDauHighA5 = 0;
static bool g_dangGiuLowA5 = false;
static bool g_dangGiuHighA5 = false;

inline void resetBoLocA5() {
  g_trangThaiLocA5 = A5_CHO_DAT_VAT;
  g_thoiGianBatDauLowA5 = 0;
  g_thoiGianBatDauHighA5 = 0;
  g_dangGiuLowA5 = false;
  g_dangGiuHighA5 = false;
}

// Đọc tín hiệu cảm biến nhấc chai: hỗ trợ cả chân PB0 (mặc định) và chân PA5 (dự phòng)
inline bool docMucThapCamBienChai() {
  bool pb0Low = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) || (digitalRead(PB0) == LOW);
  bool pa5Low = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) || (digitalRead(PA5) == LOW);
  return pb0Low || pa5Low;
}

inline bool kiemTraCamBienA5_PhatHienMatTinHieu() {
  if (g_trangThaiLocA5 == A5_DA_KHOA_VINH_VIEN) {
    return false;
  }

  bool pinMucThap = docMucThapCamBienChai();
  uint32_t now = millis();

  // 1. Chờ đặt chai vào (Mức LOW ổn định >= 100ms)
  if (g_trangThaiLocA5 == A5_CHO_DAT_VAT) {
    if (pinMucThap) {
      if (!g_dangGiuLowA5) {
        g_dangGiuLowA5 = true;
        g_thoiGianBatDauLowA5 = now;
      } else if (now - g_thoiGianBatDauLowA5 >= 100) {
        g_trangThaiLocA5 = A5_DA_CO_VAT;
        g_dangGiuHighA5 = false;
      }
    } else {
      g_dangGiuLowA5 = false;
    }
    return false;
  }

  // 2. Chờ nhấc chai ra (Mức HIGH ổn định >= 100ms)
  if (g_trangThaiLocA5 == A5_DA_CO_VAT) {
    if (!pinMucThap) {
      if (!g_dangGiuHighA5) {
        g_dangGiuHighA5 = true;
        g_thoiGianBatDauHighA5 = now;
      } else if (now - g_thoiGianBatDauHighA5 >= 100) {
        g_trangThaiLocA5 = A5_DA_KHOA_VINH_VIEN; // Khóa chốt 1 lần duy nhất!
        return true;
      }
    } else {
      g_dangGiuHighA5 = false;
    }
    return false;
  }

  return false;
}

inline bool kiemTraNutNhanPA5() {
  return kiemTraCamBienA5_PhatHienMatTinHieu();
}

inline bool kiemTraCamBienPB0_KichHoat() {
  return kiemTraCamBienA5_PhatHienMatTinHieu();
}

// Nút bấm cơ phụ trên PB1 kích hoạt bộ gõ (nếu có dùng test)
inline bool kiemTraNutNhanPB1_BoGo() {
  static uint32_t timerPB1 = 0;
  static bool daNhanPB1 = false;
  bool low = (digitalRead(PB1) == LOW);
  uint32_t now = millis();
  if (low) {
    if (!daNhanPB1) {
      if (timerPB1 == 0) timerPB1 = now;
      else if (now - timerPB1 >= 40) {
        daNhanPB1 = true;
        return true;
      }
    }
  } else {
    timerPB1 = 0;
    daNhanPB1 = false;
  }
  return false;
}

// ==============================================================================
// KHỞI TẠO HỆ THỐNG
// ==============================================================================
inline void khoiTaoHeThong() {
  g_p6DaBat = false;
  g_pcfData = 0x0000;
  resetBoLocA5();

  pinMode(PIN_I2C_SCL, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_I2C_SDA, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_I2C_SCL, HIGH);
  digitalWrite(PIN_I2C_SDA, HIGH);

  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStructA = {0};
  GPIO_InitStructA.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  GPIO_InitStructA.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructA.Pull = GPIO_PULLUP;
  GPIO_InitStructA.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructA);

  GPIO_InitTypeDef GPIO_InitStructB = {0};
  GPIO_InitStructB.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructB.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructB.Pull = GPIO_PULLUP;
  GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructB);

  pinMode(PA6, INPUT_PULLUP);
  pinMode(PA5, INPUT_PULLUP);
  pinMode(PA4, INPUT_PULLUP);
  pinMode(PB0, INPUT_PULLUP);
  pinMode(PB1, INPUT_PULLUP);

  pinMode(PIN_OUTPUT_A7, OUTPUT);
  tatChanA7();

  SoundSerial.begin(UART_BAUDRATE);
  pinMode(PA3, INPUT_PULLUP); // Chống nhiễu RX

  batP0_P1_TatTatCaConLai();
}

// Kiểm tra nút phục hồi PA4
inline bool kiemTraNutNhanPA4() {
  bool mucThap = (digitalRead(PIN_BTN_KHOI_PHUC) == LOW);
  if (mucThap) {
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      if (digitalRead(PIN_BTN_KHOI_PHUC) != LOW) return false;
    }
    return true;
  }
  return false;
}

// Kiểm tra chân PA6 (Cái xe)
inline bool kiemTraNutNhanPA6() {
  bool mucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) || 
                 (digitalRead(PA6) == LOW);
  if (mucThap) {
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      bool vanMucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) || 
                        (digitalRead(PA6) == LOW);
      if (!vanMucThap) return false;
    }
    return true;
  }
  return false;
}

inline void led_bat() { digitalWrite(PIN_LED_STATUS, LOW); }
inline void led_tat() { digitalWrite(PIN_LED_STATUS, HIGH); }

inline void guiLenhSound(char cmd) {
  SoundSerial.print(cmd);
  SoundSerial.flush();
}

inline char docLenhSound() {
  if (SoundSerial.available() > 0) {
    return (char)SoundSerial.read();
  }
  return 0;
}
