#include <Arduino.h>
#include "config.h"
#include "pcf8575_relay.h"

// ==============================================================================
// 🎭 CHƯƠNG TRÌNH BIỂU DIỄN HIỆU ỨNG DÀN 13 RELAY (MODULE I2C PCF8575)
// ==============================================================================
// Cấu hình chuẩn theo README_RELAY.md:
// - Module PCF8575: PB12 (SCL) & PB13 (SDA), địa chỉ 0x20
// - 13 Relay: Kênh P0 đến P12 (Relay 1 -> Relay 13)
// - Mức kích hoạt: Active-LOW (Bit 0 = Mức LOW = Bật; Bit 1 = Mức HIGH = Tắt)
// - Các hiệu ứng: Chạy nhảy, đuổi một chiều, tản từ tâm vào ra, so le, dồn tích lũy, chớp nhanh...
// ==============================================================================

// ------------------------------------------------------------------------------
// 1. HIỆU ỨNG: Chạy đuổi Trái sang Phải (Relay 1 -> 13)
// ------------------------------------------------------------------------------
void hieuUng_DuoiTraiSangPhai(int speedMs) {
  tatTatCaRelay();
  for (uint8_t i = 1; i <= TOTAL_RELAYS; i++) {
    nhapNhayLed();
    batRelay(i);
    delay(speedMs);
    tatRelay(i);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 2. HIỆU ỨNG: Chạy đuổi Phải sang Trái (Relay 13 -> 1)
// ------------------------------------------------------------------------------
void hieuUng_DuoiPhaiSangTrai(int speedMs) {
  tatTatCaRelay();
  for (uint8_t i = TOTAL_RELAYS; i >= 1; i--) {
    nhapNhayLed();
    batRelay(i);
    delay(speedMs);
    tatRelay(i);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 3. HIỆU ỨNG: Quét qua lại sao băng (Ping-pong)
// ------------------------------------------------------------------------------
void hieuUng_QuetSaoBang(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    for (uint8_t i = 1; i <= TOTAL_RELAYS; i++) {
      nhapNhayLed();
      batRelay(i);
      delay(speedMs);
      tatRelay(i);
    }
    for (uint8_t i = TOTAL_RELAYS - 1; i >= 2; i--) {
      nhapNhayLed();
      batRelay(i);
      delay(speedMs);
      tatRelay(i);
    }
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 4. HIỆU ỨNG: Tản từ trong tâm ra ngoài (Vào Ra: Tâm là Relay 7 / P6 bung ra hai biên 1 & 13)
// ------------------------------------------------------------------------------
void hieuUng_TuTrongRaNgoai(int speedMs) {
  tatTatCaRelay();
  int center = (TOTAL_RELAYS - 1) / 2; // Tâm là index 6 (Relay 7)
  for (int dist = 0; dist <= center; dist++) {
    nhapNhayLed();
    int leftRelay = (center - dist) + 1;
    int rightRelay = (center + dist) + 1;

    if (leftRelay >= 1) batRelay(leftRelay);
    if (rightRelay <= TOTAL_RELAYS) batRelay(rightRelay);

    delay(speedMs);

    if (leftRelay >= 1) tatRelay(leftRelay);
    if (rightRelay <= TOTAL_RELAYS) tatRelay(rightRelay);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 5. HIỆU ỨNG: Tụ từ hai đầu dồn vào trong tâm (Hai biên 1 & 13 dồn về tâm Relay 7)
// ------------------------------------------------------------------------------
void hieuUng_TuNgoaiVaoTrong(int speedMs) {
  tatTatCaRelay();
  int center = (TOTAL_RELAYS - 1) / 2;
  for (int dist = center; dist >= 0; dist--) {
    nhapNhayLed();
    int leftRelay = (center - dist) + 1;
    int rightRelay = (center + dist) + 1;

    if (leftRelay >= 1) batRelay(leftRelay);
    if (rightRelay <= TOTAL_RELAYS) batRelay(rightRelay);

    delay(speedMs);

    if (leftRelay >= 1) tatRelay(leftRelay);
    if (rightRelay <= TOTAL_RELAYS) tatRelay(rightRelay);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 6. HIỆU ỨNG: Dồn tích lũy từng tầng (Bóng chạy đến cuối rồi xếp chồng cả 13 relay)
// ------------------------------------------------------------------------------
void hieuUng_DonTichLuy(int speedMs) {
  tatTatCaRelay();
  for (int top = TOTAL_RELAYS; top >= 1; top--) {
    for (int i = 1; i <= top; i++) {
      nhapNhayLed();
      batRelay(i);
      delay(speedMs);
      if (i < top) {
        tatRelay(i);
      }
    }
  }
  delay(200);
}

// ------------------------------------------------------------------------------
// 7. HIỆU ỨNG: Xả sạch dần từng relay (Tắt lần lượt từ Relay 1 đến 13)
// ------------------------------------------------------------------------------
void hieuUng_XaDan(int speedMs) {
  for (uint8_t i = 1; i <= TOTAL_RELAYS; i++) {
    nhapNhayLed();
    tatRelay(i);
    delay(speedMs);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 8. HIỆU ỨNG: Chạy sóng đôi nhảy múa (2 bóng liền kề chạy song song)
// ------------------------------------------------------------------------------
void hieuUng_ChaySongDoi(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    for (int i = 1; i < TOTAL_RELAYS; i++) {
      nhapNhayLed();
      batRelay(i);
      batRelay(i + 1);
      delay(speedMs);
      tatRelay(i);
      tatRelay(i + 1);
    }
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 9. HIỆU ỨNG: Nhảy chớp so le chẵn lẻ
// ------------------------------------------------------------------------------
void hieuUng_ChopSoLe(int speedMs, int soLan) {
  for (int l = 0; l < soLan; l++) {
    nhapNhayLed();
    // Bật kênh lẻ: 1, 3, 5, 7, 9, 11, 13 - Tắt kênh chẵn
    for (uint8_t i = 1; i <= TOTAL_RELAYS; i++) {
      if (i % 2 != 0) batRelay(i);
      else tatRelay(i);
    }
    delay(speedMs);

    // Bật kênh chẵn: 2, 4, 6, 8, 10, 12 - Tắt kênh lẻ
    for (uint8_t i = 1; i <= TOTAL_RELAYS; i++) {
      if (i % 2 == 0) batRelay(i);
      else tatRelay(i);
    }
    delay(speedMs);
  }
  tatTatCaRelay();
}

// ------------------------------------------------------------------------------
// 10. HIỆU ỨNG: Chớp nháy nhanh tổng lực (Strobe dồn dập toàn bộ 13 Relay)
// ------------------------------------------------------------------------------
void hieuUng_ChopNhanhTongLuc(int speedMs, int soLan) {
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
// SETUP: KHỞI TẠO HỆ THỐNG
// ==============================================================================
void setup() {
  khoiTaoHeThong();     // Tự động khởi tạo I2C trên PB12/PB13, mở UART và ngắt toàn bộ 13 Relay
  delay(200);           // Chờ ổn định nguồn
  tatTatCaRelay();      // Đảm bảo ban đầu tắt an toàn

  // Chớp nháy toàn bộ 2 nhịp báo hiệu sẵn sàng biểu diễn
  for (int i = 0; i < 2; i++) {
    batTatCaRelay();
    delay(150);
    tatTatCaRelay();
    delay(150);
  }
  delay(300);
}

// ==============================================================================
// LOOP: BIỂU DIỄN TUẦN HOÀN CÁC HIỆU ỨNG CHẠY NHẢY, ĐUỔI, VÀO RA, TẢN NHÁY
// ==============================================================================
void loop() {
  // 1. Chạy đuổi Trái sang Phải (2 lượt)
  for (int i = 0; i < 2; i++) hieuUng_DuoiTraiSangPhai(50);
  delay(80);

  // 2. Chạy đuổi Phải sang Trái (2 lượt)
  for (int i = 0; i < 2; i++) hieuUng_DuoiPhaiSangTrai(50);
  delay(80);

  // 3. Quét sao băng qua lại (2 lượt)
  hieuUng_QuetSaoBang(45, 2);
  delay(80);

  // 4. Tản từ trong tâm Relay 7 bung ra ngoài hai biên (3 lượt)
  for (int i = 0; i < 3; i++) {
    hieuUng_TuTrongRaNgoai(60);
    delay(40);
  }
  delay(80);

  // 5. Tụ từ hai biên dồn vào trong tâm Relay 7 (3 lượt)
  for (int i = 0; i < 3; i++) {
    hieuUng_TuNgoaiVaoTrong(60);
    delay(40);
  }
  delay(80);

  // 6. Chạy sóng đôi nhảy múa (2 lượt)
  hieuUng_ChaySongDoi(60, 2);
  delay(80);

  // 7. Dồn bóng tích lũy (Xếp chồng từng relay từ 13 về 1)
  hieuUng_DonTichLuy(45);
  delay(150);

  // 8. Xả dần từng relay
  hieuUng_XaDan(55);
  delay(100);

  // 9. Chớp nháy so le chẵn lẻ (5 nhịp)
  hieuUng_ChopSoLe(90, 5);
  delay(100);

  // 10. Chớp nhanh tổng lực dồn dập (8 nhịp strobe nhanh)
  hieuUng_ChopNhanhTongLuc(55, 8);
  delay(400); // Nghỉ 0.4 giây rồi lặp lại vòng biểu diễn
}
