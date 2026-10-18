#include <Arduino.h>

// ==============================================================================
// TEST CẢM BIẾN TIỆM CẬN QUANG E3F-DS30C4 (NPN - NO) KÍCH ĐÈN TÍN HIỆU
// ==============================================================================
// 1. SƠ ĐỒ ĐẤU DÂY:
// - Dây NÂU cảm biến        -> Cắm chân 5V của STM32 Blue Pill
// - Dây XANH DƯƠNG cảm biến -> Cắm chân GND của STM32 Blue Pill
// - Dây ĐEN cảm biến (OUT)  -> Cắm chân PB0 của STM32 Blue Pill (Active-LOW)
// - ĐÈN TÍN HIỆU            -> Chân PC13 (sáng khi có vật cản)
// - Chân phụ dự phòng       -> Chân PB12 (xuất HIGH khi có vật cản)
// ==============================================================================

#define PIN_CAM_BIEN  PB0   // Dây ĐEN cảm biến (kéo xuống LOW khi có vật cản)
#define PIN_DEN       PC13  // Đèn tín hiệu ở chân PC13 (LED onboard trên mạch Blue Pill)
#define PIN_DEN_PHU   PB12  // Chân phụ PB12 xuất HIGH (phòng khi cắm thêm relay/đèn ngoài)

// Chú ý với chân PC13 của STM32 Blue Pill:
// LED onboard PC13 sáng ở mức LOW (0V) và tắt ở mức HIGH (3.3V)
#define DEN_ON   LOW
#define DEN_OFF  HIGH

void setup() {
  // 1. Cổng giao tiếp máy tính (Serial Monitor 115200 baud)
  Serial.begin(115200);

  // 2. Cấu hình chân ĐÈN TÍN HIỆU (PC13) làm OUTPUT
  pinMode(PIN_DEN, OUTPUT);
  digitalWrite(PIN_DEN, DEN_OFF); // Mặc định tắt đèn (HIGH với PC13)

  // 3. Cấu hình chân đèn phụ (PB12)
  pinMode(PIN_DEN_PHU, OUTPUT);
  digitalWrite(PIN_DEN_PHU, LOW);

  // 4. Cấu hình chân cảm biến NPN: BẮT BUỘC KÉO LÊN NỘI BỘ (INPUT_PULLUP)
  pinMode(PIN_CAM_BIEN, INPUT_PULLUP);

  // Chờ ổn định 500ms
  delay(500);

  Serial.println("\n=======================================================");
  Serial.println("  CHUONG TRINH TEST CAM BIEN TIEM CAN E3F-DS30C4 (NPN)");
  Serial.println("  - Chan cam bien: PB0 (Day den NPN)");
  Serial.println("  - Chan den bao : PC13 (LED onboard)");
  Serial.println("  - Chan den phu : PB12 (Xuat 3.3V khi co vat)");
  Serial.println("=======================================================\n");
}

void loop() {
  // Đọc tín hiệu cảm biến:
  // - CÓ vật cản: Cảm biến NPN dẫn thông xuống GND -> Chân PB0 nhận mức LOW (0V)
  // - KHÔNG có vật cản: Trở kéo lên giữ chân PB0 ở mức HIGH (3.3V)
  int trangThaiCamBien = digitalRead(PIN_CAM_BIEN);

  if (trangThaiCamBien == LOW) {
    // 👉 CÓ VẬT CẢN TRƯỚC CẢM BIẾN:
    digitalWrite(PIN_DEN, DEN_ON);          // BẬT ĐÈN PC13 (kéo xuống LOW để sáng LED PC13)
    digitalWrite(PIN_DEN_PHU, HIGH);        // BẬT chân phụ PB12
    Serial.println("🔴 [PHAT HIEN VAT CAN] -> BAT DEN PC13 (ON)");
  } else {
    // 👉 KHÔNG CÓ VẬT CẢN:
    digitalWrite(PIN_DEN, DEN_OFF);         // TẮT ĐÈN PC13
    digitalWrite(PIN_DEN_PHU, LOW);         // TẮT chân phụ PB12
    Serial.println("⚪ [KHONG CO VAT CAN]  -> TAT DEN PC13 (OFF)");
  }

  delay(100); // Lấy mẫu mỗi 100ms
}
