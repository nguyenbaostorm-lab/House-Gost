#include <Arduino.h>
#include "config.h"
#include "pc13_relay.h"
#include "pcf8575_relay.h"

// ==============================================================================
// ĐỊNH NGHĨA CÁC TRẠNG THÁI HỆ THỐNG
// ==============================================================================
enum TrangThaiHeThong {
  TRANG_THAI_KHOI_DONG = 0,
  TRANG_THAI_CHO_LENH_S = 1,
  TRANG_THAI_KICH_P0_P3 = 2,
  TRANG_THAI_CHUYEN_P1 = 3,
  TRANG_THAI_CHO_50S_P2 = 4,
  TRANG_THAI_CHO_LENH_K = 5,
  TRANG_THAI_DA_NHAN_K = 6
};

static TrangThaiHeThong trangThai = TRANG_THAI_CHO_LENH_S;

// Biến lưu mốc thời gian
static uint32_t thoiGianNhanS = 0;
static uint32_t thoiGianNhanH = 0;
static uint32_t thoiGianNhanK = 0;
static uint32_t thoiGianNhanPA6 = 0;
static uint32_t thoiGianBatP7 = 0;
static uint32_t thoiGianBatP10_P3 = 0;
static uint32_t lanRefreshCuoi = 0;

// Các cờ kịch bản
static bool daNhanLenhH = false;
static bool daBatR7_SauH = false;
static bool daBatP10_P3_SauH = false;

static bool daNhanLenhK = false;
static bool daTatP13_Sau10sK = false;
static bool daXuLy30sSauK = false;

static bool daXuLyTinHieuA6 = false;
static bool choBatP7_SauA6 = false;
static bool daBatP7_SauA6 = false;
static bool choBatP10_P3_SauA6 = false;
static bool daBatP10_P3_SauA6 = false;
static bool pa6DaSanSang = false;

// Kịch bản Bộ gõ P14
struct MauBoGo {
  uint32_t thoiGianBatMs;
  uint32_t thoiGianTatMs;
};

static const MauBoGo CAC_MAU_BO_GO[] = {
    {1000UL, 1500UL}, // Mẫu 1: 1s BẬT, 1.5s TẮT
    {2000UL, 2000UL}, // Mẫu 2: 2s BẬT, 2s TẮT
    {3000UL, 3000UL}  // Mẫu 3: 3s BẬT, 3s TẮT
};
static const uint8_t SO_MAU_BO_GO = sizeof(CAC_MAU_BO_GO) / sizeof(CAC_MAU_BO_GO[0]);

static bool boGoDangChay = false;
static uint32_t thoiGianBatDauBoGo = 0;
static uint32_t thoiGianChuyenTrangThaiBoGo = 0;
static uint32_t thoiGianGiuTrangThaiHienTai = 0;
static bool trangThaiBoGoHienTai = false;
static uint8_t chiSoMauHienTai = 0;
static bool daKetThuc30sBoGo = false;
static bool daTatDenSauBoGo = false;
static bool daGuiLenhJ = false;
static bool daXuLyTinHieuA5 = false;

// Kịch bản sau khi gửi 'J' (bật P2 trong 53s hoặc nhận 'U')
static uint32_t thoiGianGuiJ = 0;
static bool daGuiJ_BatP2 = false;
static bool daTatP2_BatP0_P1 = false;

// Chế độ chiến thắng 'V'
static bool cheDoV_DangBat = false;

// ==============================================================================
// 1. SETUP: KHỞI TẠO BAN ĐẦU
// ==============================================================================
void setup() {
  g_p6DaBat = false;
  daXuLyTinHieuA6 = false;
  pa6DaSanSang = false;
  daXuLyTinHieuA5 = false;
  daGuiJ_BatP2 = false;
  daTatP2_BatP0_P1 = false;
  cheDoV_DangBat = false;
  daTatP13_Sau10sK = false;
  daXuLy30sSauK = false;

  khoiTaoHeThong();
  resetBoLocA5();
  pc13_relay_init();
  randomSeed(analogRead(PA0) + micros());

  // Ổn định ban đầu trong 3 giây: Đèn PC13 chớp nhanh, duy trì P0 và P1 bật sẵn
  pc13_relay_set_state(LED_CHOP_NHANH);
  uint32_t startInit = millis();
  while (millis() - startInit < THOI_GIAN_KHOI_DONG_ON_DINH_MS) {
    pc13_relay_update();
    batP0_P1_TatTatCaConLai();
    delay(10);
  }

  // Khi đã ổn định: Chuyển sang chờ lệnh 'S', đèn PC13 nhấp nháy chậm
  batP0_P1_TatTatCaConLai();
  pc13_relay_set_state(LED_CHOP_CHAM);
  trangThai = TRANG_THAI_CHO_LENH_S;
}

