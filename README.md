# ĐỒ ÁN CẤU TRÚC DỮ LIỆU & GIẢI THUẬT: HỆ THỐNG MÔ PHỎNG ATM (C++)

Dự án mô phỏng hoạt động thực tế của cây ATM ngân hàng bằng ngôn ngữ C++17, sử dụng kiến trúc Hướng đối tượng (OOP) kết hợp Cấu trúc dữ liệu Generic Template tự cài đặt (`LinkedList<T>`), lưu trữ dữ liệu bằng hệ thống Flat Files (`.txt`).

---

## 1. Cấu trúc Thư mục Dự án

```
CTDL/
├── Makefile              # Makefile chuẩn biên dịch g++ c++17
├── build.ps1             # Script hỗ trợ biên dịch nhanh trên Windows PowerShell
├── .gitignore            # Loại bỏ các file nhị phân, đối tượng tạm (.exe, .o)
├── README.md             # Hướng dẫn dự án và bàn giao
├── data/                 # Thư mục chứa dữ liệu Flat Files (.txt)
│   ├── Admin.txt         # Danh sách tài khoản Quản trị viên
│   ├── TheTu.txt         # Danh sách thẻ từ (ID 14 số, PIN 6 số)
│   ├── KhoaThe.txt       # Danh sách ID thẻ bị khóa
│   ├── [ID].txt          # Chi tiết tài khoản (Tên, Số dư, Tiền tệ)
│   └── LichSu[ID].txt    # Nhật ký lịch sử biến động số dư
├── include/              # Khai báo Header (.h)
│   ├── Common.h          # Hằng số hệ thống, Enums, ErrorCode
│   ├── LinkedList.h      # Generic Template LinkedList<T> tự quản lý bộ nhớ
│   ├── Admin.h           # Class Model Admin
│   ├── Card.h            # Class Model Thẻ từ
│   ├── Account.h         # Class Model Tài khoản
│   ├── Transaction.h     # Class Model Giao dịch
│   ├── FileService.h     # Tầng giao tiếp đọc/ghi file
│   ├── ConsoleView.h     # Tiện ích giao diện CLI, màu ANSI, bẫy lỗi cin
│   └── AtmController.h   # Bộ điều phối trung tâm hệ thống ATM
├── src/                  # Mã nguồn hiện thực (.cpp)
│   ├── Admin.cpp
│   ├── Card.cpp
│   ├── Account.cpp
│   ├── Transaction.cpp
│   ├── FileService.cpp
│   ├── ConsoleView.cpp
│   ├── AtmController.cpp
│   └── main.cpp          # Điểm vào chương trình
├── test/
│   └── test_atm.cpp      # Bộ kiểm thử tự động (Unit Test Suite)
└── docs/                 # Tài liệu kỹ thuật
    ├── Architecture.md   # Phần 03: Kiến trúc hệ thống
    └── GitRules.md       # Phần 09: Quy tắc phát triển & Git
```

---

## 2. Kết quả Bàn giao Tuần 1 - Thành viên A (Tech Lead)

### Mục tiêu Tuần 1 đã hoàn thành:
1. **Task 1.1: Khởi tạo dự án & Tiêu chuẩn hóa (Ngày 1)**:
   - Khởi tạo Git repository, cấu hình `.gitignore`.
   - Viết `Makefile` chuẩn và script `build.ps1` hỗ trợ PowerShell.
   - Định nghĩa `Common.h` chuẩn Header Guards, hằng số UPPERCASE, Enums.
2. **Task 1.2: Xây dựng AtmController Base (Ngày 2 - Ngày 3)**:
   - Thiết kế `AtmController.h` và `AtmController.cpp`.
   - Vòng lặp Menu chính, bẫy lỗi nhập số và dọn dẹp bộ nhớ an toàn khi thoát (`clear()`).
3. **Task 1.3: Luồng Đăng nhập Admin & Xem danh sách thẻ (Ngày 4 - Ngày 5)**:
   - Xử lý đăng nhập Admin tối đa 3 lần, ẩn mật khẩu thành ký tự `*`.
   - Duyệt danh sách liên kết `LinkedList<Card>` và in bảng thẻ từ chuẩn hóa định dạng.
4. **Task 1.4: Luồng Thêm / Xóa / Mở khóa thẻ (Ngày 6 - Ngày 7)**:
   - Thêm thẻ: Kiểm tra 14 số, gán PIN mặc định `123456`, tự động sinh 2 file `[ID].txt` và `LichSu[ID].txt`.
   - Xóa thẻ: Xóa khỏi RAM, cập nhật `TheTu.txt`, xóa `[ID].txt` nhưng bảo toàn `LichSu[ID].txt`.
   - Mở khóa thẻ: Duyệt danh sách thẻ bị khóa, reset số lần nhập sai về 0, gỡ ID khỏi `KhoaThe.txt`.
5. **Nhiệm vụ Kiểm thử (Unit Testing)**:
   - File `test/test_atm.cpp` kiểm thử 5 ca tự động: LinkedList, Đăng nhập Admin, Thuật toán Khóa/Mở khóa, Định dạng ID 14 số, Vòng đời tài khoản. Tất cả đều đạt 100% Pass.
6. **Nhiệm vụ Tài liệu (Documentation)**:
   - Hoàn thành `docs/Architecture.md` (Mục 03 trong báo cáo).
   - Hoàn thành `docs/GitRules.md` (Mục 09 trong báo cáo).

---

## 3. Hướng dẫn Biên dịch & Chạy chương trình

### Trên Windows (PowerShell):
```powershell
# 1. Chạy bộ kiểm thử tự động
.\build.ps1 test

# 2. Biên dịch ứng dụng chính
.\build.ps1 all

# 3. Chạy ứng dụng ATM
.\build.ps1 run
# Hoặc chạy trực tiếp:
.\atm_project.exe
```

### Sử dụng Make (nếu có môi trường Linux/MinGW/make):
```bash
make clean
make all
./atm_project.exe
make test
```
