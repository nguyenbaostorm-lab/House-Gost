#pragma once
#include <Arduino.h>
#include "config.h"

// ==============================================================================
// THƯ VIỆN ĐIỀU KHIỂN PCF8575 I2C OPEN-DRAIN CHUẨN XÁC THEO PHẦN CỨNG
// ==============================================================================

// Biến lưu trạng thái 16-bit của PCF8575
static uint16_t g_pcfData = 0x0000;
// 🔥 CHỐT P6: Nếu P6 (Cửa từ 1) đã được bật thì KHÔNG BAO GIỜ TẮT LẠI!
static bool g_p6DaBat = false;

// Cổng UART USART2 phần cứng: PA2 (TX), PA3 (RX) giao tiếp với bo Sound
#define SoundSerial Serial2

// ------------------------------------------------------------------------------
// GIAO TIẾP I2C OPEN-DRAIN CHUẨN
// ------------------------------------------------------------------------------
static inline void i2c_delay() { delayMicroseconds(6); } // 6us: Tần số ~80kHz chuẩn I2C, xung vuông cực đẹp và chống suy hao đường truyền xa
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

// Hàm ghi 16-bit dữ liệu tới PCF8575 (Địa chỉ 0x27) kèm khóa ngắt bảo vệ & cơ chế Retry 3 lần
inline bool writePCF8575(uint16_t data) {
  // 🔥 ÉP BUỘC P15, P16, P17 LUÔN MỨC 1 (INPUT OPEN-DRAIN ĐỂ ĐỌC TÍN HIỆU RF433)
  data |= MASK_RF_INPUT_PINS;
  // 🔥 CHỐT P6: NẾU ĐÃ BẬT THÌ KHÔNG BAO GIỜ TẮT LẠI!
  if (g_p6DaBat) {
    data |= (1 << 6);
    g_pcfData |= (1 << 6);
  }

  bool thanhCong = false;
  for (uint8_t retry = 0; retry < 3; retry++) {
    noInterrupts(); // Khóa ngắt toàn cục: đảm bảo SysTick/UART không làm biến dạng xung I2C
    i2c_start();
    bool ack1 = i2c_write_byte(PCF8575_ADDR << 1);            // Địa chỉ ghi (0x27 << 1 = 0x4E)
    bool ack2 = i2c_write_byte((uint8_t)(data & 0xFF));         // Byte thấp (P00 - P07)
    bool ack3 = i2c_write_byte((uint8_t)((data >> 8) & 0xFF)); // Byte cao (P10 - P17)
    i2c_stop();
    interrupts();   // Mở lại ngắt

    if (ack1 && ack2 && ack3) {
      thanhCong = true;
      break;
    }
    delayMicroseconds(50); // Nghỉ 50us trước khi thử lại nếu bị xung nhiễu chập chờn
  }
  return thanhCong;
}