// ==============================================================================
// 2. LOOP: VÒNG LẶP CHÍNH CỦA HỆ THỐNG (CHUẨN V1 TỐI ƯU CỰC KỲ ỔN ĐỊNH)
// ==============================================================================
void loop() {
  pc13_relay_update();

  // ==========================================================================
  // 👉 TRIỆT TIÊU 100% NGUYÊN NHÂN LÀM RƠ-LE BỊ GIẬT GIẬT:
  // 1. Loại bỏ quét RF tự động đọc I2C mỗi 20ms (tránh nhiễu kênh R3 chớp relay 13, 7, 10).
  // 2. Loại bỏ lệnh pcf8575_update() ghi bus định kỳ 500ms (PCF8575 giữ chốt cứng vĩnh viễn).
  // ==========================================================================

  // ==========================================================================
  // 👉 ĐỌC LỆNH UART TỪ STM32_SOUND
  // ==========================================================================
  char lenhTuSound = docLenhSound();

  // Lệnh '0': Reset cứng toàn bộ hệ thống
  if (lenhTuSound == '0') {
    tatTatCaRelay();
    writePCF8575(0x0000);
    tatChanA7();
    delay(50);
    NVIC_SystemReset();
  }

  uint32_t now = millis();

  // ==========================================================================
  // 👉 LUỒNG XỬ LÝ CHÂN PA6 (CÁI XE):
  // Khi đặt xe vào: TẮT P5, TẮT P3, BẬT CỬA TỪ P6, HẸN GIỜ 5S BẬT P7, 45S BẬT P10/P3
  // ==========================================================================
  if (!pa6DaSanSang && (millis() >= THOI_GIAN_KHOI_DONG_ON_DINH_MS)) {
    if (digitalRead(PA6) == HIGH) {
      pa6DaSanSang = true;
    }
  }

  if (pa6DaSanSang && !daXuLyTinHieuA6 && kiemTraNutNhanPA6()) {
    daXuLyTinHieuA6 = true;
    g_p6DaBat = true;
    g_pcfData |= (1 << 6);                     // BẬT Cửa từ 1 (P6)
    g_pcfData &= ~(1 << 3);                    // TẮT Đèn quan sát (P3)
    g_pcfData &= ~(1 << RELAY_LED_DO_CHOI_P5); // TẮT Đèn đồ chơi (P5)
    writePCF8575(g_pcfData);
    datKhoaNhietRelay(300);
    thoiGianNhanPA6 = now;
    choBatP7_SauA6 = true;
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
    guiLenhSound('G');
  }

  // Hẹn giờ sau PA6: 5s bật P7
  if (choBatP7_SauA6 && !daBatP7_SauA6 && (now - thoiGianNhanPA6 >= THOI_GIAN_H_BAT_R7_MS)) {
    batChanPCF(RELAY_DEN_BUC_TRANH_P7);
    daBatP7_SauA6 = true;
    choBatP10_P3_SauA6 = true;
    thoiGianBatP7 = now;
  }

  // Hẹn giờ sau P7: 45s bật P10 và P3
  if (choBatP10_P3_SauA6 && !daBatP10_P3_SauA6 && (now - thoiGianBatP7 >= THOI_GIAN_SAU_R7_BAT_P10_P3_MS)) {
    batChanPCF(RELAY_DEN_RONG_PHUONG_P10);
    batChanPCF(3);
    daBatP10_P3_SauA6 = true;
    thoiGianBatP10_P3 = now;
  }

  // ==========================================================================
  // 👉 KHI NHẬN ĐƯỢC LỆNH 'K' TỪ STM32_SOUND:
  // ==========================================================================
  if (!daNhanLenhK && (lenhTuSound == 'K' || lenhTuSound == 'k') && (trangThai == TRANG_THAI_CHO_LENH_K || daTatP2_BatP0_P1)) {
    chuyenPha_NhanLenhK_BatP11_P13_P0_P1();
    thoiGianNhanK = now;
    daNhanLenhK = true;
    daTatP13_Sau10sK = false;
    daXuLy30sSauK = false;
    trangThai = TRANG_THAI_DA_NHAN_K;
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
    while (SoundSerial.available() > 0) {
      SoundSerial.read();
    }
  }

  // Lệnh 'L': Bật thêm đèn quan sát P3 (PA7 đã chuyển sang cho Bộ gõ)
  if (lenhTuSound == 'L' || lenhTuSound == 'l') {
    batChanPCF(3);
    pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
  }

  // Lệnh 'V': Chế độ chiến thắng
  if (lenhTuSound == 'V' || lenhTuSound == 'v') {
    cheDoV_DangBat = !cheDoV_DangBat;
    if (cheDoV_DangBat) {
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daKetThuc30sBoGo = true;
      daTatDenSauBoGo = true;
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = true;
      daXuLy30sSauK = true;
      batCheDoChienThangV();
      pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
    } else {
      tatTatCaRelay();
      tatChanA7();
      trangThai = TRANG_THAI_CHO_LENH_S;
      daNhanLenhH = false;
      daBatR7_SauH = false;
      daBatP10_P3_SauH = false;
      boGoDangChay = false;
      daKetThuc30sBoGo = false;
      daTatDenSauBoGo = false;
      daGuiLenhJ = false;
      daNhanLenhK = false;
      daTatP13_Sau10sK = false;
      daXuLy30sSauK = false;
      daXuLyTinHieuA5 = false;
      resetBoLocA5();
      daGuiJ_BatP2 = false;
      daTatP2_BatP0_P1 = false;
      batP0_P1_TatTatCaConLai();
      pc13_relay_set_state(LED_CHOP_CHAM);
    }
  }

  if (cheDoV_DangBat) {
    uint16_t maskV = (1 << 0) | (1 << 1) | (1 << 10);
    if (g_p6DaBat) maskV |= (1 << 6);
    if ((g_pcfData & 0x1FFF) != maskV) {
      g_pcfData = maskV;
      writePCF8575(g_pcfData);
      tatChanA7();
    }
    return;
  }

  // ==========================================================================
  // 👉 MÁY TRẠNG THÁI CHÍNH
  // ==========================================================================
  switch (trangThai) {
    case TRANG_THAI_CHO_LENH_S:
      if (lenhTuSound == 'S' || lenhTuSound == 's') {
        thoiGianNhanS = now;
        tatTatCaRelay();
        batChanPCF(RELAY_LOA_P0); // Bật P0
        batChanPCF(3);            // Bật P3
        pc13_relay_set_state(LED_SANG_3S_ROI_TAT);
        trangThai = TRANG_THAI_KICH_P0_P3;
      }
      break;

    case TRANG_THAI_KICH_P0_P3:
      if (now - thoiGianNhanS >= THOI_GIAN_CHUYEN_PHA_10S_MS) {
        tatChanPCF(RELAY_LOA_P0);
        tatChanPCF(3);
        batChanPCF(RELAY_LOA_P1); // BẬT P1 (Loa gác)
        pc13_relay_set_state(LED_CHOP_CHAM);
        guiLenhSound('D');
        trangThai = TRANG_THAI_CHUYEN_P1;
      }
      break;

    case TRANG_THAI_CHUYEN_P1:
      if (lenhTuSound == 'F' || lenhTuSound == 'f') {
        batChanPCF(RELAY_LED_DO_CHOI_P5);
        tatChanPCF(3);
        batChanPCF(RELAY_LOA_P0);
        batChanPCF(RELAY_LOA_P1);
      }

      if (lenhTuSound == 'H' || lenhTuSound == 'h') {
        thoiGianNhanH = now;
        daNhanLenhH = true;
        daBatR7_SauH = false;
        daBatP10_P3_SauH = false;
      }
      break;

    case TRANG_THAI_CHO_50S_P2:
    case TRANG_THAI_CHO_LENH_K:
    case TRANG_THAI_DA_NHAN_K:
      break;
  }

  // Xử lý chuỗi H (nếu có lệnh H từ Sound)
  if (daNhanLenhH) {
    if (!daBatR7_SauH && (now - thoiGianNhanH >= THOI_GIAN_H_BAT_R7_MS)) {
      batChanPCF(RELAY_DEN_BUC_TRANH_P7);
      daBatR7_SauH = true;
    }
    if (daBatR7_SauH && !daBatP10_P3_SauH &&
        (now - thoiGianNhanH >= THOI_GIAN_H_BAT_R7_MS + THOI_GIAN_SAU_R7_BAT_P10_P3_MS)) {
      batChanPCF(RELAY_DEN_RONG_PHUONG_P10);
      batChanPCF(3);
      daBatP10_P3_SauH = true;
      thoiGianBatP10_P3 = now;
    }
  }

  // Xử lý sau lệnh K (10s tắt P13, 20s sau bật P3)
  if (daNhanLenhK) {
    if (!daTatP13_Sau10sK && (now - thoiGianNhanK >= THOI_GIAN_K_BAT_P13_MS)) {
      chuyenPha_Sau10sK_TatP13();
      daTatP13_Sau10sK = true;
    }
    if (daTatP13_Sau10sK && !daXuLy30sSauK && (now - thoiGianNhanK >= THOI_GIAN_K_TONG_MS)) {
      chuyenPha_Sau30sK_TatP13_BatP3();
      daXuLy30sSauK = true;
    }
  }

  // ==========================================================================
  // 👉 KÍCH HOẠT BỘ GÕ PA7 (LỆNH CHAI - V9.1):
  // 1. KHI NHẤC CÁI CHAI RA KHỎI CẢM BIẾN (chân PB0 / PA5): KÍCH HOẠT BỘ GÕ NGAY!
  // 2. Tự động chạy sau khi đã bật đèn P10 & P3 (dự phòng nếu người chơi không nhấc chai)
  // 3. Kích hoạt trực tiếp qua UART khi nhận lệnh 'B' hoặc 'b'
  // 4. Nút bấm cơ phụ trên PB1 hoặc PA4 (nếu có đấu nối thử nghiệm)
  // ==========================================================================
  bool nhacChaiRa = kiemTraCamBienA5_PhatHienMatTinHieu();
  bool nutBamPhu = kiemTraNutNhanPB1_BoGo() || kiemTraNutNhanPA4();
  bool tuDongKichBoGo = (daBatP10_P3_SauA6 || daBatP10_P3_SauH) && (thoiGianBatP10_P3 > 0) && (now - thoiGianBatP10_P3 >= 3000);
  bool uartKichBoGo = (lenhTuSound == 'B' || lenhTuSound == 'b');
  bool coTinHieuKichBoGo = nhacChaiRa || nutBamPhu || tuDongKichBoGo || uartKichBoGo;

  if (!daXuLyTinHieuA5 && !boGoDangChay && !daKetThuc30sBoGo && !daTatDenSauBoGo && coTinHieuKichBoGo) {
    daXuLyTinHieuA5 = true; // Chốt chỉ nhận đúng 1 lần duy nhất!
    boGoDangChay = true;
    thoiGianBatDauBoGo = now;
    thoiGianChuyenTrangThaiBoGo = now;
    chiSoMauHienTai = random(0, SO_MAU_BO_GO);
    trangThaiBoGoHienTai = true; // Bắt đầu bằng BẬT PA7
    thoiGianGiuTrangThaiHienTai = CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianBatMs;
    batBoGo(); // ⚡ BẬT BỘ GÕ QUA CHÂN PA7 (Direct GPIO, 0 I2C traffic!)
    daKetThuc30sBoGo = false;
    daTatDenSauBoGo = false;
    daGuiLenhJ = false;
    while (SoundSerial.available() > 0) {
      SoundSerial.read();
    }
  }

  // ==========================================================================
  // TIẾN TRÌNH BỘ GÕ PA7
  // - 30s: Nhấp nháy ngẫu nhiên theo CAC_MAU_BO_GO trên chân PA7
  // - Hết 30s: Tắt PA7
  // - Sau đó 10s (tổng 40s): Tắt tất cả, chỉ bật Loa P2, gửi 'J'
  // ==========================================================================
  if (boGoDangChay) {
    uint32_t thoiGianTroiQua = now - thoiGianBatDauBoGo;

    // Giai đoạn 1: Trong vòng 30s đầu -> nhấp nháy ngẫu nhiên trên chân PA7
    if (thoiGianTroiQua < THOI_GIAN_BO_GO_MS) {
      if (now - thoiGianChuyenTrangThaiBoGo >= thoiGianGiuTrangThaiHienTai) {
        thoiGianChuyenTrangThaiBoGo = now;
        trangThaiBoGoHienTai = !trangThaiBoGoHienTai;
        if (trangThaiBoGoHienTai) {
          chiSoMauHienTai = random(0, SO_MAU_BO_GO);
          thoiGianGiuTrangThaiHienTai = CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianBatMs;
          batBoGo(); // ⚡ BẬT PA7 (Không ghi I2C PCF8575!)
        } else {
          thoiGianGiuTrangThaiHienTai = CAC_MAU_BO_GO[chiSoMauHienTai].thoiGianTatMs;
          tatBoGo(); // ⚡ TẮT PA7 (Không ghi I2C PCF8575!)
        }
      }
    }
    // Giai đoạn 2: Đủ 30s -> Tắt dứt điểm bộ gõ PA7
    else if (!daKetThuc30sBoGo) {
      tatBoGo(); // Tắt dứt điểm PA7
      daKetThuc30sBoGo = true;
    }

    // Giai đoạn 3: Sau đó 10s (tổng 40s): Tắt hết, bật Loa P2, gửi 'J'
    if (thoiGianTroiQua >= THOI_GIAN_BO_GO_MS + THOI_GIAN_SAU_BO_GO_TAT_DEN_MS) {
      if (!daTatDenSauBoGo) {
        tatBoGo();                               // 1. Đảm bảo PA7 tắt
        chuyenPha_SauBoGo_BatDuyNhatP2();        // 2. ⚡ Ghi nguyên khối (Atomic): Tắt tất cả P0..P14, chỉ bật DUY NHẤT Loa P2!
        if (!daGuiLenhJ) {
          daGuiLenhJ = true;
          guiLenhSound('J');                     // 3. Gửi lệnh 'J' cho bo Sound
        }
        thoiGianGuiJ = now;                      // 4. Bắt đầu đếm thời gian chờ
        daGuiJ_BatP2 = true;
        daTatP2_BatP0_P1 = false;
        daTatDenSauBoGo = true;
        daGuiLenhJ = true;
        daNhanLenhH = false;
        daBatR7_SauH = true;
        daBatP10_P3_SauH = true;
        boGoDangChay = false;                    // Hoàn tất chu kỳ bộ gõ!
        trangThai = TRANG_THAI_CHO_50S_P2;
        while (SoundSerial.available() > 0) {
          SoundSerial.read();
        }
      }
    }
  }

  // ==========================================================================
  // XỬ LÝ CHỜ SAU KHI GỬI LỆNH 'J' & BẬT P2:
  // Hết 53 giây HOẶC nhận lệnh 'U' từ Sound: Tắt P2, Bật P0 và P1
  // ==========================================================================
  if (daGuiJ_BatP2 && !daTatP2_BatP0_P1) {
    bool hetThoiGianJ = (now - thoiGianGuiJ >= THOI_GIAN_SAU_J_TAT_P2_BAT_P0_P1_MS);
    bool coLenhU = (lenhTuSound == 'U' || lenhTuSound == 'u');

    if (hetThoiGianJ || coLenhU) {
      chuyenPha_TatP2_BatP0P1(); // TẮT P2, BẬT P0 VÀ P1
      trangThai = TRANG_THAI_CHO_LENH_K;
      daTatP2_BatP0_P1 = true;
      while (SoundSerial.available() > 0) {
        SoundSerial.read();
      }
    }
  }
}
