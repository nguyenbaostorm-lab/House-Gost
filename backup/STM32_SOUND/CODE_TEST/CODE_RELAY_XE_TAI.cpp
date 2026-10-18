#include <Arduino.h>

// ==============================================================================
// 🚗 CHƯƠNG TRÌNH TEST RELAY KIỂU XE TẢI - BO STM32_SOUND (7 RELAY PA0 -> PA6)
// ==============================================================================
// Cấu hình chuẩn theo tài liệu README_SOUND.md & config.h:
// - 7 Relay: PA0, PA1, PA2, PA3, PA4, PA5, PA6
// - Chuẩn kích: Active-HIGH (Mức HIGH = 1 = Bật relay; Mức LOW = 0 = Tắt relay)
// - Đèn báo trạng thái bo: PC13
// ==============================================================================

#define PIN_RELAY_1      PA0
#define PIN_RELAY_2      PA1
#define PIN_RELAY_3      PA2
#define PIN_RELAY_4      PA3
#define PIN_RELAY_5      PA4
#define PIN_RELAY_6      PA5
#define PIN_RELAY_7      PA6
#define TOTAL_RELAYS     7

#define RELAY_ON         1  // HIGH: Bật relay
#define RELAY_OFF        0  // LOW: Tắt relay

#define PIN_LED_STATUS   PC13

static const uint32_t RELAY_PINS[TOTAL_RELAYS] = {
  PIN_RELAY_1, PIN_RELAY_2, PIN_RELAY_3, PIN_RELAY_4,
  PIN_RELAY_5, PIN_RELAY_6, PIN_RELAY_7
};

// ==============================================================================
// CÁC HÀM CƠ BẢN
// ==============================================================================
inline void tatTatCaRelay() {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }
}

inline void batTatCaRelay() {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_ON);
  }
}

inline void datRelay(uint8_t index, bool bat) {
  if (index < TOTAL_RELAYS) {
    digitalWrite(RELAY_PINS[index], bat ? RELAY_ON : RELAY_OFF);
  }
}

inline void nhapNhayLed() {
  static unsigned long last = 0;
  if (millis() - last >= 100) {
    last = millis();
    digitalWrite(PIN_LED_STATUS, !digitalRead(PIN_LED_STATUS));
  }
}

// ==============================================================================
// BỘ HIỆU ỨNG RELAY KIỂU XE TẢI (ĐUỔI CHẠY, VÀO RA, DỒN TÍCH LŨY, CHỚP SO LE)
// ==============================================================================

// 1. HIỆU ỨNG: Đuổi một chiều Trái sang Phải (PA0 -> PA6)
void hieuUng_DuoiTraiSangPhai(int speedMs) {
  tatTatCaRelay();
  for (int i = 0; i < TOTAL_RELAYS; i++) {
    nhapNhayLed();
    datRelay(i, true);
    delay(speedMs);
    datRelay(i, false);
  }
  tatTatCaRelay();
}

// 2. HIỆU ỨNG: Đuổi một chiều Phải sang Trái (PA6 -> PA0)
void hieuUng_DuoiPhaiSangTrai(int speedMs) {
  tatTatCaRelay();
  for (int i = TOTAL_RELAYS - 1; i >= 0; i--) {
    nhapNhayLed();
    datRelay(i, true);
    delay(speedMs);
    datRelay(i, false);
  }
  tatTatCaRelay();
}

// 3. HIỆU ỨNG: Quét qua lại sao băng (Ping-pong / KITT Scanner)
void hieuUng_QuetSaoBang(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    for (int i = 0; i < TOTAL_RELAYS; i++) {
      nhapNhayLed();
      datRelay(i, true);
      delay(speedMs);
      datRelay(i, false);
    }
    for (int i = TOTAL_RELAYS - 2; i > 0; i--) {
      nhapNhayLed();
      datRelay(i, true);
      delay(speedMs);
      datRelay(i, false);
    }
  }
  tatTatCaRelay();
}

// 4. HIỆU ỨNG: Từ trong tâm chạy bung ra ngoài (Vào Ra - Center to Out)
void hieuUng_TuTrongRaNgoai(int speedMs) {
  tatTatCaRelay();
  int center = (TOTAL_RELAYS - 1) / 2; // Với 7 relay, tâm là relay index 3 (PA3)
  for (int dist = 0; dist <= center; dist++) {
    nhapNhayLed();
    int leftIdx = center - dist;
    int rightIdx = center + dist;
    if (leftIdx >= 0) datRelay(leftIdx, true);
    if (rightIdx < TOTAL_RELAYS) datRelay(rightIdx, true);
    delay(speedMs);
    if (leftIdx >= 0) datRelay(leftIdx, false);
    if (rightIdx < TOTAL_RELAYS) datRelay(rightIdx, false);
  }
  tatTatCaRelay();
}

