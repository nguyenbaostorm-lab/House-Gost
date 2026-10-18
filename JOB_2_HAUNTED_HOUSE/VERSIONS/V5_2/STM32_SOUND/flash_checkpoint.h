#pragma once
#include <Arduino.h>
#include "stm32f1xx_hal.h"

// ==============================================================================
// MODULE LƯU TRỮ VÀ KHÔI PHỤC CHECKPOINT KHI MẤT ĐIỆN (POWER-LOSS RECOVERY)
// Dành cho STM32F103C8 (Sử dụng Page 63 cuối cùng của bộ nhớ Flash nội 64KB)
// ==============================================================================

#define FLASH_CHECKPOINT_ADDR 0x0800FC00U // Địa chỉ bắt đầu của Page 63 (1KB cuối Flash)
#define FLASH_MAGIC_KEY       0x534F554EU // Ký tự "SOUN" (0x534F554E) xác thực dữ liệu hợp lệ

// Cấu trúc dữ liệu mốc lưu trữ trong Flash (16 bytes = 4 Words)
struct DuLieuCheckpointSound {
  uint32_t magic;      // Mã xác thực dữ liệu hợp lệ (FLASH_MAGIC_KEY)
  uint32_t stage;      // Mốc kịch bản (0..7)
  uint32_t p6Opened;   // 1 nếu Cửa từ 1 (P6) đã mở chốt, 0 nếu chưa
  uint32_t extra;      // Dự phòng (luôn bằng 0)
};

// 1. Đọc dữ liệu từ bộ nhớ Flash
inline DuLieuCheckpointSound docCheckpointFlashSound() {
  DuLieuCheckpointSound data;
  volatile uint32_t *pFlash = (volatile uint32_t *)FLASH_CHECKPOINT_ADDR;
  data.magic    = pFlash[0];
  data.stage    = pFlash[1];
  data.p6Opened = pFlash[2];
  data.extra    = pFlash[3];
  return data;
}

// 2. Ghi dữ liệu vào bộ nhớ Flash (CHỈ GHI KHI CÓ THAY ĐỔI ĐỂ BẢO VỆ TUỔI THỌ CHIP)
inline bool ghiCheckpointFlashSound(uint32_t stage, uint32_t p6Opened = 0) {
  DuLieuCheckpointSound cur = docCheckpointFlashSound();

  // Nếu dữ liệu trong Flash đã giống hệt dữ liệu mới -> Không ghi đè để bảo vệ chip
  if (cur.magic == FLASH_MAGIC_KEY && cur.stage == stage && cur.p6Opened == p6Opened) {
    return true;
  }

  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_CHECKPOINT_ADDR;
  EraseInitStruct.NbPages     = 1;
  uint32_t PageError = 0;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK) {
    HAL_FLASH_Lock();
    return false;
  }

  uint32_t dataWords[4] = {
    FLASH_MAGIC_KEY,
    stage,
    p6Opened,
    0
  };

  for (uint32_t i = 0; i < 4; i++) {
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_CHECKPOINT_ADDR + (i * 4), dataWords[i]) != HAL_OK) {
      HAL_FLASH_Lock();
      return false;
    }
  }

  HAL_FLASH_Lock();
  return true;
}

// 3. Xóa dữ liệu checkpoint về 0 (khi bắt đầu lượt mới hoặc reset)
inline void resetCheckpointFlashSound() {
  ghiCheckpointFlashSound(0, 0);
}