// Hàm đọc 16-bit dữ liệu từ PCF8575 (Địa chỉ 0x27, chế độ Read 0x4F)
inline uint16_t readPCF8575() {
  noInterrupts(); // Khóa ngắt bảo vệ chu trình đọc dữ liệu
  i2c_start();
  if (!i2c_write_byte((PCF8575_ADDR << 1) | 0x01)) { // 0x27 << 1 | 1 = 0x4F
    i2c_stop();
    interrupts();
    return 0xFFFF; // Lỗi truyền thông (NACK)
  }

  // 1. Đọc Byte thấp (P00 - P07)
  uint8_t lowByte = 0;
  sda_high(); // Thả SDA ở chế độ Open-Drain
  for (uint8_t i = 0; i < 8; i++) {
    lowByte <<= 1;
    scl_high();
    i2c_delay();
    if (digitalRead(PIN_I2C_SDA) == HIGH) lowByte |= 1;
    scl_low();
    i2c_delay();
  }
  // Gửi Master ACK (kéo SDA xuống LOW) để đọc tiếp Byte cao
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
  // Gửi Master NACK (giữ SDA HIGH) để kết thúc đọc
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
// CÁC HÀM XỬ LÝ ĐẶC THÙ CHO 3 KÊNH RF433 (P15, P16, P17)
// ==============================================================================

// P15 (Kênh R2): Bit 13
inline bool docTinHieuRF_P15(uint16_t raw) {
  bool bitState = (raw & (1 << 13)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState; // Active LOW: 0V là có bấm remote
#else
  return bitState;
#endif
}

// P16 (Kênh R3): Bit 14
inline bool docTinHieuRF_P16(uint16_t raw) {
  bool bitState = (raw & (1 << 14)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState;
#else
  return bitState;
#endif
}

// P17 (Kênh R4): Bit 15
inline bool docTinHieuRF_P17(uint16_t raw) {
  bool bitState = (raw & (1 << 15)) != 0;
#if RF_ACTIVE_LEVEL == 0
  return !bitState;
#else
  return bitState;
#endif
}

// ==============================================================================
// ĐIỀU KHIỂN CHÂN XUẤT TÍN HIỆU A7 (PA7) TRÊN STM32 BLUE PILL
// ==============================================================================
inline void batChanA7() {
  digitalWrite(PIN_OUTPUT_A7, MUC_A7_ON);
}

inline void tatChanA7() {
  digitalWrite(PIN_OUTPUT_A7, MUC_A7_OFF);
}

// 1. TẮT TẤT CẢ RELAY TỪ P4 ĐẾN P14 (Kênh R2 - P15)
// Cửa sổ khóa chống xung cuộn dây Relay (EMI Blanking Window):
// Khi có sự kiện đóng ngắt rơ-le lớn, tạm ngưng đọc RF trong 300ms để triệt tiêu hoàn toàn xung nhiễu
static uint32_t g_thoiGianKhoaNhietRelay = 0;

inline void datKhoaNhietRelay(uint32_t ms = 300) {
  g_thoiGianKhoaNhietRelay = millis() + ms;
}

// 1. TẮT TẤT CẢ RELAY TỪ P4 ĐẾN P14 (Kênh R2 - P15)
// Giữ nguyên Loa P0, P1, P2 luôn BẬT; tắt sạch từ bit 4 đến bit 12 (P4..P14)
// TUYỆT ĐỐI không xóa bit 13, 14, 15 (là các chân Input RF)
inline void tatTatCaRelayP4DenP14() {
  uint16_t maskXoa = 0x1FF0; // Bits 4..12 (P4..P14)
  g_pcfData &= ~maskXoa;
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6); // P6 đã bật thì không bao giờ tắt lại
  }
  writePCF8575(g_pcfData);
  tatChanA7();
  datKhoaNhietRelay(300);
}

// 2. CHỚP TẮT RELAY 13, RELAY 7 VÀ RELAY 10 (Kênh R3 - P16)
// Relay 7 = Bit 7, Relay 10 = Bit 8, Relay 13 = Bit 11
#define MASK_RELAY_CHOP ((1 << 7) | (1 << 8) | (1 << 11))

inline void batRelayChopRF() {
  g_pcfData |= MASK_RELAY_CHOP;
  writePCF8575(g_pcfData);
}

inline void tatRelayChopRF() {
  g_pcfData &= ~MASK_RELAY_CHOP;
  writePCF8575(g_pcfData);
}

// Mặt nạ toàn bộ các đèn trên PCF8575:
// Bit 3 (P3), Bit 4 (P4 TV), Bit 5 (P5 Đồ chơi), Bit 7 (P7 Tranh), Bit 8 (P10 Rồng phượng), Bit 10 (P12 Cuốn sổ)
#define MASK_TAT_CA_DEN ((1 << 3) | (1 << 4) | (1 << 5) | (1 << 7) | (1 << 8) | (1 << 10))

// 3. RELAY 13 (P13: ĐÈN ĐỎ) BẬT KHI NHẤN R4 (P17), KHÔNG NHẤN THÌ TẮT
// Relay 13 = Bit 11
#define MASK_RELAY_DEN_DO (1 << 11)
#define MASK_RELAY_RF_P13 MASK_RELAY_DEN_DO

inline void batRelay13() {
  g_pcfData |= MASK_RELAY_DEN_DO;
  writePCF8575(g_pcfData);
}

inline void tatRelay13() {
  g_pcfData &= ~MASK_RELAY_DEN_DO;
  writePCF8575(g_pcfData);
}

// Bí danh điều khiển ĐÈN ĐỎ (Relay 13 / P13)
inline void batRelayDenDo() { batRelay13(); }
inline void tatRelayDenDo() { tatRelay13(); }

// Giữ alias tương thích nếu có code cũ gọi tới
inline void batRelay10() { batRelay13(); }
inline void tatRelay10() { tatRelay13(); }

// Cập nhật giá trị biến trạng thái ra chip PCF8575
inline void pcf8575_update() {
  writePCF8575(g_pcfData);
}

// ==============================================================================
// CÁC HÀM CƠ BẢN ĐIỀU KHIỂN RELAY
// Ánh xạ chân chuẩn 1-đối-1 theo đúng ký hiệu in trên mạch PCF8575:
// - P00..P07: Bit 0 -> 7
// - P10..P14: Bit 8 -> 12 (P10 = Bit 8, P11 = Bit 9, P12 = Bit 10, P13 = Bit 11, P14 = Bit 12)
// - P15..P17: Bit 13 -> 15 (Chân RF Input, TUYỆT ĐỐI không ghi đè thành Output)
// ==============================================================================

// 1. TẮT TẤT CẢ các rơ-le (nếu P6 đã bật thì P6 vẫn giữ nguyên)
inline void tatTatCaRelay() {
  g_pcfData = g_p6DaBat ? (1 << 6) : 0x0000;
  writePCF8575(g_pcfData);
  tatChanA7(); // Đảm bảo chân A7 cũng tắt an toàn
  datKhoaNhietRelay(300);
}

// 2. BẬT TẤT CẢ các rơ-le (chỉ bật P0..P14, giữ P15..P17 là Input)
inline void batTatCaRelay() {
  g_pcfData = 0x1FFF; // Bật Bit 0..12
  writePCF8575(g_pcfData);
  datKhoaNhietRelay(300);
}

// 2b. BẬT CHẾ ĐỘ CHIẾN THẮNG 'V': BẬT P12, P0, P1; giữ nguyên P6 (nếu đã bật); TẮT toàn bộ relay khác & chân A7
inline void batCheDoChienThangV() {
  uint16_t maskV = (1 << 0) | (1 << 1) | (1 << 10); // P0: Bit 0, P1: Bit 1, P12: Bit 10
  if (g_p6DaBat) {
    maskV |= (1 << 6); // Giữ nguyên P6 cửa từ 1 (Bit 6) nếu đã mở chốt
  }
  g_pcfData = maskV;
  writePCF8575(g_pcfData);
  tatChanA7(); // Đảm bảo chân A7 tắt
  datKhoaNhietRelay(300);
}

// 3. BẬT chân relay trên PCF8575
inline void batChanPCF(uint8_t pin) {
  // Chốt P6: Đánh dấu P6 đã bật, từ nay không bao giờ tắt lại!
  if (pin == 6) {
    g_p6DaBat = true;
  }

  if (pin == 14) {
    g_pcfData |= (1 << 12); // P14: Bit 12
  }
  else if (pin == 13) {
    g_pcfData |= (1 << 11); // P13: Bit 11
  }
  else if (pin == 12) {
    g_pcfData |= (1 << 10); // P12: Bit 10
  }
  else if (pin == 11 || pin == 9) {
    g_pcfData |= (1 << 9);  // P11: Bit 9
  }
  else if (pin == 10 || pin == 8) {
    g_pcfData |= (1 << 8);  // P10: Bit 8
  }
  else if (pin <= 7) {
    g_pcfData |= (1 << pin); // P0..P7: Bit 0..7
  }
  writePCF8575(g_pcfData);
}

// 4. TẮT chân relay trên PCF8575
inline void tatChanPCF(uint8_t pin) {
  // 🔥 CHẶN TẮT: P6 NẾU ĐÃ BẬT THÌ KHÔNG BAO GIỜ TẮT LẠI VÌ BẤT KỲ LÝ DO GÌ
  if (pin == 6 && g_p6DaBat) {
    return;
  }

  if (pin == 14) {
    g_pcfData &= ~(1 << 12);
  }
  else if (pin == 13) {
    g_pcfData &= ~(1 << 11);
  }
  else if (pin == 12) {
    g_pcfData &= ~(1 << 10);
  }
  else if (pin == 11 || pin == 9) {
    g_pcfData &= ~(1 << 9);
  }
  else if (pin == 10 || pin == 8) {
    g_pcfData &= ~(1 << 8);
  }
  else if (pin <= 7) {
    g_pcfData &= ~(1 << pin);
  }
  writePCF8575(g_pcfData);
}

// 5. CHUYỂN PHA NGUYÊN KHỐI (ATOMIC): TẮT P2, BẬT LẠI P0 VÀ P1
// Ghi đúng 1 lệnh I2C duy nhất, loại bỏ hoàn toàn hiện tượng relay giật / kích chéo rơ-le khác!
inline void chuyenPha_TatP2_BatP0P1() {
  g_pcfData &= ~(1 << RELAY_LOA_P2); // TẮT P2 (Bit 2)
  g_pcfData |= (1 << RELAY_LOA_P0);  // BẬT P0 (Bit 0)
  g_pcfData |= (1 << RELAY_LOA_P1);  // BẬT P1 (Bit 1)
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  delay(5);
  writePCF8575(g_pcfData); // Ghi kép bảo vệ
  datKhoaNhietRelay(1000); // Khóa chống nhiễu cuộn dây 1000ms
}

// 6. CHUYỂN PHA NGUYÊN KHỐI (ATOMIC): KHI NHẬN LỆNH K TỪ STM32_SOUND (BƯỚC 23)
// 🎯 BẬT P11 (Bit 9: Búp bê rơi)
// 🎯 BẬT P13 (Bit 11: Đèn đỏ)
// 🎯 DUY TRÌ BẬT P0 (Bit 0: Loa tổng) và P1 (Bit 1: Loa trên gác)
// 🎯 Giữ nguyên P6 nếu đã mở
// 🛡️ Ghi đúng 1 khung I2C duy nhất + Double-Write + Khóa chống xung nhiễu cuộn dây 2000ms (2 giây)!
// Triệt tiêu 100% hiện tượng xung cảm ứng làm nhảy giật các rơ-le khác!
inline void chuyenPha_NhanLenhK_BatP11_P13_P0_P1() {
  g_pcfData |= (1 << RELAY_LOA_P0);         // BẬT P0 (Bit 0: Loa tổng)
  g_pcfData |= (1 << RELAY_LOA_P1);         // BẬT P1 (Bit 1: Loa trên gác)
  g_pcfData |= (1 << 9);                    // BẬT P11 (Bit 9: Búp bê rơi)
  g_pcfData |= (1 << 11);                   // BẬT P13 (Bit 11: Đèn đỏ)
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);                  // Duy trì P6 nếu đã mở chốt
  }
  writePCF8575(g_pcfData);
  delay(5);
  writePCF8575(g_pcfData);                  // Ghi kép (Double-write) xác nhận chốt cứng trạng thái
  datKhoaNhietRelay(2000);                  // Khóa toàn diện chống xung nhiễu cuộn dây & tải cảm ứng 2000ms (2s)
}

