#include <Arduino.h>

// ==============================================================================
// CHƯƠNG TRÌNH TEST TẤT CẢ CÁC CHÂN GPIO TRÊN STM32F103C8T6 (BLUE PILL)
// ==============================================================================
// Tổng cộng có 30+ chân GPIO trên 2 hàng rào cắm của Blue Pill:
// - Cổng A: PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA11, PA12, PA15
//   (PA9: TX, PA10: RX dùng báo Serial máy tính)
//   (PA13, PA14 là SWDIO, SWCLK giữ nguyên để nạp code qua ST-Link, không can thiệp)
// - Cổng B: PB0, PB1, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15
// - Cổng C: PC13 (LED onboard), PC14, PC15
// ==============================================================================

extern "C" int _write(int file, char *ptr, int len) {
    (void)file; (void)ptr;
    return len;
}

// Danh sách các chân GPIO cần test
const uint8_t PINS_TO_TEST[] = {
    // Port A (12 chân)
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA11, PA12, PA15,
    // Port B (15 chân)
    PB0, PB1, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
    // Port C (3 chân)
    PC13, PC14, PC15
};

const uint8_t TOTAL_PINS = sizeof(PINS_TO_TEST) / sizeof(PINS_TO_TEST[0]);

uint32_t chu_ky = 0;

void setup() {
    // Giải phóng chân PA15, PB3, PB4 khỏi chế độ JTAG để dùng làm GPIO thường
    // (Vẫn giữ nguyên chế độ SWD để nạp code bằng ST-Link trên PA13/PA14)
    #if defined(__HAL_AFIO_REMAP_SWJ_NOJTAG)
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    #endif

    // Cấu hình tất cả chân làm OUTPUT và kéo xuống mức LOW ban đầu
    for (uint8_t i = 0; i < TOTAL_PINS; i++) {
        pinMode(PINS_TO_TEST[i], OUTPUT);
        digitalWrite(PINS_TO_TEST[i], LOW);
    }

    // Khởi động Serial 115200 (PA9-TX, PA10-RX) để báo cáo
    Serial.begin(115200);
    delay(500);

    Serial.println("\n==================================================================");
    Serial.println("  CHUONG TRINH TEST TOAN BO CAC CHAN GPIO CUA BO MACH STM32");
    Serial.println("==================================================================");
    Serial.print(">> So luong chan GPIO dang kiem tra: ");
    Serial.print(TOTAL_PINS);
    Serial.println(" chan");
    Serial.println(">> Che do kiem tra: ");
    Serial.println("   - TOAN BO CAC CHAN dong loat xuat 3.3V (1 giay) roi ve 0V (1 giay)");
    Serial.println("   - Dung dong ho VOM (thang do DCV) hoac 1 LED + dien tro 1k:");
    Serial.println("     + Que den cam vao GND");
    Serial.println("     + Que do cham vao bat ky chan nao tren mach");
    Serial.println("     + Dien ap se nhay: 3.3V -> 0V -> 3.3V -> 0V theo nhip LED PC13");
    Serial.println("==================================================================\n");
}

void loop() {
    chu_ky++;

    // 1. BẬT TẤT CẢ CÁC CHÂN LÊN MỨC CAO (3.3V)
    // Lưu ý: LED PC13 trên bo Blue Pill kích LOW, nên khi các chân khác HIGH (3.3V) thì PC13 kéo LOW để sáng
    for (uint8_t i = 0; i < TOTAL_PINS; i++) {
        if (PINS_TO_TEST[i] == PC13) {
            digitalWrite(PINS_TO_TEST[i], LOW);  // Sáng LED PC13
        } else {
            digitalWrite(PINS_TO_TEST[i], HIGH); // Xuất 3.3V ra chân
        }
    }
    Serial.print("[CHU KY ");
    Serial.print(chu_ky);
    Serial.println("] >> TAT CA CHAN DANG XUAT MUC CAO (3.3V) - LED PC13 SANG");
    delay(1000);

    // 2. TẮT TẤT CẢ CÁC CHÂN XUỐNG MỨC THẤP (0V)
    for (uint8_t i = 0; i < TOTAL_PINS; i++) {
        if (PINS_TO_TEST[i] == PC13) {
            digitalWrite(PINS_TO_TEST[i], HIGH); // Tắt LED PC13
        } else {
            digitalWrite(PINS_TO_TEST[i], LOW);  // Xuất 0V ra chân
        }
    }
    Serial.print("[CHU KY ");
    Serial.print(chu_ky);
    Serial.println("] >> TAT CA CHAN DANG VE MUC THAP (0.0V)  - LED PC13 TAT");
    delay(1000);
}

