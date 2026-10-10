# ATM Simulation Project (Hệ thống Mô phỏng Máy ATM)

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Course](https://img.shields.io/badge/Course-Data%20Structures%20%26%20Algorithms-green.svg)](https://cntt.hcmue.edu.vn/)
[![Institution](https://img.shields.io/badge/University-HCMUE-red.svg)](https://hcmue.edu.vn/)
[![Standard](https://img.shields.io/badge/Coding%20Standard-C%2B%2B%20Standard%20V2-orange.svg)](docs/01_tieu_chi_cham.md)

Đồ án môn học **Cấu trúc Dữ liệu & Giải thuật** (Khoa Công nghệ Thông tin - Trường Đại học Sư phạm TP. Hồ Chí Minh).  
Dự án tập trung vào việc áp dụng mô hình **Lập trình Hướng đối tượng (OOP)** và **Cấu trúc dữ liệu Generic Template** tự xây dựng để mô phỏng hoạt động thực tế của một cây ATM ngân hàng.

---

## THÀNH VIÊN NHÓM THỰC HIỆN

| STT | Thành viên | Vai trò & Phụ trách                                                                                         |                                Nhánh Git                                 |
| :-: | :--------- | :---------------------------------------------------------------------------------------------------------- | :----------------------------------------------------------------------: |
|  1  | **Phát**   | **Tech Lead** / Core Controller, Luồng nghiệp vụ Admin, Khởi tạo dự án & Makefile                           | [`phat`](https://github.com/phattvh/DataStructure_ATM-Project/tree/phat) |
|  2  | **Trí**    | **Data Engineer** / Template Cấu trúc dữ liệu `LinkedList<T>`, Tầng lưu trữ `FileService`, Kiểm định bộ nhớ |  [`tri`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tri)  |
|  3  | **Tuấn**   | **Business & UI Engineer** / Tầng giao diện `ConsoleView`, Models nghiệp vụ, Luồng User, Báo cáo & Video    | [`tuan`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tuan) |

---

## ĐẶC ĐIỂM NỔI BẬT & TIÊU CHUẨN KỸ THUẬT

1. **Cấu trúc dữ liệu Generic Template (`LinkedList<T>`)**:
   - Tự cài đặt danh sách liên kết đơn có con trỏ đuôi `_pTail` (không sử dụng `std::vector` hay `std::list` của STL).
   - Thao tác thêm cuối (`addTail`) đạt độ phức tạp tối ưu $\mathcal{O}(1)$.
   - Quản lý bộ nhớ nghiêm ngặt: Thu hồi toàn bộ node trong Destructor, vô hiệu hóa Copy Constructor (`= delete`) để loại bỏ triệt để lỗi `Double Free`.
2. **Tuân thủ chuẩn C++ Coding Standard Version 2**:
   - Đặt tên biến theo **Hungarian Notation** (`strId`, `iCount`, `lBalance`, `bIsLocked`, `pNode`).
   - Biến thành viên class có tiền tố `_` (`_strId`, `_lBalance`).
   - Tách bạch 100% giữa tệp giao diện khai báo `.h` và tệp hiện thực `.cpp`.
   - Sử dụng tường minh con trỏ `this->` khi truy xuất thành viên nội bộ.
   - Format chú thích hàm chuẩn `@Description`, `@return`, `@attention`.
3. **Bảo mật & Trải nghiệm Console (CLI UX)**:
   - Mã hóa thời gian thực mật khẩu Admin và mã PIN User thành ký tự `*` (hỗ trợ xóa lùi phím Backspace).
   - Thiết kế giao diện màu sắc trực quan qua mã màu chuẩn **ANSI Color** (Xanh lá: thành công, Đỏ: lỗi, Vàng: cảnh báo, Cyan: bảng thông tin).
   - Bẫy lỗi ngoại lệ `cin.fail()` ngăn chặn hoàn toàn lỗi sập ứng dụng hoặc lặp vô hạn khi người dùng gõ chữ vào trường số tiền.
4. **Hệ thống Lưu trữ Tệp tin Bền vững (Flat-file Storage)**:
   - Tự động quản lý và đồng bộ 5 loại tệp: `Admin.txt`, `TheTu.txt`, `KhoaThe.txt`, `[ID].txt`, `[LichSuID].txt`.
   - Giao dịch chuyển tiền đảm bảo tính nguyên tử (Atomicity - ghi log thời gian thực ở cả hai tài khoản gửi và nhận).

---

## CẤU TRÚC THƯ MỤC DỰ ÁN

```text
DataStructure_ATM-Project/
├── Makefile                          # Script biên dịch tự động (g++ -std=c++17 -Wall -Wextra)
├── README.md                         # Tài liệu giới thiệu chính của repository
├── Project/                          # Tài liệu đề bài và biểu mẫu gốc từ giảng viên
│   ├── Đề Bài.pdf
│   ├── Yêu cầu thực hiện Project.pdf
│   ├── Tài Liệu Đọc.pdf             # Quy chuẩn C++ Coding Standard V2
│   └── Mau_BaoCao_DoAn.docx
├── data/                             # Thư mục cơ sở dữ liệu tệp tin (.txt)
│   ├── Admin.txt                     # Danh sách tài khoản quản trị (>= 3 tài khoản)
│   ├── TheTu.txt                     # Danh sách thẻ từ (>= 10 thẻ, gồm ID và PIN)
│   ├── KhoaThe.txt                   # Danh sách ID thẻ đang bị khóa do nhập sai 3 lần
│   ├── [ID].txt                      # Thông tin chi tiết từng tài khoản (Tên, Số dư, Tiền tệ)
│   └── LichSu[ID].txt                # Nhật ký biến động số dư theo thời gian thực
├── docs/                             # Hệ thống tài liệu kỹ thuật chi tiết
│   ├── README.md                     # Mục lục điều hướng tài liệu
│   ├── 01_tieu_chi_cham.md           # Thang điểm chi tiết & 4 bẫy kỹ thuật C++
│   ├── 02_yeu_cau_va_pham_vi.md      # Phạm vi nghiệp vụ & Đặc tả yêu cầu
│   ├── 03_kien_truc_he_thong.md      # Kiến trúc phân tầng Layered Architecture
│   ├── 04_ctdl_va_thuat_toan.md      # Thiết kế Template LinkedList & Phân tích Big-O
│   ├── 05_thiet_ke_chi_tiet.md       # Thiết kế chi tiết từng Class & Method
│   ├── 06_ke_hoach_trien_khai.md     # Phân công công việc & Lộ trình 14 ngày
│   └── 07_ke_hoach_kiem_thu.md       # Ma trận Test Cases & Kiểm định Valgrind
├── include/                          # Tệp tin Header (*.h)
│   ├── Common.h                      # Hằng số, Enum, ErrorCode
│   ├── LinkedList.h                  # Template Class Cấu trúc dữ liệu
│   ├── Admin.h                       # Model Admin
│   ├── Card.h                        # Model Thẻ từ
│   ├── Account.h                     # Model Tài khoản
│   ├── Transaction.h                 # Model Lịch sử giao dịch
│   ├── FileService.h                 # Dịch vụ thao tác tệp tin
│   ├── ConsoleView.h                 # Tiện ích giao diện Console
│   └── AtmController.h               # Bộ điều khiển trung tâm
├── src/                              # Tệp tin Hiện thực (*.cpp)
│   ├── Admin.cpp
│   ├── Card.cpp
│   ├── Account.cpp
│   ├── Transaction.cpp
│   ├── FileService.cpp
│   ├── ConsoleView.cpp
│   ├── AtmController.cpp
│   └── main.cpp                      # Điểm vào chính của ứng dụng
└── test/                             # Kiểm thử tự động (Unit Test)
    └── test_atm.cpp                  # Test suite kiểm tra CTDL và Logic
```

---

## HỆ THỐNG TÀI LIỆU KỸ THUẬT (DOCS)

Nhóm đã xây dựng tài liệu chi tiết cho từng giai đoạn phát triển tại thư mục [`docs/`](docs/):

- [**01. Tiêu chí chấm điểm & Bẫy kỹ thuật**](docs/01_tieu_chi_cham.md): Phân bổ điểm số (10.0đ), các bẫy `std::getline` khoảng trắng, bẫy `cin.fail()`, thời gian thực và quản lý bộ nhớ.
- [**02. Yêu cầu & Phạm vi dự án**](docs/02_yeu_cau_va_pham_vi.md): Quy định chức năng Admin, chức năng User, các ràng buộc giao dịch ($\ge 50.000$ VNĐ, bội số của $50.000$ VNĐ, số dư duy trì tối thiểu $50.000$ VNĐ).
- [**03. Kiến trúc hệ thống**](docs/03_kien_truc_he_thong.md): Sơ đồ phân tầng 5 lớp (Presentation, Controller, Domain, Service, Foundation), cơ chế cách ly phiên (Session Isolation) và tự phục hồi dữ liệu (Auto-Recovery).
- [**04. Cấu trúc dữ liệu & Thuật toán**](docs/04_ctdl_va_thuat_toan.md): Cài đặt Template `LinkedList<T>`, quy tắc Rule 17 (New/Delete), bảng độ phức tạp Big-O và so sánh với Mảng tĩnh / Dynamic Array.
- [**05. Thiết kế chi tiết**](docs/05_thiet_ke_chi_tiet.md): Đặc tả chi tiết các thuộc tính, phương thức, giá trị trả về, kiểm soát lỗi cho toàn bộ file mã nguồn.
- [**06. Kế hoạch triển khai & Phân công**](docs/06_ke_hoach_trien_khai.md): Lộ trình 14 ngày (4 Phase), ma trận phân công RACI, quy chuẩn đặt tên nhánh và commit message.
- [**07. Kế hoạch kiểm thử & QA**](docs/07_ke_hoach_kiem_thu.md): Ma trận kịch bản Test Cases (Admin, User, Giao dịch), kiểm thử tấn công input và lệnh kiểm tra rò rỉ bộ nhớ với Valgrind.

---

## HƯỚNG DẪN BIÊN DỊCH & CHẠY ỨNG DỤNG

### 1. Yêu cầu môi trường

- **Hệ điều hành**: Linux (Ubuntu 20.04/22.04/24.04), macOS, hoặc Windows (hỗ trợ WSL / MinGW / MSVC).
- **Trình biên dịch**: `g++` hoặc `clang++` hỗ trợ chuẩn **C++17** trở lên.
- **Công cụ xây dựng**: `make`.
- **Công cụ kiểm định bộ nhớ (tùy chọn)**: `valgrind` (trên Linux).

### 2. Các lệnh thao tác với `Makefile`

```bash
# 1. Biên dịch toàn bộ chương trình (sản phẩm nằm tại bin/atm_app)
make

# 2. Khởi chạy ứng dụng máy ATM
make run

# 3. Biên dịch và chạy bộ kiểm thử tự động (Unit Test cho LinkedList)
make test

# 4. Kiểm tra rò rỉ bộ nhớ với Valgrind (Yêu cầu môi trường Linux có cài valgrind)
make memcheck

# 5. Dọn dẹp các tệp tin biên dịch tạm (.o) và tệp thực thi (.exe / binary)
make clean
```

---

## QUY CHUẨN LÀM VIỆC VỚI GIT

### 1. Cấu trúc nhánh

- `main`: Nhánh ổn định, chỉ chứa mã nguồn đã qua kiểm thử và nghiệm thu.
- `phat`: Nhánh phát triển của Thành viên Phát.
- `tri`: Nhánh phát triển của Thành viên Trí.
- `tuan`: Nhánh phát triển của Thành viên Tuấn.

### 2. Tiêu chuẩn viết Commit Message

Nội dung commit sử dụng tiếng Anh chuẩn mực, bắt đầu bằng các tiền tố quy định:

- `feat:` Bổ sung tính năng mới (ví dụ: `feat: implement LinkedList generic template`).
- `fix:` Sửa lỗi logic hoặc xử lý ngoại lệ (ví dụ: `fix: handle cin.fail on money input`).
- `docs:` Cập nhật tài liệu (ví dụ: `docs: update system architecture and big-o table`).
- `style:` Căn chỉnh định dạng mã nguồn theo chuẩn Hungarian Notation mà không đổi logic.
- `refactor:` Tái cấu trúc mã nguồn.
- `test:` Bổ sung kịch bản kiểm thử tự động.

---

## 📜 GIẤY PHÉP & BẢN QUYỀN

Dự án được thực hiện phục vụ mục đích học tập và nghiên cứu trong khuôn khổ môn học Cấu trúc Dữ liệu & Giải thuật tại Trường Đại học Sư phạm TP. Hồ Chí Minh.