// 7a. CHUYỂN PHA NGUYÊN KHỐI (ATOMIC): SAU 10S TỪ LỆNH K (BƯỚC 24a)
// 🎯 TẮT chân P13 (Bit 11: Đèn đỏ)
// 🎯 Duy trì P11 (Búp bê rơi), P0, P1 và P6 (nếu đã mở)
inline void chuyenPha_Sau10sK_TatP13() {
  g_pcfData &= ~(1 << 11);                  // TẮT P13 (Bit 11: Đèn đỏ)
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  delay(5);
  writePCF8575(g_pcfData);                  // Ghi kép chốt cứng
  datKhoaNhietRelay(300);                   // Khóa chống nhiễu cuộn dây 300ms
}

// 7b. CHUYỂN PHA NGUYÊN KHỐI (ATOMIC): SAU 30S TỪ LỆNH K (10S BẬT + 20S TẮT - BƯỚC 25)
// 🎯 TẮT chân P13 (Bit 11: Đèn đỏ)
// 🎯 BẬT chân P3 TRỞ LẠI (Bit 3: Đèn quan sát)
// 🎯 Duy trì P11 (Búp bê rơi), P0, P1 và P6 (nếu đã mở)
inline void chuyenPha_Sau30sK_TatP13_BatP3() {
  g_pcfData &= ~(1 << 11);                  // TẮT P13 (Bit 11)
  g_pcfData |= (1 << 3);                    // BẬT P3 (Bit 3)
  if (g_p6DaBat) {
    g_pcfData |= (1 << 6);
  }
  writePCF8575(g_pcfData);
  delay(5);
  writePCF8575(g_pcfData);                  // Ghi kép chốt cứng
  datKhoaNhietRelay(1000);                  // Khóa chống nhiễu cuộn dây 1000ms
}

