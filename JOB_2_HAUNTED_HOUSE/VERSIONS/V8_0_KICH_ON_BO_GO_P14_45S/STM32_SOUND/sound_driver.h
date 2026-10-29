#pragma once
#include "config.h"

// ==============================================================================
// THƯ VIỆN ĐIỀU KHIỂN 7 RELAY, 6 NÚT BẤM & UART PHẦN MỀM CHUẨN STM32 HAL
// ==============================================================================

// Mảng chân của 7 Relay trên GPIOA
static const uint16_t RELAY_PINS[TOTAL_RELAYS] = {
  PIN_RELAY_1_PIN, PIN_RELAY_2_PIN, PIN_RELAY_3_PIN, PIN_RELAY_4_PIN,
  PIN_RELAY_5_PIN, PIN_RELAY_6_PIN, PIN_RELAY_7_PIN
};

// Mảng chân của 6 Nút bấm trên GPIOB
static const uint16_t BTN_PINS[6] = {
  PIN_BTN_1_PIN, PIN_BTN_2_PIN, PIN_BTN_3_PIN,
  PIN_BTN_4_PIN, PIN_BTN_5_PIN, PIN_BTN_6_PIN
};

// ------------------------------------------------------------------------------
// HÀM TẠO TRỄ MICROSECOND CHÍNH XÁC CHUẨN ARDUINO & HAL
// ------------------------------------------------------------------------------
#include <Arduino.h>

static inline void init_dwt_counter(void) {
  // Không cần DWT, dùng delayMicroseconds chuẩn của framework
}

static inline void delay_us(uint32_t us) {
  delayMicroseconds(us);
}

// ==============================================================================
// CÁC HÀM TIỆN ÍCH DÀNH CHO NGƯỜI DÙNG (CHUẨN STM32 HAL)
// ==============================================================================

// 1. Khởi tạo toàn bộ hệ thống
inline void khoiTaoHeThong(void) {
  // Bật xung nhịp Clock cho GPIOA, GPIOB, GPIOC
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  init_dwt_counter();

  // Cấu hình LED trạng thái bo PC13
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = PIN_LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PIN_LED_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PIN_LED_PORT, PIN_LED_PIN, LED_OFF);

  // Cấu hình 7 chân Relay trên GPIOA (PA0 -> PA6)
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[i], RELAY_OFF);
    GPIO_InitStruct.Pin = RELAY_PINS[i];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[i], RELAY_OFF);
  }

  // Cấu hình 6 chân nút nhấn trên GPIOB chế độ kéo lên nội bộ Pull-Up
  for (uint8_t i = 0; i < 6; i++) {
    GPIO_InitStruct.Pin = BTN_PINS[i];
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(PIN_BTN_PORT, &GPIO_InitStruct);
  }

  // Đảm bảo cấu hình Input Pull-Up chuẩn cả HAL lẫn Arduino (Đặc biệt chân PB7)
  pinMode(PB0, INPUT_PULLUP);
  pinMode(PB5, INPUT_PULLUP);
  pinMode(PB6, INPUT_PULLUP);
  pinMode(PB7, INPUT_PULLUP);
  pinMode(PB8, INPUT_PULLUP);
  pinMode(PB9, INPUT_PULLUP);

  // Cấu hình chân UART: PB12 (TX) Output Push-Pull, PB13 (RX) Input Pull-Up
  GPIO_InitStruct.Pin = PIN_UART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(PIN_UART_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET); // Idle = HIGH

  GPIO_InitStruct.Pin = PIN_UART_RX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PIN_UART_PORT, &GPIO_InitStruct);
}

// 2. BẬT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 7)
inline void batRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[relayNum - 1], RELAY_ON);
  }
}

// 3. TẮT 1 rơ-le cụ thể (Số thứ tự từ 1 đến 7)
inline void tatRelay(uint8_t relayNum) {
  if (relayNum >= 1 && relayNum <= TOTAL_RELAYS) {
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[relayNum - 1], RELAY_OFF);
  }
}

// 4. BẬT toàn bộ 7 rơ-le cùng lúc
inline void batTatCaRelay(void) {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[i], RELAY_ON);
  }
}

// 5. TẮT toàn bộ 7 rơ-le cùng lúc
inline void tatTatCaRelay(void) {
  for (uint8_t i = 0; i < TOTAL_RELAYS; i++) {
    HAL_GPIO_WritePin(GPIOA, RELAY_PINS[i], RELAY_OFF);
  }
}

// 6. Gửi lệnh ký tự sang bo STM32 Relay (qua PB12 TX với chuẩn 9600 baud = 104us/bit)
inline void guiLenhRelay(char cmd) {
  // Start bit: Kéo xuống LOW
  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_RESET);
  delay_us(104);

  // 8 Data bits (LSB trước, MSB sau)
  uint8_t data = (uint8_t)cmd;
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x01) {
      HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET);
    } else {
      HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_RESET);
    }
    delay_us(104);
    data >>= 1;
  }

  // Stop bit: Kéo lên HIGH
  HAL_GPIO_WritePin(PIN_UART_PORT, PIN_UART_TX_PIN, GPIO_PIN_SET);
  delay_us(104);
}

