#include <Arduino.h>
#include <SoftwareSerial.h>

// ==============================================================================
// TEST GIAO TIẾP UART: STM32_SOUND <---> STM32_JUMPSCARE
// BẢN MẠCH: STM32_JUMPSCARE (SLAVE PHẢN HỒI PONG)
// ==============================================================================
// SƠ ĐỒ ĐẤU DÂY:
// 1. GND Bo Jumpscare     <---> GND Bo Sound (BẮT BUỘC NỐI CHUNG GND)
// 2. Chân PB6 (RX Jump)   <---> Chân PB10 (TX Sound)
// 3. Chân PB5 (TX Jump)   <---> Chân PB11 (RX Sound)
// 4. LED PC13 (LED onboard bo Jumpscare):
//    - Đảo trạng thái (chớp tắt) mỗi khi nhận được lệnh Ping 'P' từ Sound
// 5. Rơ-le 1 (PA3):
//    - Nhảy 'tạch' nhẹ (bật 60ms rồi tắt) mỗi khi nhận được lệnh 'P' từ Sound
// 6. Công tắc 1 (PA6):
//    - Khi nhấn nối âm GND: Chủ động gửi ký tự 'T' sang bo Sound
// 7. Cổng Serial Monitor  : 115200 baud (Chân PA9/PA10 hoặc qua mạch nạp)
// ==============================================================================

#define PIN_LED_PC13      PC13
#define PIN_RX_FROM_SOUND PB6
#define PIN_TX_TO_SOUND   PB5

#define PIN_RELAY1        PA3
#define PIN_SW1           PA6

// Cổng SoftwareSerial trên chân PB6 (RX) và PB5 (TX)
SoftwareSerial SerialSound(PIN_RX_FROM_SOUND, PIN_TX_TO_SOUND);

static unsigned long countPingRecv = 0;
static bool ledState = false;
static unsigned long relayTurnOffTime = 0;
static bool relayIsOn = false;

static int lastButtonState = HIGH;
static unsigned long lastDebounceTime = 0;

void setup() {
  // 1. Serial Monitor giao tiếp máy tính (115200 baud)
  Serial.begin(115200);

  // 2. Cổng UART giao tiếp với bo Sound (9600 baud)
  SerialSound.begin(9600);

  // 3. LED PC13 (Active-LOW: LOW là BẬT, HIGH là TẮT)
  pinMode(PIN_LED_PC13, OUTPUT);
  digitalWrite(PIN_LED_PC13, HIGH);

  // 4. Rơ-le PA3 (Active-HIGH)
  pinMode(PIN_RELAY1, OUTPUT);
  digitalWrite(PIN_RELAY1, LOW);

  // 5. Nút bấm PA6 (kéo lên PULLUP, kích LOW)
  pinMode(PIN_SW1, INPUT_PULLUP);

  delay(500);
  Serial.println("\n=======================================================");
  Serial.println("  CHUONG TRINH TEST UART: BO STM32_JUMPSCARE (SLAVE)");
  Serial.println("  - RX (Sound -> Jump): PB6");
  Serial.println("  - TX (Jump -> Sound): PB5");
  Serial.println("  - Relay test        : PA3 (nhay tach khi nhan Ping)");
  Serial.println("  - Nut test          : PA6 (nhan de gui 'T' sang Sound)");
  Serial.println("  - Baudrate          : 9600 baud");
  Serial.println("  - Bat buoc          : NOI CHUNG GND 2 BO!");
  Serial.println("=======================================================\n");
}

void loop() {
  unsigned long now = millis();

  // 1. Lắng nghe tín hiệu từ bo STM32_SOUND gửi sang
  if (SerialSound.available()) {
    char c = (char)SerialSound.read();

    if (c == 'P') {
      countPingRecv++;
      // Đảo trạng thái đèn LED PC13
      ledState = !ledState;
      digitalWrite(PIN_LED_PC13, ledState ? LOW : HIGH);

      // Nhảy nhẹ rơ-le PA3 60ms để nghe tiếng tạch xác nhận
      digitalWrite(PIN_RELAY1, HIGH);
      relayIsOn = true;
      relayTurnOffTime = now + 60;

      // Gửi ngay phản hồi 'K' lại cho bo Sound
      SerialSound.write('K');

      Serial.print("[JUMPSCARE] #");
      Serial.print(countPingRecv);
      Serial.println(" Nhan duoc Ping 'P' tu SOUND -> Da gui phan hoi 'K' lai! (OK)");
    } else {
      Serial.print("[JUMPSCARE] Nhan duoc ky tu khac tu SOUND: '");
      Serial.print(c);
      Serial.println("'");
    }
  }

  // 2. Tự động ngắt Rơ-le sau 60ms
  if (relayIsOn && now >= relayTurnOffTime) {
    digitalWrite(PIN_RELAY1, LOW);
    relayIsOn = false;
  }

  // 3. Đọc nút bấm PA6: Nhấn nút để gửi thử ký tự 'T' sang bo Sound
  int reading = digitalRead(PIN_SW1);
  if (reading != lastButtonState) {
    lastDebounceTime = now;
    lastButtonState = reading;
  }

  if ((now - lastDebounceTime) > 50) {
    if (reading == LOW) {
      static bool daGuiNut = false;
      if (!daGuiNut) {
        daGuiNut = true;
        SerialSound.write('T');
        Serial.println(">> [JUMPSCARE] Da nhan nut PA6 -> Gui tin hieu 'T' sang SOUND!");
      }
    } else {
      static bool daGuiNut = false;
      daGuiNut = false;
    }
  }
}