// Các hàm alias tương thích
inline void relay_on(uint8_t pin)  { batChanPCF(pin); }
inline void relay_off(uint8_t pin) { tatChanPCF(pin); }
inline void batRelay(uint8_t num)  { if (num >= 1 && num <= TOTAL_RELAYS) batChanPCF(num - 1); }
inline void tatRelay(uint8_t num)  { if (num >= 1 && num <= TOTAL_RELAYS) tatChanPCF(num - 1); }

// ==============================================================================
// BỘ LỌC SỐ CHỐNG DỘI / CHỐNG NHIỄU CAO CẤP CHO CẢM BIẾN NPN 5V CHÂN PA5 (A5):
// - Chân PA5 kéo lên nội bộ INPUT_PULLUP (Mức nghỉ CAO / 3.3V).
// - Khi ĐẶT VẬT vào: Cảm biến NPN kéo chân xuống mức THẤP (LOW / GND).
//   -> Phải giữ mức THẤP liên tục ổn định >= 150ms mới xác nhận là ĐÃ CÓ VẬT!
// - Khi NHẤC VẬT RA (MẤT TÍN HIỆU): Chân PA5 trở lại mức CAO (HIGH).
//   -> Phải giữ mức CAO liên tục ổn định >= 150ms mới xác nhận là THỰC SỰ NHẤC VẬT!
// - ⚡ CHỐT CHẶN KHÓA CỨNG (ONE-SHOT HARD LOCK):
//   Sau khi đã phát hiện nhấc vật thành công 1 lần:
//   Khóa chốt vĩnh viễn, từ đó về sau KHÔNG BAO GIỜ nhận lại tín hiệu A5 nữa,
//   loại bỏ 100% tình trạng nhận qua lại, gửi lặp lệnh làm treo/sập hệ thống!
// ==============================================================================

