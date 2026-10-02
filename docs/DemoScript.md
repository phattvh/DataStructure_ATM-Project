# KỊCH BẢN DEMO BẢO VỆ ĐỒ ÁN ATM (DEMO SCRIPT)
**Dự án**: Mô phỏng Hệ thống Cây ATM Ngân hàng (C++ OOP & Data Structures)  
**Tác giả**: Nhóm Đồ án - Vai trò Member A (Tech Lead)

---

## 1. Dữ liệu Thử nghiệm Ban đầu (Mock Data)
- **Tài khoản Admin**:
  - `admin1` / `123456`
  - `admin2` / `123456`
  - `admin3` / `123456`
- **Tài khoản User mẫu**:
  - `10014504500001` (Nguyễn Văn An, Số dư: 500.000 VND, PIN gốc: `123456`)
  - `10014504500002` (Trần Thị Bình, Số dư: 1.200.000 VND, PIN gốc: `123456`)
  - `10014504500003` (Lê Văn Cường, Số dư: 750.000 VND, PIN gốc: `123456`)

---

## 2. Kịch bản Trình diễn Chi tiết (Phủ trọn 5.0 điểm Chức năng)

### KỊCH BẢN 1: TÍNH TỰ PHỤC HỒI & PHÂN HỆ QUẢN TRỊ VIÊN (ADMIN)

1. **Khởi động và Tự phục hồi dữ liệu**:
   - Chạy lệnh: `.\build.ps1 run`
   - Quan sát thông báo: Hệ thống tự động tạo thư mục `data/`, khởi tạo `Admin.txt`, `TheTu.txt`, `KhoaThe.txt` và 10 tài khoản kèm file lịch sử rỗng.
2. **Đăng nhập Admin**:
   - Chọn `1` (Đăng nhập Admin).
   - Nhập sai mật khẩu 1-2 lần để biểu diễn thông báo cảnh báo và bộ đếm số lần còn lại.
   - Nhập đúng: `admin1` / `123456` (mật khẩu hiển thị dấu `*`). Đăng nhập thành công.
3. **Xem danh sách thẻ từ**:
   - Chọn `1` trong Menu Admin.
   - Màn hình hiển thị bảng danh sách 10 thẻ từ ban đầu với màu sắc trực quan (Trạng thái màu xanh "Hoạt động").
4. **Thêm thẻ mới**:
   - Chọn `2`.
   - Nhập ID: `11112222333344` (14 số).
   - Nhập Tên: `Pham Nhat Vuong`.
   - Nhập Số dư ban đầu: `100000` VND.
   - Hệ thống báo thành công, cấp mã PIN mặc định `123456`.
   - Mở thư mục `data/` để giảng viên chứng thực có sinh ra `11112222333344.txt` và `LichSu11112222333344.txt`.
5. **Xóa thẻ**:
   - Chọn `3`.
   - Nhập ID: `11112222333344`. Xác nhận `y`.
   - Kiểm tra thư mục: File `11112222333344.txt` đã bị xóa, nhưng file `LichSu11112222333344.txt` vẫn được giữ nguyên vẹn để phục vụ kiểm toán ngân hàng.

---

### KỊCH BẢN 2: PHÂN HỆ KHÁCH HÀNG (USER) & CÁC RÀNG BUỘC BẢO MẬT

1. **Bảo mật Đăng nhập & Khóa thẻ**:
   - Từ Menu chính, chọn `2` (Đăng nhập User).
   - Nhập ID: `10014504500001`.
   - Nhập sai mã PIN 3 lần liên tiếp: `000000`, `111111`, `222222`.
   - Hệ thống cảnh báo đỏ, tự động khóa thẻ, ghi ID vào `KhoaThe.txt` và đóng chương trình theo đúng quy định.
2. **Thử đăng nhập lại thẻ bị khóa**:
   - Chạy lại ứng dụng, chọn Đăng nhập User, nhập `10014504500001`.
   - Hệ thống chặn ngay từ đầu: "Thẻ đang bị khóa do nhập sai PIN quá 3 lần! Vui lòng liên hệ Admin".
