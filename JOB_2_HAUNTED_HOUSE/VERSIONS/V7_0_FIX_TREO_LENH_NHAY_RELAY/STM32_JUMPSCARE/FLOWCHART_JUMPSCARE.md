# LƯU ĐỒ THUẬT TOÁN (FLOWCHART) - VI ĐIỀU KHIỂN STM32_JUMPSCARE
*(Toàn bộ nội dung bằng Tiếng Việt 100%)*

---

## 1. SƠ ĐỒ KHỐI TỔNG QUAN HỆ THỐNG

```mermaid
graph TD
    A([BẮT ĐẦU CẤP ĐIỆN NGUỒN]) --> B[Khởi tạo cổng truyền thông UART 9600 baud:<br>Chân Nhận PB6 - Chân Truyền PB5]
    B --> C[Cấu hình 3 chân nút bấm: PA6, PA7, PB12<br>Chế độ kéo lên điện áp nguồn PULL-UP]
    C --> D[Cấu hình 3 chân Rơ-le: PA3, PA4, PB13<br>Ép TẮT TUYỆT ĐỐI mức LOW]
    D --> E[Gửi lệnh 'P' sang bo STM32_SOUND:<br>Báo trạng thái hệ thống đang RẢNH]
    E --> F[VÀO VÒNG LẶP CHÍNH LIÊN TỤC]
    
    F --> G[NHIỆM VỤ 1:<br>Quét 3 nút nhấn công tắc khô<br>Bộ lọc chống nhiễu, chống rung 50 mili-giây]
    F --> H[NHIỆM VỤ 2:<br>Bộ đếm thời gian tự động ngắt Rơ-le<br>Kênh 1: 71 giây | Kênh 2: 57 giây | Kênh 3: 86 giây]
    F --> I[NHIỆM VỤ 3:<br>Truyền nhận tín hiệu với bo Âm Thanh<br>Nhịp tim Heartbeat 500ms & Nhận lệnh từ xa]
    F --> J[NHIỆM VỤ 4:<br>Đèn xanh báo trạng thái PC13<br>Nháy nhanh 100ms khi chạy / Nháy chậm 500ms khi nghỉ]

    G --> F
    H --> F
    I --> F
    J --> F
```

---

## 2. LƯU ĐỒ QUÉT CÔNG TẮC & KHÓA CHỐNG KÍCH ĐÈ

```mermaid
flowchart TD
    BatDau([Bắt đầu quét kênh nút bấm: 1, 2 hoặc 3]) --> DocTinHieu[Đọc tín hiệu điện áp chân công tắc]
    DocTinHieu --> KiemTraDoiMuc{Tín hiệu có thay đổi<br>so với lần đọc trước?}
    
    KiemTraDoiMuc -- Có thay đổi --> GhiNhanMocThoiGian[Lưu lại mốc thời gian rung dội] --> KiemTraDuThoiGian
    KiemTraDoiMuc -- Giữ nguyên --> KiemTraDuThoiGian{Đã giữ nguyên ổn định<br>quá 50 mili-giây chưa?}
    
    KiemTraDuThoiGian -- Chưa đủ 50ms --> KetThucQuet([Chờ tiếp chu kỳ sau])
    KiemTraDuThoiGian -- Đã đủ 50ms --> KiemTraXacNhan{Tín hiệu mới này có khác<br>trạng thái nút đã chốt?}
    
    KiemTraXacNhan -- Giống nhau --> KetThucQuet
    KiemTraXacNhan -- Khác nhau --> ChotTrangThaiMoi[Chốt trạng thái mới của nút]
    
    ChotTrangThaiMoi --> NutCoDuocNhan{Nút bấm đang ở mức THẤP?<br>Có người chạm công tắc nối GND}
    NutCoDuocNhan -- Đang nhả nút --> KetThucQuet
    
    NutCoDuocNhan -- Đang NHẤN NÚT --> KiemTraKenhDangChay{Hiện tại đang có kênh nào<br>kích hoạt hay không?}
    
    %% Nhánh 1: Chưa có kênh nào chạy
    KiemTraKenhDangChay -- Chưa có kênh nào chạy<br>Hệ thống đang nghỉ rảnh rỗi --> BatKenhMoi[KÍCH HOẠT KÊNH NÀY:<br>1. Ghi nhận kênh này đang chạy<br>2. BẬT Rơ-le của kênh tương ứng<br>3. Bắt đầu đếm mốc thời gian]
    BatKenhMoi --> GuiTinHieuBat[BẮN LỆNH SANG BO ÂM THANH:<br>• Lệnh 'S': Ngắt ngay âm thanh nền<br>• Lệnh 'T': Tắt sạch đèn phòng]
    GuiTinHieuBat --> KetThucQuet
    
    %% Nhánh 2: Bấm lại chính kênh đang chạy (Tắt sớm)
    KiemTraKenhDangChay -- Nhấn trúng chính kênh<br>đang phát hoạt động --> TatSomKenh[TẮT SỚM KHẨN CẤP:<br>1. TẮT ngay Rơ-le của kênh này<br>2. Trả hệ thống về trạng thái nghỉ]
    TatSomKenh --> GuiTinHieuTat[BẮN LỆNH SANG BO ÂM THANH:<br>• Lệnh 'P': Cho phép mở lại âm thanh nền]
    GuiTinHieuTat --> KetThucQuet
    
    %% Nhánh 3: Khóa kích đè
    KiemTraKenhDangChay -- Có kênh KHÁC đang chạy<br>Chưa hoàn thành --> KhoaLienDong[BỊ KHÓA BẢO VỆ LIÊN ĐỘNG:<br>Không cho phép kích đè!<br>Bỏ qua hoàn toàn lệnh bấm]
    KhoaLienDong --> KetThucQuet
```