enum TrangThaiLocA5 {
  A5_CHO_DAT_VAT = 0,      // Ban đầu: Chờ đặt vật vào (mức LOW ổn định)
  A5_DA_CO_VAT = 1,        // Đã có vật ổn định: Chờ nhấc vật ra (mức HIGH ổn định)
  A5_DA_KHOA_VINH_VIEN = 2 // Đã kích hoạt 1 lần: Khóa vĩnh viễn không nhận lại
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

// ==============================================================================
// KHỞI TẠO VÀ GIAO TIẾP
// ==============================================================================

inline void khoiTaoHeThong() {
  // 🔥 KHỞI TẠO BAN ĐẦU: P6 (CỬA TỪ 1) TUYỆT ĐỐI CHƯA KÍCH HOẠT (TẮT HOÀN TOÀN)
  g_p6DaBat = false;
  g_pcfData = 0x0000;
  resetBoLocA5();

  // Cấu hình chân I2C ở chế độ Open-Drain chuẩn
  pinMode(PIN_I2C_SCL, OUTPUT_OPEN_DRAIN);
  pinMode(PIN_I2C_SDA, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_I2C_SCL, HIGH);
  digitalWrite(PIN_I2C_SDA, HIGH);

  // Đèn LED trạng thái PC13
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  // Bật Clock và cấu hình chân nút nhấn PA6 (A6) và PB0 (Cảm biến NPN 5V bộ gõ) chuẩn HAL lẫn Arduino
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitTypeDef GPIO_InitStructB = {0};
  GPIO_InitStructB.Pin = GPIO_PIN_0;
  GPIO_InitStructB.Mode = GPIO_MODE_INPUT;
  GPIO_InitStructB.Pull = GPIO_PULLUP;
  GPIO_InitStructB.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructB);