// 7. Đọc lệnh từ bo STM32 Relay gửi sang (qua PB13 RX, trả về 0 nếu chưa có lệnh)
inline char docLenhRelay(void) {
  // Kiểm tra chân RX: Nếu ở mức LOW nghĩa là có Start bit truyền tới
  if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) == GPIO_PIN_RESET) {
    // Chờ nửa chu kỳ bit (52us) để lấy mẫu ở giữa Start bit nhằm chống nhiễu
    delay_us(52);
    if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) != GPIO_PIN_RESET) {
      return 0; // Tín hiệu nhiễu chớp nhoáng, bỏ qua
    }

    uint8_t receivedChar = 0;
    // Lần lượt đọc 8 Data bits
    for (uint8_t i = 0; i < 8; i++) {
      delay_us(104); // Chờ đến giữa bit kế tiếp
      if (HAL_GPIO_ReadPin(PIN_UART_PORT, PIN_UART_RX_PIN) == GPIO_PIN_SET) {
        receivedChar |= (1 << i);
      }
    }

    // Chờ Stop bit
    delay_us(104);
    return (char)receivedChar;
  }
  return 0;
}

// 8. Kiểm tra xem nút nào đang được bấm (Trả về 1..6, hoặc 0 nếu không bấm)
inline uint8_t kiemTraNutNhan(void) {
  for (uint8_t i = 0; i < 6; i++) {
    if (HAL_GPIO_ReadPin(PIN_BTN_PORT, BTN_PINS[i]) == GPIO_PIN_RESET) {
      return i + 1;
    }
  }
  return 0;
}

// ==============================================================================
// HÀM KIỂM TRA CẢM BIẾN / NÚT NHẤN NỐI GND Ở CHÂN PB7 (BTN4 / NHẠC CHIẾN THẮNG):
// - Cấu hình chân PB7 kéo lên Pull-Up nội bộ (bình thường mức HIGH).
// - Khi nhận tín hiệu kích âm GND (Mức LOW): lọc chống dội kỹ qua 5 lần đọc liên tiếp trong 50ms
//   (mỗi lần cách nhau 10ms) để loại bỏ hoàn toàn mọi xung nhiễu điện từ / xung khởi động nguồn,
//   hoàn toàn chuẩn xác theo cơ chế của chân PA6 bên mạch STM32_RELAY.
// ==============================================================================
inline bool kiemTraNutNhanPB7(void) {
  bool mucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_4_PIN) == GPIO_PIN_RESET) || 
                 (digitalRead(PB7) == LOW);
  if (mucThap) {
    // Kiểm tra liên tục 5 mẫu cách nhau 10ms (tổng 50ms)
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      bool vanMucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_4_PIN) == GPIO_PIN_RESET) || 
                        (digitalRead(PB7) == LOW);
      if (!vanMucThap) {
        return false; // Nếu bị ngắt quãng do xung gai nhiễu -> hủy, không kích hoạt
      }
    }
    return true; // Thực sự có tín hiệu kích âm GND mức LOW ổn định
  }
  return false;
}

// ==============================================================================
// HÀM KIỂM TRA NÚT BẤM KHÔI PHỤC KHI CÚP ĐIỆN Ở CHÂN PB8 (BTN5 / NÚT KHÔI PHỤC):
// - Cấu hình chân PB8 kéo lên Pull-Up nội bộ (bình thường mức HIGH).
// - Khi nhấn nút nối mass GND: mức LOW ổn định trong 50ms (lọc 5 lần x 10ms).
// ==============================================================================
inline bool kiemTraNutNhanPB8(void) {
  bool mucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_5_PIN) == GPIO_PIN_RESET) || 
                 (digitalRead(PB8) == LOW);
  if (mucThap) {
    for (uint8_t i = 0; i < 5; i++) {
      delay(10);
      bool vanMucThap = (HAL_GPIO_ReadPin(PIN_BTN_PORT, PIN_BTN_5_PIN) == GPIO_PIN_RESET) || 
                        (digitalRead(PB8) == LOW);
      if (!vanMucThap) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// 9. Đảo trạng thái đèn LED báo PC13
inline void daoTrangThaiLed(void) {
  HAL_GPIO_TogglePin(PIN_LED_PORT, PIN_LED_PIN);
}

// 10. Lấy mốc thời gian hệ thống tính theo mili-giây
inline uint32_t layThoiGianMs(void) {
  return HAL_GetTick();
}

// 11. Chờ mili-giây
inline void choMiliGiay(uint32_t ms) {
  HAL_Delay(ms);
}

// 12. Nhấp nháy đèn LED trạng thái bo PC13 liên tục (mỗi 100ms)
inline void nhapNhayLed(void) {
  static uint32_t lastTime = 0;
  if (layThoiGianMs() - lastTime >= 100) {
    lastTime = layThoiGianMs();
    daoTrangThaiLed();
  }
}