// 5. HIỆU ỨNG: Từ hai đầu chạy dồn vào trong tâm (Ngoại vào Trong - Out to Center)
void hieuUng_TuNgoaiVaoTrong(int speedMs) {
  tatTatCaRelay();
  int center = (TOTAL_RELAYS - 1) / 2;
  for (int dist = center; dist >= 0; dist--) {
    nhapNhayLed();
    int leftIdx = center - dist;
    int rightIdx = center + dist;
    if (leftIdx >= 0) datRelay(leftIdx, true);
    if (rightIdx < TOTAL_RELAYS) datRelay(rightIdx, true);
    delay(speedMs);
    if (leftIdx >= 0) datRelay(leftIdx, false);
    if (rightIdx < TOTAL_RELAYS) datRelay(rightIdx, false);
  }
  tatTatCaRelay();
}

// 6. HIỆU ỨNG: Dồn tích lũy (Bóng chạy đến cuối rồi giữ sáng lần lượt)
void hieuUng_DonTichLuy(int speedMs) {
  tatTatCaRelay();
  for (int top = TOTAL_RELAYS - 1; top >= 0; top--) {
    for (int i = 0; i <= top; i++) {
      nhapNhayLed();
      datRelay(i, true);
      delay(speedMs);
      if (i < top) {
        datRelay(i, false);
      }
    }
  }
  delay(300);
}

// 7. HIỆU ỨNG: Xả dần (Tắt lần lượt từng relay từ đầu đến cuối)
void hieuUng_XaDan(int speedMs) {
  for (int i = 0; i < TOTAL_RELAYS; i++) {
    nhapNhayLed();
    datRelay(i, false);
    delay(speedMs);
  }
  tatTatCaRelay();
}

// 8. HIỆU ỨNG: Chớp nháy so le (Chẵn / Lẻ luân phiên)
void hieuUng_ChopSoLe(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    nhapNhayLed();
    // Bật lẻ, tắt chẵn
    for (int i = 0; i < TOTAL_RELAYS; i++) {
      datRelay(i, (i % 2 == 0));
    }
    delay(speedMs);
    // Bật chẵn, tắt lẻ
    for (int i = 0; i < TOTAL_RELAYS; i++) {
      datRelay(i, (i % 2 != 0));
    }
    delay(speedMs);
  }
  tatTatCaRelay();
}

// 9. HIỆU ỨNG: Chớp nháy tổng lực dồn dập (Strobe xe tải cảnh báo)
void hieuUng_StrobeTongLuc(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    nhapNhayLed();
    batTatCaRelay();
    delay(speedMs);
    tatTatCaRelay();
    delay(speedMs);
  }
  tatTatCaRelay();
}

// ==============================================================================
// KHỞI TẠO HỆ THỐNG
// ==============================================================================
void setup() {
  // Giải phóng triệt để USART2 trên PA2/PA3 để chân PA2 hoàn toàn là chân GPIO của Relay 3
  Serial.end();

  // Cấu hình LED trạng thái bo PC13
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);

  // Cấu hình 7 chân Relay ở chế độ OUTPUT và xuất mức OFF ngay từ đầu
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }

  // Chớp nháy toàn bộ 2 nhịp chào mừng
  for (int i = 0; i < 2; i++) {
    batTatCaRelay();
    delay(150);
    tatTatCaRelay();
    delay(150);
  }
  delay(400);
}

// ==============================================================================
// VÒNG LẶP CHÍNH: CHẠY TUẦN HOÀN BỘ HIỆU ỨNG XE TẢI
// ==============================================================================
void loop() {
  // 1. Chạy đuổi Trái sang Phải (2 lượt)
  for (int i = 0; i < 2; i++) hieuUng_DuoiTraiSangPhai(80);
  delay(100);

  // 2. Chạy đuổi Phải sang Trái (2 lượt)
  for (int i = 0; i < 2; i++) hieuUng_DuoiPhaiSangTrai(80);
  delay(100);

  // 3. Quét sao băng qua lại (2 lượt)
  hieuUng_QuetSaoBang(70, 2);
  delay(100);

  // 4. Từ trong tâm chạy bung ra ngoài (3 lượt)
  for (int i = 0; i < 3; i++) {
    hieuUng_TuTrongRaNgoai(90);
    delay(60);
  }
  delay(100);

  // 5. Từ hai đầu chạy dồn vào trong tâm (3 lượt)
  for (int i = 0; i < 3; i++) {
    hieuUng_TuNgoaiVaoTrong(90);
    delay(60);
  }
  delay(100);

  // 6. Dồn tích lũy toàn bộ 7 relay
  hieuUng_DonTichLuy(70);
  delay(200);

  // 7. Xả dần từng relay
  hieuUng_XaDan(90);
  delay(150);

  // 8. Chớp so le chẵn lẻ (5 nhịp)
  hieuUng_ChopSoLe(120, 5);
  delay(150);

  // 9. Strobe cảnh báo tổng lực (6 nhịp chớp nhanh)
  hieuUng_StrobeTongLuc(70, 6);
  delay(500); // Nghỉ 0.5s rồi lặp lại chu kỳ
}
