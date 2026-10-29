# TÀI LIỆU MẠCH STM32_JUMPSCARE (BẢN MỚI)

## 1. Chức năng chính
- Điều khiển phát âm thanh thông qua module **DFPlayer Mini** bằng cổng phần cứng UART3 (PB10 TX, PB11 RX).
- Quét 3 tiếp điểm Relay (PA1, PA2, PA3) tương ứng 3 bài hát:
  - Relay 1 (`PA1`) -> Bài 0001
  - Relay 2 (`PA2`) -> Bài 0002
  - Relay 3 (`PA3`) -> Bài 0003
- Báo hiệu qua 3 đèn LED riêng biệt (âm chung, kích mức CAO qua trở 330Ω):
  - LED 1 (`PA7`): Đèn báo bài 0001
  - LED 2 (`PA6`): Đèn báo bài 0002
  - LED 3 (`PB4`): Đèn báo bài 0003
- Đèn báo trạng thái mạch trên chân `PC13`:
  - Khởi động: Nháy 3 lần.
  - Đang phát nhạc: Sáng liên tục (`LOW`).
  - Khi rảnh hoặc dừng bài: Tắt (`HIGH`).
  - Khi đang phát bài này mà bấm kích bài khác: Nháy cảnh báo nhanh.
  
## 2. Nguyên lý hoạt động
- **Khi đang rảnh (`currentPlayingTrack == 0`)**: Kích Relay nào sẽ phát ngay bài đó và sáng đèn LED tương ứng.
- **Khi đang phát bài hiện tại mà bấm lại chính Relay đó**: Dừng phát ngay lập tức (Toggle/Hủy phát) và tắt LED.
- **Khi đang phát mà bấm Relay bài khác**: Bị khóa liên động, không ngắt bài đang chạy, đồng thời nháy đèn PC13 cảnh báo.
- **Tự động ngắt khi hết bài**: Lắng nghe phản hồi từ DFPlayer Mini (mã `0x3D`), tự chuyển về trạng thái rảnh và tắt toàn bộ đèn LED.

## 3. Sơ đồ nối dây
| Thiết bị | Chân STM32 | Ghi chú |
| :--- | :--- | :--- |
| DFPlayer RX | **PB10** (TX) | Nối qua trở 1kΩ |
| DFPlayer TX | **PB11** (RX) | Nối trực tiếp |
| Relay 1 (Bài 0001) | **PA1** | Kéo lên nội (`INPUT_PULLUP`), kích GND |
| Relay 2 (Bài 0002) | **PA2** | Kéo lên nội (`INPUT_PULLUP`), kích GND |
| Relay 3 (Bài 0003) | **PA3** | Kéo lên nội (`INPUT_PULLUP`), kích GND |
| LED 1 (Báo bài 0001) | **PA7** | Kích mức `HIGH` qua trở 330Ω |
| LED 2 (Báo bài 0002) | **PA6** | Kích mức `HIGH` qua trở 330Ω |
| LED 3 (Báo bài 0003) | **PB4** | Kích mức `HIGH` qua trở 330Ω |
| LED trạng thái mạch | **PC13** | Tích hợp trên mạch |
