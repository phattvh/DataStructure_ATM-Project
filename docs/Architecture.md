# 03. KIẾN TRÚC HỆ THỐNG (SYSTEM ARCHITECTURE)
**Tác giả phụ trách**: Thành viên A (Tech Lead / Core System & Admin Flow)

---

## 1. Tổng quan Kiến trúc

Hệ thống mô phỏng cây ATM ngân hàng được thiết kế theo mô hình kiến trúc phân lớp (Layered Architecture) kết hợp Hướng đối tượng (OOP) và Cấu trúc dữ liệu Generic Template tự cài đặt. Kiến trúc chia rõ trách nhiệm giữa tầng Giao diện (Presentation), tầng Điều khiển nghiệp vụ (Controller/Business Logic), tầng Dữ liệu bộ nhớ (Data Structure & Domain Models) và tầng Lưu trữ tệp tin (Persistence/Flat-file Storage).

```
+-------------------------------------------------------------+
|                  TẦNG GIAO DIỆN (ConsoleView)               |
|   - ANSI Color, hiển thị bảng, che dấu PIN/Password (*)     |
|   - Bẫy lỗi luồng nhập liệu cin.fail()                      |
+------------------------------+------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|               TẦNG ĐIỀU PHỐI (AtmController)                |
|   - Quản lý trạng thái phiên đăng nhập (Session)            |
|   - Luồng nghiệp vụ Quản trị viên (Admin Module)            |
|   - Luồng nghiệp vụ Khách hàng (User Module)                |
+---------------+-----------------------------+---------------+
                |                             |
                v                             v
+-------------------------------+  +--------------------------+
|  TẦNG THỰC THỂ (Domain Models)|  | TẦNG LƯU TRỮ (FileService|
|  - Admin                      |  |  - Admin.txt             |
|  - Card                       |  |  - TheTu.txt             |
|  - Account                    |  |  - KhoaThe.txt           |
|  - Transaction                |  |  - [ID].txt              |
+---------------+---------------+  |  - LichSu[ID].txt        |
                |                  +--------------------------+
                v
+-------------------------------------------------------------+
|        CẤU TRÚC DỮ LIỆU TỰ XÂY DỰNG (LinkedList<T>)         |
|   - Template Node<T> & LinkedList<T>                        |
|   - Tự quản lý bộ nhớ động (new / delete, 0 memory leak)    |
|   - Delete Copy Semantics để triệt tiêu lỗi Double Free     |
+-------------------------------------------------------------+
```

---

## 2. Chi tiết các Phân lớp & Trách nhiệm

### 2.1. Tầng Điều phối (AtmController)
- **Vai trò**: Trung tâm điều khiển toàn bộ luồng chương trình.
- **Trách nhiệm**:
  - Khởi tạo hệ thống (`initSystem`): Kiểm tra và phục hồi dữ liệu gốc (`initMockDataIfMissing`), nạp dữ liệu từ tệp tin vào danh sách liên kết trên RAM.
  - Vòng lặp Menu chính (`run`): Điều hướng đăng nhập Admin, đăng nhập User hoặc thoát hệ thống.
  - Phân hệ Admin (`processAdminLogin`, `processAdminMenu`):
    - Kiểm tra đăng nhập với tối đa 3 lần thử.
    - Xem danh sách thẻ từ trong hệ thống.
    - Thêm tài khoản thẻ mới (kiểm tra định dạng 14 số, gán PIN mặc định 123456, tự sinh file `[ID].txt` và `LichSu[ID].txt`).
    - Xóa thẻ: Gỡ bỏ khỏi RAM, cập nhật `TheTu.txt`, xóa `[ID].txt` và bảo toàn `LichSu[ID].txt`.
    - Mở khóa thẻ: Đọc danh sách khóa từ `KhoaThe.txt`, đặt lại số lần nhập sai về 0, mở trạng thái khóa.
  - Quản lý bộ nhớ: Tự động dọn dẹp các danh sách liên kết khi thoát, hủy con trỏ tài khoản hiện tại để ngăn ngừa thất thoát bộ nhớ.

### 2.2. Tầng Lưu trữ & Quản lý Tệp (FileService)
- **Vai trò**: Cung cấp các phương thức tĩnh (`static`) độc lập để đọc/ghi tệp tin văn bản flat-file.
- **Nguyên tắc kỹ thuật**:
  - Luôn mở file và kiểm tra `is_open()`.
  - Luôn gọi `file.close()` trước khi thoát hàm (Rule 18).
  - Sử dụng chế độ `ios::trunc` khi ghi đè số dư/danh sách thẻ, `ios::app` khi ghi nối dòng lịch sử giao dịch.
  - Tuyệt đối không xóa tệp tin lịch sử khi xóa tài khoản nhằm đảm bảo tính toàn vẹn kiểm toán ngân hàng.

### 2.3. Tầng Cấu trúc Dữ liệu Bộ nhớ (LinkedList<T>)
- **Thiết kế Generic Template**: Tái sử dụng linh hoạt cho mọi kiểu dữ liệu (`LinkedList<Admin>`, `LinkedList<Card>`, `LinkedList<string>`).
- **Tối ưu hóa độ phức tạp**:
  - `addTail`: Thêm phần tử qua con trỏ `_pTail` với độ phức tạp $O(1)$.
  - `getSize`: Trả về `_iSize` với độ phức tạp $O(1)$.
  - `findIf`: Tìm kiếm tuần tự với độ phức tạp $O(N)$.
  - `removeIf`: Xóa node và nối lại liên kết với độ phức tạp $O(N)$ tìm kiếm + $O(1)$ thao tác nối.
- **An toàn bộ nhớ**:
  - Destructor tự động dọn dẹp toàn bộ node (`delete pCurrent`).
  - Xóa bỏ Copy Constructor và Copy Assignment Operator (`= delete`) để loại bỏ nguy cơ con trỏ trỏ lậu và lỗi Double Free khi truyền tham số.

### 2.4. Tầng Giao diện (ConsoleView)
- Độc lập hoàn toàn với tầng Logic (không chứa nghiệp vụ ngân hàng bên trong).
- Sử dụng mã ANSI để hiển thị màu sắc trực quan (Xanh lá: Thành công, Đỏ: Lỗi, Vàng: Cảnh báo, Cyan/Xanh dương: Tiêu đề).
- Che mật khẩu/mã PIN theo thời gian thực thành ký tự `*` và hỗ trợ phím Backspace xóa lùi.
- Bẫy lỗi `cin.fail()` chống tràn bộ đệm khi nhập sai kiểu dữ liệu.

---

## 3. Tính Toàn vẹn & An toàn Hệ thống
1. **Khả năng tự phục hồi (Auto-Recovery)**: Khởi động không crash nếu thiếu thư mục `data/` hoặc file dữ liệu gốc; hệ thống tự tạo mock data (3 Admin, 10 thẻ).
2. **Cách ly phiên làm việc (Session Isolation)**: Dữ liệu tài khoản chỉ tải lên RAM khi đăng nhập và được giải phóng ngay khi đăng xuất.
3. **Tuân thủ chuẩn Coding Standard V2**: Đặt tên Hungarian, PascalCase, const getter, `this->`, không memory leak.