3. **Admin mở khóa thẻ**:
   - Đăng nhập Admin (`admin1` / `123456`).
   - Chọn `4` (Mở khóa thẻ).
   - Chọn ID `10014504500001`. Mở khóa thành công, số lần nhập sai được đặt lại về 0.
   - Đăng xuất Admin (`0`).

---

### KỊCH BẢN 3: ĐĂNG NHẬP LẦN ĐẦU & ÉP ĐỔI MÃ PIN MẶC ĐỊNH

1. **Ép đổi PIN mặc định**:
   - Vào Đăng nhập User: `10014504500001`, nhập PIN mặc định `123456`.
   - Hệ thống phát hiện PIN mặc định, chặn không cho vào Menu, bắt buộc đổi PIN.
   - Thử nhập lại `123456` -> Báo lỗi: "Không được trùng mã mặc định".
   - Nhập PIN mới: `888888`, xác nhận `888888`.
   - Đổi PIN thành công và tự động vào Menu User.

---

### KỊCH BẢN 4: GIAO DỊCH RÚT TIỀN & KIỂM TRA RÀNG BUỘC

1. **Xem thông tin tài khoản**:
   - Chọn `1` (Xem thông tin): Hiển thị Tên: Nguyễn Văn An, Số dư: 500.000 VND.
2. **Rút tiền với các bẫy validation**:
   - Chọn `2` (Rút tiền).
   - Thử rút `40000` -> Báo lỗi: "Số tiền tối thiểu 50.000 VND". Hệ thống hỏi muốn nhập lại không -> chọn `y`.
   - Thử rút `75000` -> Báo lỗi: "Số tiền phải là bội số của 50.000 VND". Chọn `y`.
   - Thử rút `500000` -> Báo lỗi: "Số dư không đủ! Phải duy trì tối thiểu 50.000 VND". Chọn `y`.
   - Nhập số tiền hợp lệ: `100000` VND. Xác nhận `y`.
   - Rút tiền thành công. Số dư mới còn `400.000 VND`.

---

### KỊCH BẢN 5: CHUYỂN TIỀN NGUYÊN TỬ (ATOMICITY) & LỊCH SỬ HAI CHIỀU

1. **Chuyển tiền**:
   - Chọn `3` (Chuyển tiền).
   - Nhập chính ID mình (`10014504500001`) -> Bị từ chối: "Không thể tự chuyển cho chính mình".
   - Nhập ID người nhận: `10014504500002`.
   - Hệ thống hiển thị: "Tài khoản nhận: Trần Thị Bình".
   - Nhập số tiền chuyển: `150000` VND. Xác nhận `y`.
   - Chuyển tiền thành công!
   - Số dư người gửi (`10014504500001`): `400.000 - 150.000 = 250.000 VND`.
2. **Xem lịch sử giao dịch**:
   - Chọn `4` (Xem lịch sử giao dịch).
   - Màn hình in ra chi tiết các giao dịch vừa thực hiện:
     1. Rút tiền: -100.000 VND (kèm ngày giờ).
     2. Chuyển tiền: -150.000 VND cho TK 10014504500002 (Trần Thị Bình).
3. **Đăng xuất an toàn**:
   - Chọn `0` (Trả thẻ - Thoát). Vùng nhớ tài khoản giải phóng hoàn toàn trong RAM.

---

### KỊCH BẢN 6: CHỨNG THỰC BÊN NGƯỜI NHẬN

1. **Đăng nhập tài khoản người nhận (`10014504500002`)**:
   - Đăng nhập bằng ID `10014504500002`, PIN `123456` -> Đổi sang `654321`.
   - Chọn `1` (Xem thông tin): Số dư ban đầu `1.200.000 + 150.000 = 1.350.000 VND` (Chính xác 100%).
   - Chọn `4` (Xem lịch sử): Thấy ngay dòng nhận tiền: "Nhận tiền từ TK: 10014504500001 (Nguyen Van An)" kèm thời gian thực chuẩn `YYYY-MM-DD HH:MM:SS`.
