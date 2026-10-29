#include <Arduino.h>
#include "config.h"

// ==========================================
// GIAO TIẾP DFPLAYER MINI (Hardware UART3)
// STM32: PB10 (TX) -> trở 1k -> DFPlayer RX
// STM32: PB11 (RX) <- DFPlayer TX
// ==========================================
Uart SerialDFPlayer(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);

void sendCMD(uint8_t cmd, uint16_t param) {
  uint16_t checksum = 0 - (0xFF + 0x06 + cmd + 0x00 + ((param >> 8) & 0xFF) + (param & 0xFF));
  uint8_t packet[10] = {
    0x7E, 0xFF, 0x06, cmd, 0x00,
    (uint8_t)((param >> 8) & 0xFF),
    (uint8_t)(param & 0xFF),
    (uint8_t)((checksum >> 8) & 0xFF),
    (uint8_t)(checksum & 0xFF),
    0xEF
  };
  for (int i = 0; i < 10; i++) {
    SerialDFPlayer.write(packet[i]);
  }
}

// Phát bài hát
void playTrack(uint16_t track) {
  sendCMD(0x03, track); // Lệnh phát trực tiếp
}

// Dừng phát nhạc
void stopTrack() {
  sendCMD(0x16, 0);
}

// Tắt toàn bộ 3 đèn LED báo bài hát
void turnOffAllTrackLeds() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
}

// Bật đèn tương ứng với bài đang phát (trackNum: 1, 2, 3)
void setTrackLed(int trackNum) {
  turnOffAllTrackLeds();
  if (trackNum >= 1 && trackNum <= 3) {
    digitalWrite(LED_PINS[trackNum - 1], HIGH); // Kích mức CAO qua trở 330R
  }
}

// Biến quản lý trạng thái: 0 = Đang rảnh, 1 = Bài 1, 2 = Bài 2, 3 = Bài 3
int currentPlayingTrack = 0;

void setup() {
  // Cấu hình LED PC13
  pinMode(PIN_LED_PC13, OUTPUT);
  digitalWrite(PIN_LED_PC13, HIGH); // Tắt LED PC13

  // Cấu hình 3 đèn LED báo bài (kích HIGH, ban đầu tắt LOW)
  for (int i = 0; i < 3; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
  }

  // Cấu hình chân Relay kéo lên nội
  for (int i = 0; i < 3; i++) {
    pinMode(RELAY_PINS[i], INPUT_PULLUP);
  }

  SerialDFPlayer.begin(DFPLAYER_BAUDRATE);

  // Nháy PC13 3 lần xác nhận khởi động
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED_PC13, LOW);
    delay(150);
    digitalWrite(PIN_LED_PC13, HIGH);
    delay(150);
  }

  delay(1500);          // Chờ thẻ nhớ TF
  sendCMD(0x09, 0x02);  // Chọn thẻ TF
  delay(200);
  sendCMD(0x06, DFPLAYER_DEFAULT_VOLUME); // Âm lượng 25/30
  delay(200);
}

void loop() {
  // 1. Quét khi DFPlayer báo phát xong bài tự nhiên
  while (SerialDFPlayer.available() >= 10) {
    if (SerialDFPlayer.read() == 0x7E) {
      SerialDFPlayer.read(); // ver
      SerialDFPlayer.read(); // len
      uint8_t cmd = SerialDFPlayer.read();
      if (cmd == 0x3D) {     // Mã phát xong bài
        currentPlayingTrack = 0;
        turnOffAllTrackLeds();            // Tắt đèn báo bài
        digitalWrite(PIN_LED_PC13, HIGH); // Tắt đèn PC13
      }
    }
  }

  // 2. Quét 3 tiếp điểm Relay
  for (int i = 0; i < 3; i++) {
    if (digitalRead(RELAY_PINS[i]) == LOW) {
      int trackNum = i + 1;

      if (currentPlayingTrack == 0) {
        // TRƯỜNG HỢP 1: Chưa phát -> Phát bài và SÁNG ĐÈN tương ứng
        playTrack(trackNum);
        currentPlayingTrack = trackNum;
        
        setTrackLed(trackNum);            // Bật đèn PA7 (Bài 1), PA6 (Bài 2), hoặc PB4 (Bài 3)
        digitalWrite(PIN_LED_PC13, LOW);  // Sáng đèn PC13 báo đang phát
      } 
      else if (currentPlayingTrack == trackNum) {
        // TRƯỜNG HỢP 2: Đang phát mà bấm lại đúng nút này -> HỦY PHÁT & TẮT ĐÈN
        stopTrack();
        currentPlayingTrack = 0;
        
        turnOffAllTrackLeds();            // Tắt đèn bài hát
        digitalWrite(PIN_LED_PC13, HIGH); // Tắt đèn PC13
      } 
      else {
        // TRƯỜNG HỢP 3: Đang phát bài khác -> Bị chặn, nháy cảnh báo đèn PC13
        digitalWrite(PIN_LED_PC13, HIGH);
        delay(70);
        digitalWrite(PIN_LED_PC13, LOW);
        delay(70);
        digitalWrite(PIN_LED_PC13, HIGH);
        delay(70);
        digitalWrite(PIN_LED_PC13, LOW);
      }

      // Chờ nhả relay
      while (digitalRead(RELAY_PINS[i]) == LOW) {
        delay(15);
      }
      delay(80); // Chống rung phím
    }
  }
}
