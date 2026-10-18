#include <Arduino.h>

// ==============================================================================
// DỰ ÁN: CODE TEST RELAY STM32 (DÀN 7 RELAY TỪ CHÂN PA0 ĐẾN PA6 / PA7)
// ==============================================================================
// Sơ đồ cắm dây dàn Relay (Dãy chân PA0 -> PA7 liên tiếp cực đẹp):
// - Relay 1 -> PA0
// - Relay 2 -> PA1
// - Relay 3 -> PA2
// - Relay 4 -> PA3
// - Relay 5 -> PA4
// - Relay 6 -> PA5
// - Relay 7 -> PA6
// - (Relay 8 nếu có -> PA7)
// - Cáp USB cắm máy tính theo dõi Serial Monitor (Baudrate 115200)
// ==============================================================================

// Định nghĩa stub _write để tránh lỗi linker libc_nano trên gcc-arm
extern "C" int _write(int file, char *ptr, int len) {
  (void)file;
  (void)ptr;
  return len;
}

// ------------------------------------------------------------------------------
// CẤU HÌNH MỨC KÍCH HOẠT RELAY & DANH SÁCH CHÂN
// ------------------------------------------------------------------------------
// Module Relay thông dụng là Active-LOW (Mức LOW đóng tiếp điểm, HIGH ngắt)
#define RELAY_ACTIVE_LOW 1 

#if RELAY_ACTIVE_LOW
  #define RELAY_ON   LOW
  #define RELAY_OFF  HIGH
#else
  #define RELAY_ON   HIGH
  #define RELAY_OFF  LOW
#endif

// Đèn LED tích hợp trên board Blue Pill
#define LED_PC13 PC13

// Mảng chân Relay: Mặc định cấu hình 7 chân từ PA0 đến PA6 (nếu dùng 8 relay thêm PA7)
const uint8_t RELAY_PINS[] = { PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7 };

// Chọn số lượng relay cần test: Đặt là 7 (PA0..PA6) hoặc 8 (PA0..PA7)
#define NUM_RELAYS 7 

// Tốc độ chuyển hiệu ứng (ms)
#define STEP_DELAY_MS 100

// ------------------------------------------------------------------------------
// HÀM ĐIỀU KHIỂN CƠ BẢN
// ------------------------------------------------------------------------------
void allRelaysOff() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }
}

void allRelaysOn() {
  for (int i = 0; i < NUM_RELAYS; i++) {
    digitalWrite(RELAY_PINS[i], RELAY_ON);
  }
}

void setRelay(uint8_t index, bool state) {
  if (index < NUM_RELAYS) {
    digitalWrite(RELAY_PINS[index], state ? RELAY_ON : RELAY_OFF);
  }
}

// ==============================================================================
// 1. HIỆU ỨNG: TRÁI SANG PHẢI (Left to Right)
// ==============================================================================
void effect_LeftToRight(int speedMs) {
  Serial.println(">>> [1/4] Chay hieu ung: TRAI SANG PHAI (PA0 -> PA6)");
  allRelaysOff();
  for (int i = 0; i < NUM_RELAYS; i++) {
    setRelay(i, true);
    delay(speedMs);
    setRelay(i, false);
  }
  allRelaysOff();
}

// ==============================================================================
// 2. HIỆU ỨNG: PHẢI SANG TRÁI (Right to Left)
// ==============================================================================
void effect_RightToLeft(int speedMs) {
  Serial.println(">>> [2/4] Chay hieu ung: PHAI SANG TRAI (PA6 -> PA0)");
  allRelaysOff();
  for (int i = NUM_RELAYS - 1; i >= 0; i--) {
    setRelay(i, true);
    delay(speedMs);
    setRelay(i, false);
  }
  allRelaysOff();
}

