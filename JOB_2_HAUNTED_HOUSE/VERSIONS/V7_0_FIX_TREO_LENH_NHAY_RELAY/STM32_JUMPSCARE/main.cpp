/*
 * BO 1: STM32 - JUMPSCARE (V6.6 - TÍCH HỢP UART VỚI STM32_SOUND)
 * Sơ đồ chân:
 *  - SW1 (tiếp điểm khô nối GND): PA6  -> Relay 1: PA3  (1p11s = 71 giây)
 *  - SW2 (tiếp điểm khô nối GND): PA7  -> Relay 2: PA4  (57 giây)
 *  - SW3 (tiếp điểm khô nối GND): PB12 -> Relay 3: PB13 (1p26s = 86 giây)
 *  - Giao tiếp UART sang SOUND: PB5 (TX), PB6 (RX)
 *    + PB5 (TX Jump)  -> Nối sang PB11 (RX Sound)
 *    + PB6 (RX Jump)  <- Nối từ PB10 (TX Sound)
 *    + Nối chung mass GND 2 bo
 *  - LED PC13: Nháy nhanh 100ms khi có Jumpscare kích, nháy chậm 500ms khi nghỉ
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"

struct RelayChannel {
  int swPin;
  int relayPin;
  unsigned long duration;
  bool relayState;
  unsigned long startTime;
  int lastButtonReading;
  int buttonState;
  unsigned long lastDebounceTime;
};

static RelayChannel channels[3] = {
  { PIN_SW1, PIN_RELAY1, DURATION_CH1, false, 0, HIGH, HIGH, 0 },
  { PIN_SW2, PIN_RELAY2, DURATION_CH2, false, 0, HIGH, HIGH, 0 },
  { PIN_SW3, PIN_RELAY3, DURATION_CH3, false, 0, HIGH, HIGH, 0 }
};

static int activeChannel = -1; // -1: không có kênh nào kích

// SoftwareSerial giao tiếp với bo Sound: RX=PB6, TX=PB5
SoftwareSerial soundSerial(PB6, PB5); // RX=PB6, TX=PB5
static unsigned long lastHeartbeatTime = 0;
static unsigned long lastLedToggle = 0;
static bool ledState = false;

inline void sendCommandToSound(char cmd) {
  soundSerial.write(cmd);
}

void setup() {
  soundSerial.begin(UART_BAUDRATE);

  for (int i = 0; i < 3; i++) {
    pinMode(channels[i].swPin, INPUT_PULLUP);
    pinMode(channels[i].relayPin, OUTPUT);
    digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
    channels[i].relayState = false;
  }

  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH);

  // Khởi động: Báo SOUND sẵn sàng chạy bình thường
  sendCommandToSound('P');
}

void loop() {
  unsigned long currentMillis = millis();

  // ============================================================================
  // 1. QUÉT 3 CÔNG TẮC NÚT BẤM (DEBOUNCE 50MS, COOLDOWN 400MS, TOGGLE & KHÓA LIÊN ĐỘNG)
  // ============================================================================
  static unsigned long thoiGianCooldownNut = 0;

  for (int i = 0; i < 3; i++) {
    int reading = digitalRead(channels[i].swPin);

    if (reading != channels[i].lastButtonReading) {
      channels[i].lastDebounceTime = currentMillis;
    }

    if ((currentMillis - channels[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != channels[i].buttonState) {
        channels[i].buttonState = reading;

        if (channels[i].buttonState == LOW && (currentMillis >= thoiGianCooldownNut)) { // Nhấn nút (kéo xuống GND)
          thoiGianCooldownNut = currentMillis + 400; // Khóa 400ms chống rung dội tiếp điểm khô

          if (activeChannel == -1) {
            // Chưa có relay nào kích -> KÍCH HOẠT kênh này
            activeChannel = i;
            channels[i].relayState = true;
            channels[i].startTime = currentMillis;
            digitalWrite(channels[i].relayPin, RELAY_ACTIVE);

            // Báo SOUND ngắt âm thanh nền ngay lập tức
            sendCommandToSound('S');
            // Gửi lệnh T1, T2, T3 tương ứng từng kênh (K1: T1 - 71s, K2: T2 - 57s, K3: T3 - 86s)
            soundSerial.print("T");
            soundSerial.print(i + 1);
          } 
          else if (activeChannel == i) {
            // Đang chạy chính kênh này mà nhấn lại -> TẮT NGAY (Toggle)
            channels[i].relayState = false;
            digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
            activeChannel = -1;

            // Báo SOUND hoạt động trở lại
            sendCommandToSound('P');
          }
          // Nếu activeChannel != -1 và != i: Bị khóa liên động (không cho kích đè)
        }
      }
    }

    channels[i].lastButtonReading = reading;
  }

  // ============================================================================
  // 2. TỰ ĐỘNG TẮT KHI HẾT THỜI GIAN
  // ============================================================================
  if (activeChannel != -1) {
    int i = activeChannel;
    if (currentMillis - channels[i].startTime >= channels[i].duration) {
      channels[i].relayState = false;
      digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
      activeChannel = -1;

      // Hết giờ -> Báo SOUND kích lại
      sendCommandToSound('P');
    }
  }

  // ============================================================================
  // 3. HEARTBEAT ĐỒNG BỘ ĐỊNH KỲ (2 GIÂY/LẦN ĐỂ THÔNG THOÁNG ĐƯỜNG TRUYỀN UART)
  // ============================================================================
  if (currentMillis - lastHeartbeatTime >= 2000) {
    lastHeartbeatTime = currentMillis;
    sendCommandToSound(activeChannel != -1 ? 'S' : 'P');
  }

  // ============================================================================
  // 4. LẮNG NGHE LỆNH TỪ BO SOUND GỬI SANG
  // ============================================================================
  while (soundSerial.available() > 0) {
    char cmd = (char)soundSerial.read();
    if (cmd == '0') {
      // Lệnh tắt toàn bộ khẩn cấp
      for (int i = 0; i < 3; i++) {
        channels[i].relayState = false;
        digitalWrite(channels[i].relayPin, RELAY_INACTIVE);
      }
      activeChannel = -1;
      sendCommandToSound('P');
    } 
    else if (cmd >= '1' && cmd <= '3') {
      int idx = cmd - '1';
      if (activeChannel == -1) {
        activeChannel = idx;
        channels[idx].relayState = true;
        channels[idx].startTime = currentMillis;
        digitalWrite(channels[idx].relayPin, RELAY_ACTIVE);
        sendCommandToSound('S');
        soundSerial.print("T");
        soundSerial.print(idx + 1);
      }
    }
  }

  // ============================================================================
  // 5. ĐÈN BÁO TRẠNG THÁI PC13 (HEARTBEAT)
  // ============================================================================
  unsigned long chuKyLed = (activeChannel != -1) ? 100 : 500;
  if (currentMillis - lastLedToggle >= chuKyLed) {
    lastLedToggle = currentMillis;
    ledState = !ledState;
    digitalWrite(PC13, ledState ? LOW : HIGH);
  }
}
