#include <Arduino.h>
#include <SoftwareSerial.h>

// ==============================================================================
// TEST GIAO TIẾP UART: STM32_SOUND <---> STM32_JUMPSCARE
// BẢN MẠCH: STM32_SOUND (MASTER PING)
// ==============================================================================
// SƠ ĐỒ ĐẤU DÂY:
// 1. GND Bo Sound         <---> GND Bo Jumpscare (BẮT BUỘC NỐI CHUNG GND)
// 2. Chân PB10 (TX Sound) <---> Chân PB6 (RX Jumpscare)
// 3. Chân PB11 (RX Sound) <---> Chân PB5 (TX Jumpscare)
// 4. LED PC13 (LED onboard bo Sound):
//    - Nháy đơn ngắn  : Vừa phát lệnh Ping 'P' sang Jumpscare
//    - Sáng rõ 200ms  : Đã nhận được phản hồi 'K' từ Jumpscare (THÀNH CÔNG 2 CHIỀU!)
// 5. Cổng Serial Monitor  : 115200 baud (Chân PA9/PA10 hoặc qua mạch nạp)
// ==============================================================================

#define PIN_LED_PC13     PC13
#define PIN_TX_TO_JUMP   PB10
#define PIN_RX_FROM_JUMP PB11

// Khởi tạo cổng UART giao tiếp với bo Jumpscare (RX=PB11, TX=PB10)
SoftwareSerial SerialJump(PIN_RX_FROM_JUMP, PIN_TX_TO_JUMP);

static unsigned long lastPingTime = 0;
static unsigned long countSent = 0;
static unsigned long countRecv = 0;

void setup() {
  // 1. Serial Monitor giao tiếp máy tính (115200 baud)
  Serial.begin(115200);

  // 2. Cổng UART giao tiếp với bo Jumpscare (9600 baud)
  SerialJump.begin(9600);

  // 3. LED PC13 (Active-LOW: LOW là BẬT, HIGH là TẮT)
  pinMode(PIN_LED_PC13, OUTPUT);
  digitalWrite(PIN_LED_PC13, HIGH); // Tắt LED ban đầu

  delay(500);
  Serial.println("\n=======================================================");
  Serial.println("   CHUONG TRINH TEST UART: BO STM32_SOUND (MASTER)");
  Serial.println("   - TX (Sound -> Jump): PB10");
  Serial.println("   - RX (Jump -> Sound): PB11");
  Serial.println("   - Baudrate          : 9600 baud");
  Serial.println("   - Bat buoc          : NOI CHUNG GND 2 BO!");
  Serial.println("=======================================================\n");
}

void loop() {
  unsigned long now = millis();

  // Định kỳ mỗi 1.0 giây gửi 1 gói Ping 'P' sang Jumpscare
  if (now - lastPingTime >= 1000) {
    lastPingTime = now;
    countSent++;

    // Nháy nhanh LED PC13 báo vừa phát lệnh
    digitalWrite(PIN_LED_PC13, LOW);
    SerialJump.write('P');
    
    Serial.print("[SOUND -> JUMP] #");
    Serial.print(countSent);
    Serial.print(" Gui lenh Ping 'P'... ");

    // Chờ phản hồi tối đa 250ms
    unsigned long waitStart = millis();
    bool nhanDuoc = false;
    while (millis() - waitStart < 250) {
      if (SerialJump.available()) {
        char c = (char)SerialJump.read();
        if (c == 'K') {
          nhanDuoc = true;
          countRecv++;
          break;
        }
      }
    }

    if (nhanDuoc) {
      // Nhận phản hồi OK: Giữ sáng LED PC13 thêm 200ms
      digitalWrite(PIN_LED_PC13, LOW);
      Serial.print("-> [OK] Nhan phan hoi 'K' tu Jumpscare! (Tong OK: ");
      Serial.print(countRecv);
      Serial.println(")");
      delay(200);
      digitalWrite(PIN_LED_PC13, HIGH);
    } else {
      // Không nhận được: Tắt LED ngay
      digitalWrite(PIN_LED_PC13, HIGH);
      Serial.println("-> [THAT BAI] Khong thay Jumpscare tra loi! (Kiem tra day PB10, PB11, GND)");
    }
  }

  // Đọc thêm nếu Jumpscare chủ động gửi ký tự khác (ví dụ phím bấm 'T')
  if (SerialJump.available()) {
    char extra = (char)SerialJump.read();
    Serial.print(">> [SOUND] Nhan duoc tin hieu bat ngo tu Jumpscare: '");
    Serial.print(extra);
    Serial.println("'");
  }
}