---

## 3. LƯU ĐỒ TỰ ĐỘNG TẮT KHI HẾT THỜI GIAN CỦA TỪNG KÊNH

```mermaid
flowchart TD
    KiemTraGio([Bắt đầu kiểm tra thời gian]) --> CoKenhChayKhong{Có kênh nào đang chạy<br>Rơ-le hay không?}
    
    CoKenhChayKhong -- Không có kênh nào chạy --> ThoatTimer([Thoát bộ đếm thời gian])
    
    CoKenhChayKhong -- Có kênh đang chạy --> TinhThoiGianChay{Thời gian đã chạy liên tục<br>đã vượt quá quy định chưa?<br>• Kênh 1: Quá 71 giây<br>• Kênh 2: Quá 57 giây<br>• Kênh 3: Quá 86 giây}
    
    TinhThoiGianChay -- Chưa hết thời gian --> ThoatTimer
    
    TinhThoiGianChay -- ĐÃ HẾT THỜI GIAN --> TuDongNgatRelay[TỰ ĐỘNG TẮT RƠ-LE:<br>1. Tắt Rơ-le của kênh này về mức LOW<br>2. Đưa hệ thống về trạng thái nghỉ tự do]
    TuDongNgatRelay --> GuiLenhKetThuc[GỬI LỆNH SANG BO ÂM THANH:<br>Bắn lệnh 'P': Báo hiệu đã xong, cho phép bật lại âm nền]
    GuiLenhKetThuc --> ThoatTimer
```

---

## 4. LƯU ĐỒ TRUYỀN THÔNG NỐI TIẾP VỚI BO ÂM THANH (UART)

```mermaid
flowchart TD
    subgraph PHÁT_TÍN_HIỆU_ĐỊNH_KỲ [Cứ mỗi 500 mili-giây phát 1 lần]
        DinhKyStart([Đã đến chu kỳ 500ms?]) --> KiemTraKenhPhat{Hiện tại có kênh nào<br>đang kích hoạt không?}
        KiemTraKenhPhat -- Đang chạy Rơ-le --> GuiChuS[Gửi chữ 'S' sang bo Âm Thanh:<br>Ép ngắt âm thanh nền liên tục]
        KiemTraKenhPhat -- Đang nghỉ rảnh rỗi --> GuiChuP[Gửi chữ 'P' sang bo Âm Thanh:<br>Duy trì cho phép âm thanh nền hoạt động]
    end

    subgraph ĐÓN_NHẬN_TÍN_HIỆU_ĐẾN [Đón nhận lệnh từ bo Âm Thanh gửi tới]
        NhanStart([Có dữ liệu gửi đến cổng RX?]) --> DocKyTu[Đọc ký tự nhận được]
        DocKyTu --> KiemTraKyTu{Ký tự đó là gì?}
        
        KiemTraKyTu -- Nhận chữ số '0' --> XuLyReset[LỆNH DỪNG KHẨN CẤP / RESET:<br>1. Tắt toàn bộ 3 Rơ-le ngay lập tức<br>2. Trả toàn bộ biến đếm về trạng thái ban đầu]
        
        KiemTraKyTu -- Nhận '1', '2' hoặc '3' --> XuLyKichTuXa[LỆNH KÍCH TỪ XA:<br>Nếu hệ thống đang rảnh thì kích hoạt<br>ngay Rơ-le kênh tương ứng]
        
        KiemTraKyTu -- Các ký tự khác --> BoQua[Bỏ qua không xử lý]
    end
```

---

## 5. BẢNG TỔNG HỢP CHI TIẾT CÁC KÊNH VÀ CHỨC NĂNG

| Tên Kênh | Chân Nút Nhấn (Nối Mass GND) | Chân Rơ-le (Kích Mức Cao 3.3V) | Thời Gian Chạy | Lệnh Gửi Sang Bo Âm Thanh | Ý Nghĩa Thực Tế Trong Phòng |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **Kênh 1** | **PA6** | **PA3** | **71 giây** (1 phút 11 giây) | Gửi chữ **'S'** và **'T'** | Kích hoạt hiệu ứng hù ma đợt 1 |
| **Kênh 2** | **PA7** | **PA4** | **57 giây** | Gửi chữ **'S'** và **'T'** | Kích hoạt cửa sập / chuyển động đợt 2 |
| **Kênh 3** | **PB12** | **PB13** | **86 giây** (1 phút 26 giây) | Gửi chữ **'S'** và **'T'** | Kích hoạt màn hù dọa chính đợt 3 |
| **Cổng Nhận (RX)** | **PB6** | - | Tốc độ 9600 baud | Nhận **'0'**, **'1'**, **'2'**, **'3'** | Nối chéo từ chân PB10 của bo Âm Thanh |
| **Cổng Truyền (TX)** | **PB5** | - | Tốc độ 9600 baud | Truyền **'S'**, **'P'**, **'T'** | Nối chéo sang chân PB11 của bo Âm Thanh |
| **Đèn Báo PC13** | - | **PC13** | 100ms / 500ms | - | Nháy nhanh khi đang hù, nháy chậm khi nghỉ |