// ==============================================================================
// 3. HIỆU ỨNG: TỪ TRONG RA NGOÀI (Center to Out)
// ==============================================================================
void effect_CenterOut(int speedMs) {
  Serial.println(">>> [3/4] Chay hieu ung: TU TRONG RA NGOAI");
  allRelaysOff();
  int center = (NUM_RELAYS - 1) / 2; // Với 7 relay, tâm là relay 3 (PA3)

  for (int dist = 0; dist <= center; dist++) {
    int leftIdx = center - dist;
    int rightIdx = center + dist;

    if (leftIdx >= 0) setRelay(leftIdx, true);
    if (rightIdx < NUM_RELAYS) setRelay(rightIdx, true);

    delay(speedMs);

    if (leftIdx >= 0) setRelay(leftIdx, false);
    if (rightIdx < NUM_RELAYS) setRelay(rightIdx, false);
  }
  allRelaysOff();
}

// ==============================================================================
// 4. HIỆU ỨNG: TỪ NGOÀI VÀO TRONG (Outside to Center)
// ==============================================================================
void effect_CenterIn(int speedMs) {
  Serial.println(">>> [4/4] Chay hieu ung: TU NGOAI VAO TRONG");
  allRelaysOff();
  int center = (NUM_RELAYS - 1) / 2;

  for (int dist = center; dist >= 0; dist--) {
    int leftIdx = center - dist;
    int rightIdx = center + dist;

    if (leftIdx >= 0) setRelay(leftIdx, true);
    if (rightIdx < NUM_RELAYS) setRelay(rightIdx, true);

    delay(speedMs);

    if (leftIdx >= 0) setRelay(leftIdx, false);
    if (rightIdx < NUM_RELAYS) setRelay(rightIdx, false);
  }
  allRelaysOff();
}

// ==============================================================================
// KHỞI TẠO HỆ THỐNG
// ==============================================================================
void setup() {
  // Cấu hình LED PC13
  pinMode(LED_PC13, OUTPUT);
  digitalWrite(LED_PC13, LOW); // Sáng LED PC13 báo nguồn tốt

  // Reset đường USB D+ (PA12) để nhận diện Serial USB
  pinMode(PA12, OUTPUT);
  digitalWrite(PA12, LOW);
  delay(100);
  pinMode(PA12, INPUT);

  // Mở cổng USB Serial
  Serial.begin(115200);

  // Cấu hình chân Relay là OUTPUT và tắt toàn bộ ban đầu
  for (int i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], RELAY_OFF);
  }

  // Chớp nháy toàn bộ 2 nhịp xác nhận mạch sẵn sàng
  for (int i = 0; i < 2; i++) {
    allRelaysOn();
    digitalWrite(LED_PC13, LOW);
    delay(150);
    allRelaysOff();
    digitalWrite(LED_PC13, HIGH);
    delay(150);
  }

  delay(500);
  Serial.println("\r\n========================================================");
  Serial.println("       KHOI DONG THANH CONG BO TEST 7 RELAY (PA0-PA6)   ");
  Serial.println("    Chay tuan hoan: Trai-Phai -> Phai-Trai -> Trong-Ra -> Ngoai-Vao");
  Serial.println("========================================================\r\n");
}

// ==============================================================================
// VÒNG LẶP CHÍNH (LOOP)
// ==============================================================================
void loop() {
  // Nháy đèn PC13 đảo trạng thái theo mỗi chu kỳ hiệu ứng
  digitalWrite(LED_PC13, !digitalRead(LED_PC13));

  // 1. HIỆU ỨNG TRÁI SANG PHẢI
  effect_LeftToRight(STEP_DELAY_MS);
  delay(150);

  // 2. HIỆU ỨNG PHẢI SANG TRÁI
  effect_RightToLeft(STEP_DELAY_MS);
  delay(150);

  // 3. HIỆU ỨNG TỪ TRONG RA NGOÀI
  effect_CenterOut(STEP_DELAY_MS + 20);
  delay(150);

  // 4. HIỆU ỨNG TỪ NGOÀI VÀO TRONG
  effect_CenterIn(STEP_DELAY_MS + 20);
  delay(300); // Nghỉ 0.3 giây rồi tiếp tục lặp lại chu kỳ tuần hoàn
}
