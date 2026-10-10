# ATM Simulation Project (Hệ thống Mô phỏng Máy ATM Ngân hàng)

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Course](https://img.shields.io/badge/Course-Data%20Structures%20%26%20Algorithms-green.svg)](https://cntt.hcmue.edu.vn/)
[![Institution](https://img.shields.io/badge/University-HCMUE-red.svg)](https://hcmue.edu.vn/)
[![Standard](https://img.shields.io/badge/Coding%20Standard-C%2B%2B%20Standard%20V2-orange.svg)](docs/01_tieu_chi_cham.md)
[![Memory Safety](https://img.shields.io/badge/Memory%20Leak-0%20bytes%20(100%25%20Clean)-brightgreen.svg)](test/test_memory_leak.cpp)
[![Test Suite](https://img.shields.io/badge/Tests-100%25%20Passed-success.svg)](test/)

Đồ án môn học **Cấu trúc Dữ liệu & Giải thuật** — Khoa Công nghệ Thông tin, Trường Đại học Sư phạm TP. Hồ Chí Minh (HCMUE).  
Dự án áp dụng mô hình **Lập trình Hướng đối tượng (OOP)** và **Cấu trúc dữ liệu Generic Template** tự xây dựng để mô phỏng hoạt động thực tế của hệ thống máy ATM ngân hàng.

---

## THÀNH VIÊN THỰC HIỆN

| STT | Họ và Tên | Vai trò & Phụ trách | Nhánh Git |
| :-: | :--- | :--- | :---: |
| 1 | **Trần Vũ Hỏa Phát** | **Tech Lead / Core Controller & Admin Flow**<br>- Thiết kế kiến trúc 5 tầng, `Common.h`, Makefile.<br>- Xây dựng bộ điều phối trung tâm `AtmController`, `AdminController`.<br>- Bảo mật Terminal Raw Mode, Atomic Write cô lập PID, ASan/UBSan.<br>- Quản lý hợp nhất đa nhánh và chuẩn hóa hồ sơ tài liệu. | [`phat`](https://github.com/phattvh/DataStructure_ATM-Project/tree/phat) |
| 2 | **Huỳnh Minh Trí** | **Data Engineer / Memory & Storage Flow**<br>- Cài đặt Generic Template `LinkedList<T>` chuẩn $\mathcal{O}(1)$ thêm cuối.<br>- Xây dựng tầng tệp `FileService` đọc/ghi 5 file vật lý và Auto-Recovery.<br>- Model `Admin`, `Transaction`.<br>- Bộ kiểm định rò rỉ bộ nhớ Memory Audit (67 tests, 0 bytes leaked) và script Valgrind. | [`tri`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tri) |
| 3 | **Hứa Nhựt Tuấn** | **Business Logic / User Flow & UI**<br>- Tầng hiển thị `ConsoleView`, Model `Card` và `Account`.<br>- Toàn bộ logic nghiệp vụ Phân hệ Khách hàng (`UserController`).<br>- Giao dịch tài chính đĩa: Rút tiền, Chuyển tiền nguyên tử ACID, Đổi PIN, Lịch sử.<br>- Soạn thảo Báo cáo Word Đồ án và Kịch bản Demo. | [`tuan`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tuan) |

---

## ĐẶC ĐIỂM NỔI BẬT VÀ TIÊU CHUẨN KỸ THUẬT

1. **Cấu trúc dữ liệu Generic Template (`LinkedList<T>`)**:
   - Tự cài đặt danh sách liên kết đơn độc lập với con trỏ đuôi `_pTail` (không sử dụng thư viện STL như `std::vector` hay `std::list`).
   - Thao tác thêm cuối (`addTail`) đạt độ phức tạp tối ưu $\mathcal{O}(1)$.
   - Quản lý bộ nhớ: Thu hồi 100% node trong Destructor, vô hiệu hóa Copy Semantics (`= delete`) để loại bỏ hoàn toàn lỗi `Double Free`.
2. **Tuân thủ chuẩn C++ Coding Standard Version 2**:
   - Quy chuẩn đặt tên theo **Hungarian Notation** (`strId`, `iCount`, `lBalance`, `bIsLocked`, `pNode`).
   - Biến thành viên class mang tiền tố `_` (`_strId`, `_lBalance`).
   - Tách bạch 100% giữa tệp giao diện khai báo `.h` và tệp hiện thực `.cpp`.
   - Sử dụng tường minh con trỏ `this->` (Rule 17) khi truy xuất thành viên nội bộ.
   - Định dạng chú thích hàm theo chuẩn `@Description`, `@return`, `@attention`.
3. **Bảo mật và Mật mã học (Security & Cryptography)**:
   - Thuật toán băm **Salted-MD5 (RFC 1321)** thuần C++ không phụ thuộc thư viện ngoài (`SecurityService`).
   - Khóa thẻ tự động và **lưu bền vững số lần nhập sai PIN** vào `data/FailedAttempts.txt` qua các lần tắt/mở ứng dụng.
   - Cơ chế RAII `LinuxTerminalRawGuard` quản lý chế độ raw mode terminal.
   - Mã hóa thời gian thực mật khẩu Admin và mã PIN User thành ký tự `*` (hỗ trợ phím Backspace, drain chuỗi escape).
   - Giới hạn độ dài nhập liệu tối đa 32 ký tự, ngăn chặn tấn công tràn bộ đệm console DoS.
   - Định dạng giao diện trực quan bằng mã màu ANSI (Xanh lá: thành công, Đỏ: lỗi, Vàng: cảnh báo, Cyan: thông tin).
   - Bẫy lỗi ngoại lệ `cin.fail()` và tín hiệu `EOF` (`Ctrl+D`) chống sập chương trình hoặc lặp vô hạn.
4. **Hệ thống Lưu trữ Tệp tin (Flat-file Storage) và Giao dịch ACID**:
   - Quản lý và đồng bộ dữ liệu qua các tệp: `Admin.txt`, `TheTu.txt`, `KhoaThe.txt`, `FailedAttempts.txt`, `[ID].txt`, `LichSu[ID].txt`.
   - **Khóa độc quyền liên tiến trình (`flock`)**: Ngăn chặn xung đột ghi đè dữ liệu khi nhiều tiến trình/máy ATM cùng chạy.
   - Cơ chế **Ghi tệp nguyên tử (Atomic Write)** qua tệp tạm `.tmp.<PID>` và `rename`, ngăn mất mát dữ liệu khi mất nguồn hoặc crash.
   - Cơ chế **Hoàn tiền nguyên tử (Atomic Rollback)**: Tự động hoàn tiền cho người gửi nếu tài khoản nhận gặp lỗi I/O.
   - Tự động lưu trữ (Archive `.bak`) tệp lịch sử khi tái tạo thẻ cũ nhằm bảo vệ quyền riêng tư và kiểm toán tài chính.
5. **Động cơ Đa Tiền tệ & Quy đổi Ngoại tệ (Multi-Currency & FX Engine)**:
   - Hỗ trợ đầy đủ các loại tiền tệ: `VND`, `USD`, `EUR`, `JPY`, `GBP`, `SGD` với hạn mức min/max và số dư duy trì riêng biệt.
   - Quy tắc quy đổi rút tiền ngoại tệ nhận tiền mặt VND sử dụng phép chia trần (Ceiling Division), in biên lai minh bạch phần chênh lệch làm tròn.
   - Chặn tuyệt đối chuyển tiền chéo loại tiền tệ nhằm bảo toàn giá trị tài chính.
6. **Kiểm định Bộ nhớ và Kiểm thử Tự động (100% Passed)**:
   - 147/147 kịch bản kiểm thử tự động toàn hệ thống đạt kết quả PASS (100%).
   - 67/67 kịch bản kiểm thử bộ nhớ đạt chuẩn 0 byte leak.
   - Kiểm thử tải cao 50.000 nodes qua 3 chu kỳ (150.360 lần `new`/`delete`), ghi nhận **0 bytes leaked, 0 double free**.

---

## DỮ LIỆU TÀI KHOẢN MẪU (QUICK START)

Dữ liệu mẫu được lưu trữ trong thư mục `data/` phục vụ kiểm thử:

### 1. Phân hệ Quản trị viên (Admin)
- **Tài khoản**: `admin1` | **Mật khẩu**: `123456`
- **Tài khoản**: `superadmin` | **Mật khẩu**: `888888`

### 2. Phân hệ Khách hàng (User / Thẻ ATM)
| Số thẻ (14 chữ số) | Mã PIN | Chủ tài khoản | Số dư ban đầu | Trạng thái / Mục đích kiểm thử |
| :--- | :---: | :--- | :---: | :--- |
| **`10014504500003`** | `654321` | Lê Văn Cường | 750.000 VND | Đã đổi PIN, truy cập thẳng Menu chính |
| **`10014504500005`** | `888888` | Nguyễn Văn E | 3.000.000 VND | Hoạt động bình thường |
| **`10014504500001`** | `123456` | Nguyen Trung Kien | 5.000.000 VND | PIN mặc định: Yêu cầu đổi PIN trước khi vào Menu |
| **`10014504500002`** | `123456` | Tran Thi Binh | 1.200.000 VND | Sử dụng nhận tiền khi kiểm thử chuyển khoản |

---

## HƯỚNG DẪN BIÊN DỊCH VÀ CHẠY ỨNG DỤNG

Chương trình được phát triển bằng chuẩn **ISO C++17 thuần túy**, **không phụ thuộc thư viện bên ngoài (Zero External Dependencies)**. Dưới đây là các phương án thực thi phù hợp cho từng môi trường:

---

### Phương án 1: Sử dụng Script tự động (Khuyên dùng)
Script tự động phát hiện môi trường, tự biên dịch và khởi chạy ứng dụng:

#### Trên Linux / macOS / WSL:
```bash
./run.sh
```
*(Nếu cần cấp quyền thực thi: `chmod +x run.sh && ./run.sh`)*  
*Script tự động kiểm tra công cụ `make`. Nếu hệ thống không cài `make`, script tự động chuyển sang gọi trực tiếp `g++` để biên dịch và chạy file `build/atm_app`.*

#### Trên Windows:
- **Thao tác nhanh**: Click đúp chuột vào tệp **`run.bat`** trong thư mục dự án.
- **Hoặc chạy từ dòng lệnh (CMD / PowerShell)**:
  ```cmd
  run.bat
  ```
*(Tự động tạo thư mục `build`, biên dịch mã nguồn qua `g++` thành `build\atm_app.exe` và khởi chạy ứng dụng mà không cần cài đặt `make`).*

---

### Phương án 2: Biên dịch trực tiếp bằng g++ (Không cần make)
Áp dụng cho các hệ thống chỉ cài đặt trình biên dịch `g++` độc lập (MinGW trên Windows hoặc GCC trên Linux):

#### Trên Linux / macOS / WSL:
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o atm_app
./atm_app
```

#### Trên Windows (Command Prompt hoặc PowerShell có g++ MinGW):
```cmd
g++ -std=c++17 -Wall -Wextra -Iinclude src\*.cpp -o atm_app.exe
atm_app.exe
```

*Ý nghĩa các tham số biên dịch:*
- `-std=c++17`: Bật chuẩn C++17 (yêu cầu cho `inline`, `std::filesystem`).
- `-Iinclude`: Khai báo đường dẫn thư mục header.
- `src/*.cpp`: Tập hợp các tệp mã nguồn hiện thực.
- `-Wall -Wextra`: Bật kiểm tra cảnh báo biên dịch nghiêm ngặt.

---

### Phương án 3: Sử dụng Makefile (Hệ thống có sẵn make)

```bash
# 1. Biên dịch ứng dụng chính (tạo tệp build/atm_app)
make app

# 2. Khởi chạy ứng dụng
./build/atm_app

# 3. Chạy toàn bộ các bộ kiểm thử tự động
make test

# 4. Chạy kiểm định rò rỉ bộ nhớ (Memory Audit - 67 test cases)
make test_mem

# 5. Chạy kiểm thử nghiệp vụ Khách hàng (Member A - 51 test cases)
make test_phase_3_a

# 6. Kiểm tra an toàn bộ nhớ với AddressSanitizer và UndefinedBehaviorSanitizer
make test_asan

# 7. Chạy kiểm tra rò rỉ bộ nhớ với Valgrind (trên Linux)
bash scripts/valgrind_check.sh

# 8. Dọn dẹp tệp tin đối tượng (.o) và tệp thực thi trong thư mục build/
make clean
```

---

### Phương án 4: Sử dụng tệp thực thi biên dịch sẵn
Dự án cung cấp tệp nhị phân đã biên dịch sẵn trong thư mục gốc:
```bash
./atm_project.exe
# Hoặc:
./build/atm_app
```

---

### Khôi phục dữ liệu ban đầu sau kiểm thử
Trong quá trình kiểm thử, các thao tác tài chính sẽ ghi trực tiếp vào các tệp trong thư mục `data/`. Để khôi phục toàn bộ dữ liệu mẫu về trạng thái gốc:
```bash
git restore data/
git clean -fd data/
```

---

## CẤU TRÚC THƯ MỤC DỰ ÁN

```text
DataStructure_ATM-Project/
├── Makefile                          # Script biên dịch tự động (g++ -std=c++17 -Wall -Wextra)
├── run.sh                            # Script tự động hóa cho Linux / macOS (hỗ trợ có hoặc không có make)
├── run.bat                           # Script tự động hóa cho Windows (thực thi trực tiếp qua g++)
├── atm_project.exe                   # Tệp thực thi nhị phân biên dịch sẵn
├── README.md                         # Tài liệu giới thiệu tổng quan dự án
├── CHANGELOG.md                      # Nhật ký phiên bản và phân công đóng góp
├── Project/                          # Tài liệu đề bài và biểu mẫu đồ án từ nhà trường
│   ├── Đề Bài.pdf
│   ├── Yêu cầu thực hiện Project.pdf
│   ├── Tài Liệu Đọc.pdf             # Quy chuẩn C++ Coding Standard V2
│   └── Mau_BaoCao_DoAn.docx
├── data/                             # Cơ sở dữ liệu tệp tin (.txt)
│   ├── Admin.txt                     # Danh sách tài khoản quản trị
│   ├── TheTu.txt                     # Danh sách thẻ từ (ID 14 số và mã PIN)
│   ├── KhoaThe.txt                   # Danh sách ID thẻ bị khóa
│   ├── [ID].txt                      # Tệp chi tiết từng tài khoản (ID, Tên, Số dư, Tiền tệ)
│   └── LichSu[ID].txt                # Nhật ký biến động số dư theo thời gian thực
├── docs/                             # Hệ thống tài liệu kỹ thuật chi tiết
│   ├── TIEN_DO_CONG_VIEC.md          # Bảng theo dõi tiến độ công việc (Task Tracker)
│   ├── DemoScript_User.md            # Kịch bản demo phân hệ User (Member A)
│   ├── 01_tieu_chi_cham.md           # Thang điểm chi tiết và các bẫy kỹ thuật C++
│   ├── 02_yeu_cau_va_pham_vi.md      # Phạm vi nghiệp vụ và đặc tả yêu cầu
│   ├── 03_kien_truc_he_thong.md      # Kiến trúc phân tầng Layered Architecture
│   ├── 04_ctdl_va_thuat_toan.md      # Thiết kế Template LinkedList và phân tích Big-O
│   ├── 05_thiet_ke_chi_tiet.md       # Thiết kế chi tiết từng Class và Method
│   ├── 06_ke_hoach_trien_khai.md     # Phân công công việc và lộ trình 14 ngày
│   ├── 07_ke_hoach_kiem_thu.md       # Kế hoạch kiểm thử và ma trận test cases
│   └── 08_so_do_uml_va_luong_du_lieu.md # Sơ đồ UML Class, DFD và Sequence Diagram
├── include/                          # Tệp tin Header (*.h)
│   ├── Common.h                      # Hằng số, Enum, ErrorCode dùng chung
│   ├── LinkedList.h                  # Template Class Cấu trúc dữ liệu tự tạo
│   ├── Admin.h                       # Model Admin
│   ├── Card.h                        # Model Thẻ từ
│   ├── Account.h                     # Model Tài khoản
│   ├── Transaction.h                 # Model Lịch sử giao dịch
│   ├── FileService.h                 # Dịch vụ thao tác tệp tin vật lý
│   ├── ConsoleView.h                 # Tiện ích giao diện Console và ANSI UX
│   ├── AdminController.h             # Bộ điều phối Phân hệ Quản trị Admin
│   ├── UserController.h              # Bộ điều phối Phân hệ Khách hàng User
│   └── AtmController.h               # Bộ điều phối trung tâm ứng dụng
├── src/                              # Tệp tin Hiện thực (*.cpp)
│   ├── Admin.cpp
│   ├── Card.cpp
│   ├── Account.cpp
│   ├── Transaction.cpp
│   ├── FileService.cpp
│   ├── ConsoleView.cpp
│   ├── AdminController.cpp
│   ├── UserController.cpp
│   ├── AtmController.cpp
│   └── main.cpp                      # Điểm vào chính của chương trình
├── test/                             # Toàn bộ mã nguồn kiểm thử tự động
│   ├── test.cpp                      # Test suite tổng hợp
│   ├── test_member_a.cpp             # Test Unit Member A (Phase 1)
│   ├── test_member_c.cpp             # Test Unit Member C (Phase 1)
│   ├── test_phase_1_AC.cpp           # Test tích hợp Member A và C (Phase 1)
│   ├── test_phase_2_a.cpp            # Test Member A (Phase 2)
│   ├── test_phase_2_b.cpp            # Test Member B (Phase 2)
│   ├── test_phase_2_c.cpp            # Test Member C (Phase 2)
│   ├── test_phase_3.cpp              # Test tích hợp toàn trình (Phase 3)
│   ├── test_phase_3_a.cpp            # Test nghiệp vụ User và Giao dịch (Phase 3 Member A)
│   └── test_memory_leak.cpp          # Kiểm định rò rỉ bộ nhớ (Phase 4 Member B)
└── scripts/                          # Kịch bản tự động hóa
    └── valgrind_check.sh             # Script kiểm tra Valgrind Memcheck
```

---

## HỆ THỐNG TÀI LIỆU KỸ THUẬT

Tài liệu chi tiết cho từng nội dung phát triển được lưu trữ tại thư mục [`docs/`](docs/):

- [**01. Tiêu chí chấm điểm & Bẫy kỹ thuật**](docs/01_tieu_chi_cham.md): Phân bổ điểm số (10.0đ), các bẫy `std::getline` khoảng trắng, `cin.fail()`, thời gian thực và quản lý bộ nhớ.
- [**02. Yêu cầu & Phạm vi dự án**](docs/02_yeu_cau_va_pham_vi.md): Quy định chức năng Admin, chức năng User, các ràng buộc giao dịch ($\ge 50.000$ VNĐ, bội số của $50.000$ VNĐ, số dư duy trì tối thiểu $50.000$ VNĐ).
- [**03. Kiến trúc hệ thống**](docs/03_kien_truc_he_thong.md): Sơ đồ phân tầng 5 lớp (Presentation, Controller, Domain, Service, Foundation), cơ chế cách ly phiên (Session Isolation) và tự phục hồi dữ liệu (Auto-Recovery).
- [**04. Cấu trúc dữ liệu & Thuật toán**](docs/04_ctdl_va_thuat_toan.md): Cài đặt Template `LinkedList<T>`, quy tắc Rule 17 (New/Delete), bảng độ phức tạp Big-O và so sánh với Mảng tĩnh / Dynamic Array.
- [**05. Thiết kế chi tiết**](docs/05_thiet_ke_chi_tiet.md): Đặc tả chi tiết các thuộc tính, phương thức, giá trị trả về, kiểm soát lỗi cho toàn bộ file mã nguồn.
- [**06. Kế hoạch triển khai & Phân công**](docs/06_ke_hoach_trien_khai.md): Lộ trình 14 ngày (4 Phase), ma trận phân công RACI, quy chuẩn đặt tên nhánh và commit message.
- [**07. Kế hoạch kiểm thử & QA**](docs/07_ke_hoach_kiem_thu.md): Ma trận kịch bản Test Cases (Admin, User, Giao dịch), kiểm thử tấn công input và lệnh kiểm tra rò rỉ bộ nhớ với Valgrind.
- [**08. Sơ đồ UML & Luồng dữ liệu**](docs/08_so_do_uml_va_luong_du_lieu.md): Sơ đồ UML Lớp hoàn chỉnh, Sơ đồ luồng dữ liệu kiến trúc (DFD) và Biểu đồ tuần tự (Sequence Diagram) luồng chuyển tiền 2 đầu.
- [**Nhật ký thay đổi dự án (CHANGELOG.md)**](CHANGELOG.md): Toàn bộ lịch sử nâng cấp và đóng góp của cả 3 thành viên qua từng phiên bản.
- [**Bảng theo dõi tiến độ công việc (Task Tracker)**](docs/TIEN_DO_CONG_VIEC.md): Bảng Dashboard tiến độ 40 nhiệm vụ của nhóm (đạt 95% hoàn thành).

---

## QUY TRÌNH LÀM VIỆC VỚI GIT

### 1. Cấu trúc nhánh
- `main`: Nhánh phát hành chính thức (Release), mã nguồn ổn định đã qua nghiệm thu.
- `fix`: Nhánh tích hợp và kiểm thử liên phân hệ (Integration & QA).
- `phat`: Nhánh phát triển của Thành viên Phát (Member C - Tech Lead).
- `tri`: Nhánh phát triển của Thành viên Trí (Member B - Data Engineer).
- `tuan`: Nhánh phát triển của Thành viên Tuấn (Member A - Business & UI).

### 2. Tiêu chuẩn viết Commit Message
Nội dung commit sử dụng tiếng Anh theo chuẩn Conventional Commits:
- `feat:` Bổ sung tính năng mới.
- `fix:` Sửa lỗi logic hoặc xử lý ngoại lệ.
- `docs:` Cập nhật tài liệu kỹ thuật.
- `style:` Căn chỉnh định dạng mã nguồn theo chuẩn Hungarian Notation.
- `refactor:` Tái cấu trúc mã nguồn mà không làm thay đổi hành vi.
- `test:` Bổ sung hoặc cập nhật kịch bản kiểm thử tự động.

---

## BẢN QUYỀN

Dự án được thực hiện phục vụ mục đích học tập và nghiên cứu trong khuôn khổ môn học Cấu trúc Dữ liệu & Giải thuật tại Trường Đại học Sư phạm TP. Hồ Chí Minh (HCMUE).
