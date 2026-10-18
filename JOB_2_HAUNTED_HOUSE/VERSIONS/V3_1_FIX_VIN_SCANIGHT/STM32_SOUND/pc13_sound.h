#pragma once
#include "config.h"

// ==============================================================================
// MODULE ĐIỀU KHIỂN ĐÈN LED PC13 TRÊN BO STM32_SOUND
// - Chờ nút / bình thường: Nháy chậm (500ms)
// - Sau khi gửi lệnh S: Nháy nhanh lên (80ms)
// - Khi nhận lệnh D: Nháy trong 3s rồi tự động trở về nháy chậm
// ==============================================================================

enum LedSoundState {
  LED_SOUND_CHAM,               // 1. Bình thường / Chờ PB0: Nháy chậm (500ms)
  LED_SOUND_NHANH,              // 2. Sau khi gửi lệnh S: Nháy nhanh lên (80ms)
  LED_SOUND_NHAY_3S_KHI_NHAN_D  // 3. Khi nhận lệnh D: Nháy trong 3s
};

static LedSoundState g_ledSoundState = LED_SOUND_CHAM;
static uint32_t g_thoiGianChopSoundCuoi = 0;
static uint32_t g_thoiGianBatDauNhay3s = 0;

// Khởi tạo chân LED PC13
inline void pc13_sound_init(void) {
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = PIN_LED_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PIN_LED_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(PIN_LED_PORT, PIN_LED_PIN, LED_OFF);
  g_ledSoundState = LED_SOUND_CHAM;
}

// Bật sáng đèn PC13
inline void pc13_sound_on(void) {
  HAL_GPIO_WritePin(PIN_LED_PORT, PIN_LED_PIN, LED_ON);
}

// Tắt đèn PC13
inline void pc13_sound_off(void) {
  HAL_GPIO_WritePin(PIN_LED_PORT, PIN_LED_PIN, LED_OFF);
}

// Đảo trạng thái đèn PC13
inline void pc13_sound_toggle(void) {
  HAL_GPIO_TogglePin(PIN_LED_PORT, PIN_LED_PIN);
}

// Thiết lập trạng thái hoạt động cho đèn PC13
inline void pc13_sound_set_state(LedSoundState state) {
  g_ledSoundState = state;
  if (state == LED_SOUND_NHAY_3S_KHI_NHAN_D) {
    g_thoiGianBatDauNhay3s = HAL_GetTick();
  }
}

// Cập nhật nhấp nháy đèn PC13 theo đúng trạng thái trong loop()
inline void pc13_sound_update(void) {
  uint32_t now = HAL_GetTick();

  switch (g_ledSoundState) {
    case LED_SOUND_CHAM:
      if (now - g_thoiGianChopSoundCuoi >= TOC_DO_NHAY_SOUND_CHAM_MS) {
        g_thoiGianChopSoundCuoi = now;
        pc13_sound_toggle();
      }
      break;

    case LED_SOUND_NHANH:
      if (now - g_thoiGianChopSoundCuoi >= TOC_DO_NHAY_SOUND_NHANH_MS) {
        g_thoiGianChopSoundCuoi = now;
        pc13_sound_toggle();
      }
      break;

    case LED_SOUND_NHAY_3S_KHI_NHAN_D:
      // Trong 3 giây: Nháy liên tục chu kỳ 100ms
      if (now - g_thoiGianBatDauNhay3s < THOI_GIAN_LED_NHAY_KHI_NHAN_D_MS) {
        if (now - g_thoiGianChopSoundCuoi >= 100) {
          g_thoiGianChopSoundCuoi = now;
          pc13_sound_toggle();
        }
      } else {
        // Sau 3 giây: Quay lại nháy chậm
        g_ledSoundState = LED_SOUND_CHAM;
      }
      break;
  }
}
