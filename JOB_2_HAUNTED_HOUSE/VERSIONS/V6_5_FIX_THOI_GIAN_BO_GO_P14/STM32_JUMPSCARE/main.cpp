/*
 * STM32 Relay Controller with Timer and Toggle Feature
 * Sơ đồ chân:
 *  - Công tắc 1: PA6  -> Relay 1: PA3  (1p11s = 71 giây)
 *  - Công tắc 2: PA7  -> Relay 2: PA4  (57 giây)
 *  - Công tắc 3: PB12 -> Relay 3: PB13 (1p26s = 86 giây)
 * Giao tiếp UART với bo STM32_SOUND:
 *  - TX: PB5 -> nối sang PB11 bo Sound
 *  - RX: PB6 -> nối từ PB10 bo Sound
 */

#include <Arduino.h>
#include "config.h"
#include "jumpscare_driver.h"

// Biến quản lý đèn LED PC13 (Heartbeat)
static unsigned long lastLedToggle = 0;
static bool ledState = false;

void setup() {
  khoiTaoJumpscare();
}

void loop() {
  unsigned long currentMillis = millis();

  // ============================================================================
  // 1. ĐỌC NÚT BẤM / CÔNG TẮC TIẾP ĐIỂM KHÔ & ĐIỀU KHIỂN TOGGLE / TIMER
  // ============================================================================
  for (int i = 0; i < 3; i++) {
    // 1.1 Đọc nút bấm và lọc rung (Debounce)
    int reading = digitalRead(channels[i].swPin);

    if (reading != channels[i].lastButtonReading) {
      channels[i].lastDebounceTime = currentMillis;
    }

    if ((currentMillis - channels[i].lastDebounceTime) > DEBOUNCE_DELAY) {
      if (reading != channels[i].buttonState) {
        channels[i].buttonState = reading;

        // Phát hiện cạnh xuống (công tắc đóng tiếp điểm kéo chân xuống GND mức LOW)
        if (channels[i].buttonState == LOW) {
          if (!channels[i].relayState) {
            // Đang TẮT -> BẬT relay và bắt đầu đếm giờ
            batKenhRelay(i, currentMillis);
            guiLenhSound('T'); // Gửi lệnh 'T' sang bo Sound: Yêu cầu tắt sạch đèn toàn hệ thống!
          } else {
            // Đang BẬT mà nhấn tiếp -> TẮT NGAY LẬP TỨC
            tatKenhRelay(i);
          }
        }
      }
    }

    channels[i].lastButtonReading = reading;

    // 1.2 Tự động ngắt khi đếm đủ thời gian
    if (channels[i].relayState) {
      if (currentMillis - channels[i].startTime >= channels[i].duration) {
        tatKenhRelay(i);
      }
    }
  }

  // ============================================================================
  // 2. NHẬN LỆNH ĐIỀU KHIỂN TỪ BO STM32_SOUND QUA UART (PB6 RX)
  // ============================================================================
  char cmd = docLenhSound();
  if (cmd != 0) {
    switch (cmd) {
      case '1': // Lệnh '1': Kích hoạt Relay 1
        batKenhRelay(0, currentMillis);
        break;

      case '2': // Lệnh '2': Kích hoạt Relay 2
        batKenhRelay(1, currentMillis);
        break;

      case '3': // Lệnh '3': Kích hoạt Relay 3
        batKenhRelay(2, currentMillis);
        break;

      case 'J': // Lệnh 'J' hoặc 'P': Kích hoạt toàn bộ Relay
      case 'P':
        batKenhRelay(0, currentMillis);
        batKenhRelay(1, currentMillis);
        batKenhRelay(2, currentMillis);
        break;

      case '0': // Lệnh '0': Tắt toàn bộ relay ngay lập tức
        tatTatCaRelay();
        break;

      default:
        break;
    }
  }

  // ============================================================================
  // 3. ĐÈN BÁO TRẠNG THÁI (HEARTBEAT) PC13
  // ============================================================================
  bool coRelayDangBat = channels[0].relayState || channels[1].relayState || channels[2].relayState;
  unsigned long chuKyLed = coRelayDangBat ? 100 : 500; // Có relay bật thì nháy nhanh (100ms), bình thường nháy 500ms

  if (currentMillis - lastLedToggle >= chuKyLed) {
    lastLedToggle = currentMillis;
    ledState = !ledState;
    digitalWrite(PC13, ledState ? LOW : HIGH);
  }
}