  pinMode(PIN_BUTTON_OFF_R5, INPUT_PULLUP);
  pinMode(PA6, INPUT_PULLUP);
  pinMode(PIN_BUTTON_BO_GO_A5, INPUT_PULLUP); // PB0
  pinMode(PB0, INPUT_PULLUP);

  // Cấu hình chân nút nhấn Khôi phục khi cúp điện (PA4)
  pinMode(PIN_BTN_KHOI_PHUC, INPUT_PULLUP);

  // Cấu hình chân xuất tín hiệu A7 (PA7)
  pinMode(PIN_OUTPUT_A7, OUTPUT);
  tatChanA7();

  // Mở cổng UART giao tiếp với bo Sound
  SoundSerial.begin(UART_BAUDRATE);

  // ÉP TẮT SẠCH TOÀN BỘ 16 RELAY (0x0000)
  tatTatCaRelay();
}

// Hàm kiểm tra nút nhấn khôi phục cúp điện ở chân PA4 (kéo lên Pull-up, nối mass GND)
inline bool kiemTraNutNhanPA4() {
  bool mucThap = (digitalRead(PIN_BTN_KHOI_PHUC) == LOW);
  if (mucThap) {
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      if (digitalRead(PIN_BTN_KHOI_PHUC) != LOW) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// Hàm kiểm tra cảm biến / nút nhấn nối GND ở chân PA6 (A6):
// - Chân PA6 kéo lên Pull-Up nội bộ (bình thường mức HIGH).
// - Khi nhận tín hiệu kích âm GND (Mức LOW): lọc chống dội kỹ qua 5 lần đọc liên tiếp trong 50ms
//   để loại bỏ hoàn toàn mọi xung nhiễu điện từ / xung khởi động nguồn.
inline bool kiemTraNutNhanPA6() {
  bool mucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) || 
                 (digitalRead(PIN_BUTTON_OFF_R5) == LOW);
  if (mucThap) {
    // Kiểm tra liên tục 5 mẫu cách nhau 10ms (tổng 50ms)
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      bool vanMucThap = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) || 
                        (digitalRead(PIN_BUTTON_OFF_R5) == LOW);
      if (!vanMucThap) {
        return false; // Nếu bị ngắt quãng do xung gai nhiễu -> hủy, không kích hoạt
      }
    }
    return true; // Thực sự có tín hiệu kích âm GND mức LOW ổn định
  }
  return false;
}

