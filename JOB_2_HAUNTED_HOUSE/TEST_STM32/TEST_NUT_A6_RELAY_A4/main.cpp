#include <Arduino.h>

// ==============================================================================
// CHƯƠNG TRÌNH TEST: NHẤN NÚT CHÂN A6 (PA6) -> KÍCH RELAY CHÂN A4 (PA4) TRONG
// 4S
// ==============================================================================
// 📌 1. SƠ ĐỒ ĐẤU DÂY:
// - NÚT NHẤN:
//   + Chân 1 của nút nhấn  -> Chân PA6 (A6) trên mạch STM32 Blue Pill
//   + Chân 2 của nút nhấn  -> Chân GND trên mạch STM32 Blue Pill
//   (Chân PA6 được cấu hình INPUT_PULLUP kéo lên 3.3V, khi nhấn sẽ nối xuống
//   GND)
//
// - MODULE RELAY:
//   + Chân VCC của Relay   -> Cắm nguồn 5V trên STM32 (hoặc nguồn 5V ngoài)
//   + Chân GND của Relay   -> Cắm mass GND chung với STM32
//   + Chân IN của Relay    -> Cắm chân PA4 (A4) trên STM32
//
// - ĐÈN LED BÁO TRẠNG THÁI:
//   + Đèn LED PC13 tích hợp sẵn trên bo STM32 sẽ tự động SÁNG khi Relay BẬT,
//     và TẮT khi Relay TẮT giúp dễ dàng quan sát trực quan mà không cần đo đạc.
// ==============================================================================

// Cấu hình chân phần cứng
#define PIN_BUTTON_A6 PA6   // Chân nút nhấn nhận tín hiệu nối GND
#define PIN_RELAY_A4 PA4    // Chân xuất tín hiệu điều khiển Relay
#define PIN_LED_STATUS PC13 // Đèn LED xanh tích hợp sẵn trên bo Blue Pill

// Cấu hình thời gian hoạt động
#define THOI_GIAN_KICH_RELAY_MS                                                \
  4000UL                            // Thời gian giữ Relay BẬT: 4 giây (4000ms)
#define THOI_GIAN_CHONG_DOI_MS 50UL // Thời gian lọc chống dội nút nhấn: 50ms

// Cấu hình mức kích hoạt của Module Relay:
// - Đa số Module Relay 5V dùng loại Active-LOW (Mức LOW = BẬT, Mức HIGH = TẮT)
// - Nếu module của bạn dùng loại Active-HIGH (Mức HIGH = BẬT), chỉ cần đổi 2
// dòng dưới:
#define RELAY_ON LOW   // Mức kích BẬT Relay (Active-LOW: 0V)
#define RELAY_OFF HIGH // Mức TẮT Relay (Active-LOW: 3.3V/5V)

// Mức điều khiển LED PC13 trên bo Blue Pill (PC13 sáng ở mức LOW)
#define LED_ON LOW   // LED PC13 sáng
#define LED_OFF HIGH // LED PC13 tắt

// Biến quản lý trạng thái và thời gian non-blocking
static bool g_relayDangBat = false;
static uint32_t g_thoiGianBatDauKich = 0;

// Biến cho bộ lọc chống dội (Debounce) nút nhấn PA6
static int g_trangThaiNutHienTai = HIGH;
static int g_trangThaiNutTruocDo = HIGH;
static uint32_t g_thoiGianDoiCuoi = 0;
static bool g_nutDaNhan = false;

// Hàm bật Relay và đèn báo
void batRelay() {
  digitalWrite(PIN_RELAY_A4, RELAY_ON);
  digitalWrite(PIN_LED_STATUS, LED_ON);
  g_relayDangBat = true;
  g_thoiGianBatDauKich = millis();
  Serial.println(
      "🟢 [PA6 PRESSED] -> BAT RELAY A4 (PA4) trong 4 giay! (LED PC13: ON)");
}

// Hàm tắt Relay và đèn báo
void tatRelay() {
  digitalWrite(PIN_RELAY_A4, RELAY_OFF);
  digitalWrite(PIN_LED_STATUS, LED_OFF);
  g_relayDangBat = false;
  Serial.println(
      "🔴 [HET 4 GIAY] -> TAT RELAY A4 (PA4) an toan! (LED PC13: OFF)\n");
}

void setup() {
  // 1. Khởi động cổng Serial Monitor để theo dõi trạng thái qua máy tính
  // (115200 baud)
  Serial.begin(115200);

  // 2. Cấu hình chân Relay PA4: OUTPUT và đưa về trạng thái TẮT an toàn ban đầu
  pinMode(PIN_RELAY_A4, OUTPUT);
  digitalWrite(PIN_RELAY_A4, RELAY_OFF);

  // 3. Cấu hình chân LED PC13: OUTPUT và TẮT ban đầu
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LED_OFF);

  // 4. Cấu hình chân nút nhấn PA6: INPUT_PULLUP (kéo lên nội bộ 3.3V)
  pinMode(PIN_BUTTON_A6, INPUT_PULLUP);

  // Chờ ổn định điện áp 300ms
  delay(300);

  Serial.println("\n=======================================================");
  Serial.println("  CHUONG TRINH TEST: NUT NHAN A6 (PA6) -> RELAY A4 (4S)");
  Serial.println("  - Chan nut nhan : PA6 (Noi GND khi bam)");
  Serial.println("  - Chan Relay    : PA4 (Kich Relay trong 4 giay)");
  Serial.println("  - Den bao PC13  : Sang khi Relay dang bat");
  Serial.println("  - Thoi gian kich: 4000ms (4s) tu dong tat");
  Serial.println("  - Chong doi phim: 50ms (Debounce non-blocking)");
  Serial.println("=======================================================\n");
  Serial.println("San sang! Hay bam nut tai chan PA6...\n");
}

void loop() {
  uint32_t now = millis();

  // --------------------------------------------------------------------------
  // 1. BỘ LỌC CHỐNG DỘI NÚT NHẤN (DEBOUNCE FILTER) CHÂN PA6
  // Loại bỏ 100% hiện tượng rung dội cơ khí khi bấm/nhả nút
  // --------------------------------------------------------------------------
  int docNut = digitalRead(PIN_BUTTON_A6);

  // Nếu phát hiện thay đổi mức logic, cập nhật lại mốc thời gian
  if (docNut != g_trangThaiNutTruocDo) {
    g_thoiGianDoiCuoi = now;
    g_trangThaiNutTruocDo = docNut;
  }

  // Khi tín hiệu ổn định liên tục vượt qua thời gian chống dội (50ms):
  if ((now - g_thoiGianDoiCuoi) > THOI_GIAN_CHONG_DOI_MS) {
    // Nếu trạng thái logic thực sự thay đổi:
    if (docNut != g_trangThaiNutHienTai) {
      g_trangThaiNutHienTai = docNut;

      // Phát hiện sườn xuống (Falling Edge: Từ HIGH chuyển sang LOW = BẮT ĐẦU
      // NHẤN NÚT)
      if (g_trangThaiNutHienTai == LOW) {
        batRelay(); // Kích bật Relay và bắt đầu đếm 4s
      }
    }
  }

  // --------------------------------------------------------------------------
  // 2. TỰ ĐỘNG ĐẾM THỜI GIAN VÀ TẮT RELAY SAU 4 GIÂY (NON-BLOCKING)
  // --------------------------------------------------------------------------
  if (g_relayDangBat) {
    if (now - g_thoiGianBatDauKich >= THOI_GIAN_KICH_RELAY_MS) {
      tatRelay(); // Đã đủ 4 giây -> TẮT Relay
    }
  }
}