// ==============================================================================
// HÀM KIỂM TRA CẢM BIẾN NPN 5V CHÂN PA5 (A5)
// ==============================================================================
inline bool kiemTraCamBienA5_PhatHienMatTinHieu() {
  // 1. Nếu đã kích hoạt rồi -> KHÓA CỨNG VĨNH VIỄN, thoát ngay lập tức!
  if (g_trangThaiLocA5 == A5_DA_KHOA_VINH_VIEN) {
    return false;
  }

  // Đọc trạng thái chân PB0 (Mức LOW = có vật, mức HIGH = không có vật)
  bool pinMucThap = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET) || 
                    (digitalRead(PIN_BUTTON_BO_GO_A5) == LOW);
  uint32_t now = millis();

  // 2. Giai đoạn 1: CHỜ ĐẶT VẬT VÀO (Mức THẤP ổn định liên tục >= 150ms)
  if (g_trangThaiLocA5 == A5_CHO_DAT_VAT) {
    if (pinMucThap) {
      if (!g_dangGiuLowA5) {
        g_dangGiuLowA5 = true;
        g_thoiGianBatDauLowA5 = now;
      } else if (now - g_thoiGianBatDauLowA5 >= 150) {
        // Đã giữ mức LOW liên tục đủ 150ms -> Xác nhận ĐÃ CÓ VẬT ĐẶT THÀNH CÔNG!
        g_trangThaiLocA5 = A5_DA_CO_VAT;
        g_dangGiuHighA5 = false;
      }
    } else {
      // Bị ngắt quãng lên HIGH -> Reset bộ đếm LOW (chống xung nhiễu thoáng qua)
      g_dangGiuLowA5 = false;
    }
    return false;
  }

  // 3. Giai đoạn 2: ĐÃ CÓ VẬT -> CHỜ NHẤC VẬT RA (Mất tín hiệu, mức CAO ổn định >= 150ms)
  if (g_trangThaiLocA5 == A5_DA_CO_VAT) {
    if (!pinMucThap) {
      // Chân đã trở về mức CAO (nhấc vật ra)
      if (!g_dangGiuHighA5) {
        g_dangGiuHighA5 = true;
        g_thoiGianBatDauHighA5 = now;
      } else if (now - g_thoiGianBatDauHighA5 >= 150) {
        // Đã giữ mức CAO liên tục đủ 150ms -> Xác nhận THỰC SỰ ĐÃ NHẤC VẬT RA!
        // ⚡ KHÓA CHỐT VĨNH VIỄN NGAY TẠI ĐÂY:
        g_trangThaiLocA5 = A5_DA_KHOA_VINH_VIEN;
        return true; // 👉 KÍCH HOẠT CHU TRÌNH ĐÚNG 1 LẦN DUY NHẤT!
      }
    } else {
      // Vẫn còn đặt vật hoặc bị rung lắc về LOW -> Reset bộ đếm HIGH
      g_dangGiuHighA5 = false;
    }
    return false;
  }

  return false;
}

// Giữ alias tương thích
inline bool kiemTraNutNhanPA5() {
  return kiemTraCamBienA5_PhatHienMatTinHieu();
}

// Giữ alias tương thích nếu có đoạn code cũ gọi tới
inline bool kiemTraNutNhanPA8() {
  return kiemTraNutNhanPA6();
}

// Điều khiển trực tiếp đèn LED PC13
inline void led_bat() { digitalWrite(PIN_LED_STATUS, LOW); }  // PC13 Active-LOW: mức LOW = SÁNG
inline void led_tat() { digitalWrite(PIN_LED_STATUS, HIGH); } // PC13 Active-LOW: mức HIGH = TẮT

// Hàm nhấp nháy LED trạng thái bo PC13
inline void nhapNhayLed(uint32_t ms = TOC_DO_NHAY_CHAM_MS) {
  static unsigned long last = 0;
  if (millis() - last >= ms) {
    last = millis();
    digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
  }
}

// Gửi lệnh sang bo Sound
inline void guiLenhSound(char cmd) {
  SoundSerial.print(cmd);
  SoundSerial.flush();
}

// Đọc lệnh từ bo Sound
inline char docLenhSound() {
  if (SoundSerial.available() > 0) {
    return (char)SoundSerial.read();
  }
  return 0;
}
