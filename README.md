# ATM Simulation Project (Hệ thống Mô phỏng Máy ATM Ngân hàng)

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![Course](https://img.shields.io/badge/Course-Data%20Structures%20%26%20Algorithms-green.svg)](https://cntt.hcmue.edu.vn/)
[![Institution](https://img.shields.io/badge/University-HCMUE-red.svg)](https://hcmue.edu.vn/)
[![Standard](https://img.shields.io/badge/Coding%20Standard-C%2B%2B%20Standard%20V2-orange.svg)](docs/01_tieu_chi_cham.md)
[![Memory Safety](<https://img.shields.io/badge/Memory%20Leak-0%20bytes%20(100%25%20Clean)-brightgreen.svg>)](test/test_memory_leak.cpp)
[![Test Suite](https://img.shields.io/badge/Tests-100%25%20Passed-success.svg)](test/)

Đồ án môn học **Cấu trúc Dữ liệu & Giải thuật** — Khoa Công nghệ Thông tin, Trường Đại học Sư phạm TP. Hồ Chí Minh (HCMUE).  
Dự án tập trung vào việc áp dụng mô hình **Lập trình Hướng đối tượng (OOP)** và **Cấu trúc dữ liệu Generic Template** tự xây dựng để mô phỏng hoạt động thực tế của một hệ thống máy ATM ngân hàng chuẩn công nghiệp.

---

## 👥 THÀNH VIÊN NHÓM THỰC HIỆN

| STT | Họ và Tên            | Vai trò & Trách nhiệm chính                                                                                                                                                                                                                                                                                                     |                                Nhánh Git                                 |
| :-: | :------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | :----------------------------------------------------------------------: |
|  1  | **Trần Vũ Hỏa Phát** | **Tech Lead / Core Controller & Admin Flow**<br>- Thiết kế kiến trúc 5 tầng, `Common.h`, Makefile.<br>- Xây dựng bộ điều phối trung tâm `AtmController`, `AdminController`.<br>- Bảo mật Terminal Raw Mode, Atomic Write cô lập PID, ASan/UBSan.<br>- Quản lý hợp nhất đa nhánh & chuẩn hóa hồ sơ tài liệu.                     | [`phat`](https://github.com/phattvh/DataStructure_ATM-Project/tree/phat) |
|  2  | **Huỳnh Minh Trí**   | **Data Engineer / Memory & Storage Flow**<br>- Tự cài đặt Generic Template `LinkedList<T>` chuẩn $\mathcal{O}(1)$ thêm cuối.<br>- Xây dựng tầng tệp `FileService` đọc/ghi 5 file vật lý và Auto-Recovery.<br>- Model `Admin`, `Transaction`.<br>- Bộ kiểm định rò rỉ bộ nhớ Memory Audit (67 tests, 0 bytes leaked) & Valgrind. |  [`tri`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tri)  |
|  3  | **Hứa Nhựt Tuấn**    | **Business Logic / User Flow & UI**<br>- Tầng hiển thị `ConsoleView`, Model `Card` và `Account`.<br>- Toàn bộ logic nghiệp vụ Phân hệ Khách hàng (`UserController`).<br>- Giao dịch tài chính đĩa: Rút tiền, Chuyển tiền nguyên tử ACID, Đổi PIN, Lịch sử.<br>- Soạn thảo Báo cáo Word Đồ án và Kịch bản Demo.                  | [`tuan`](https://github.com/phattvh/DataStructure_ATM-Project/tree/tuan) |

---

## 🌟 ĐẶC ĐIỂM NỔI BẬT & TIÊU CHUẨN KỸ THUẬT

1. **Cấu trúc dữ liệu Generic Template (`LinkedList<T>`)**:
   - Tự cài đặt danh sách liên kết đơn độc lập với con trỏ đuôi `_pTail` (không sử dụng `std::vector` hay `std::list` của STL).
   - Thao tác thêm cuối (`addTail`) đạt độ phức tạp tối ưu $\mathcal{O}(1)$.
   - Quản lý bộ nhớ nghiêm ngặt: Thu hồi 100% node trong Destructor, vô hiệu hóa Copy Semantics (`= delete`) để loại bỏ hoàn toàn lỗi `Double Free`.
2. **Tuân thủ chuẩn C++ Coding Standard Version 2**:
   - Quy chuẩn đặt tên theo **Hungarian Notation** (`strId`, `iCount`, `lBalance`, `bIsLocked`, `pNode`).
   - Biến thành viên class mang tiền tố `_` (`_strId`, `_lBalance`).
   - Tách bạch 100% giữa tệp giao diện khai báo `.h` và tệp hiện thực `.cpp`.
   - Sử dụng tường minh con trỏ `this->` (Rule 17) khi truy xuất thành viên nội bộ.
   - Định dạng chú thích hàm chuẩn `@Description`, `@return`, `@attention`.
3. **Bảo mật & Trải nghiệm Console (CLI UX)**:
   - Cơ chế RAII `LinuxTerminalRawGuard` quản lý raw mode terminal an toàn.
   - Mã hóa thời gian thực mật khẩu Admin và mã PIN User thành ký tự `*` (hỗ trợ xóa lùi Backspace, drain sạch escape sequence).
   - Giới hạn độ dài nhập liệu (tối đa 32 ký tự) chống tấn công tràn bộ đệm console DoS.
   - Giao diện trực quan với mã màu chuẩn **ANSI Color** (Xanh lá: thành công, Đỏ: lỗi, Vàng: cảnh báo, Cyan: bảng thông tin).
   - Bẫy lỗi ngoại lệ `cin.fail()` và tín hiệu `EOF` (`Ctrl+D`) chống sập ứng dụng và treo CPU 100%.
4. **Hệ thống Lưu trữ Tệp tin Bền vững (Flat-file Storage) & ACID**:
   - Tự động quản lý và đồng bộ 5 loại tệp: `Admin.txt`, `TheTu.txt`, `KhoaThe.txt`, `[ID].txt`, `LichSu[ID].txt`.
   - Cơ chế **Ghi tệp nguyên tử (Atomic Write)** qua tệp tạm `.tmp.<PID>` và `rename`, ngăn mất mát dữ liệu khi mất điện hoặc xung đột tiến trình.
   - Cơ chế **Hoàn tiền nguyên tử (Atomic Rollback)**: Tự động hoàn tiền cho người gửi nếu tài khoản nhận gặp lỗi I/O.
   - Tự động lưu trữ (Archive `.bak`) tệp lịch sử khi tái tạo thẻ cũ để bảo vệ quyền riêng tư.
5. **Kiểm định Rò rỉ Bộ nhớ Hoàn hảo (0 Bytes Leaked)**:
   - 67/67 kịch bản kiểm thử bộ nhớ đạt PASS 100%.
   - Stress testing 50.000 nodes $\times$ 3 chu kỳ (150.306 lần `new`/`delete`) ghi nhận **0 bytes leaked, 0 double free**.

---

## 🔑 TÀI KHOẢN KIỂM THỬ NHANH (QUICK START CREDENTIALS)

Dữ liệu mẫu đã được tích hợp sẵn trong thư mục `data/` phục vụ kiểm thử ngay lập tức:

### 1. Phân hệ Quản trị viên (Admin)

- **Tài khoản**: `admin1` | **Mật khẩu**: `123456`
- **Tài khoản**: `superadmin` | **Mật khẩu**: `888888`

### 2. Phân hệ Khách hàng (User / Thẻ ATM)

| Số thẻ (14 chữ số)   |  Mã PIN  | Chủ tài khoản     | Số dư ban đầu | Trạng thái / Mục đích test                            |
| :------------------- | :------: | :---------------- | :-----------: | :---------------------------------------------------- |
| **`10014504500003`** | `654321` | Lê Văn Cường      |  750.000 VND  | Đã đổi PIN, **vào thẳng Menu chính** để test ngay     |
| **`10014504500005`** | `888888` | Nguyễn Văn E      | 3.000.000 VND | Đang hoạt động bình thường                            |
| **`10014504500001`** | `123456` | Nguyen Trung Kien | 5.000.000 VND | PIN mặc định: **Hệ thống sẽ ép đổi mã PIN mới trước** |
| **`10014504500002`** | `123456` | Tran Thi Binh     | 1.200.000 VND | Dùng để nhận tiền test chuyển khoản                   |

---

## 🚀 HƯỚNG DẪN BIÊN DỊCH & CHẠY ỨNG DỤNG CHI TIẾT

Dự án được thiết kế hoàn toàn bằng chuẩn **ISO C++17 thuần túy**, **không phụ thuộc bất kỳ thư viện bên ngoài nào (Zero External Dependencies)**. Nhóm cung cấp 4 phương án chạy linh hoạt phù hợp với mọi cấu hình máy tính của Giảng viên và người chấm bài:

---

### 🟢 PHƯƠNG ÁN 1: Chạy nhanh nhất bằng Script 1-Click (Khuyên dùng cho Giảng viên)
> Không yêu cầu cấu hình phức tạp, script tự động nhận diện hệ điều hành và tự biên dịch ứng dụng.

#### 1. Trên Linux / macOS / WSL:
Chỉ cần mở Terminal tại thư mục dự án và chạy:
```bash
./run.sh
```
*(Nếu chưa cấp quyền thực thi: `chmod +x run.sh && ./run.sh`)*  
*Script sẽ tự kiểm tra: Nếu máy có `make` sẽ dùng `make`, nếu máy **không có `make`** sẽ tự động chuyển sang gọi trực tiếp `g++` để biên dịch ra `build/atm_app` và khởi chạy ngay.*

#### 2. Trên Windows:
- **Cách 1 (Đơn giản nhất)**: **Click đúp chuột (Double Click)** trực tiếp vào file **`run.bat`** trong thư mục dự án.
- **Cách 2**: Mở Command Prompt (CMD) hoặc PowerShell tại thư mục dự án và gõ:
  ```cmd
  run.bat
  ```
*(Tự động tạo thư mục `build`, biên dịch mã nguồn qua `g++` thành `build\atm_app.exe` và mở cửa sổ ATM để kiểm thử ngay mà **tuyệt đối không cần cài đặt `make`**).*

---

### 🟡 PHƯƠNG ÁN 2: Biên dịch trực tiếp bằng lệnh `g++` (Khi máy Thầy KHÔNG CÓ `make`)
> Dành cho trường hợp máy tính của Thầy chỉ cài sẵn MinGW / GCC `g++` mà chưa cài tiện ích `make`.

#### 1. Trên Linux / macOS / WSL:
Chạy đúng 1 dòng lệnh duy nhất để biên dịch toàn bộ mã nguồn:
```bash
g++ -std=c++17 -Wall -Wextra -Iinclude src/*.cpp -o atm_app
./atm_app
```

#### 2. Trên Windows (CMD hoặc PowerShell có g++ MinGW):
```cmd
g++ -std=c++17 -Wall -Wextra -Iinclude src\*.cpp -o atm_app.exe
atm_app.exe
```

*Giải thích các cờ lệnh:*
* `-std=c++17`: Kích hoạt chuẩn C++17 bắt buộc (cho `inline` constants, `<filesystem>`, `auto`).
* `-Iinclude`: Khai báo đường dẫn chứa các file header (`.h`).
* `src/*.cpp`: Biên dịch toàn bộ các file hiện thực lớp.
* `-Wall -Wextra`: Bật toàn bộ các cảnh báo biên dịch nghiêm ngặt (mã nguồn đạt chuẩn 0 warnings).

---

### 🔵 PHƯƠNG ÁN 3: Sử dụng `Makefile` (Môi trường Linux / WSL / MSYS2 có sẵn `make`)

```bash
# 1. Biên dịch ứng dụng ATM chính thức (file thực thi tạo ra tại build/atm_app)
make app

# 2. Khởi chạy ứng dụng máy ATM vừa biên dịch
./build/atm_app

# 3. Chạy toàn bộ 100% tất cả các bộ kiểm thử tự động (51 test User, 58 test Phase 3, 67 test Mem)
make test

# 4. Chạy riêng bài kiểm định rò rỉ bộ nhớ (Memory Audit - 67 test cases, 0 bytes leaked)
make test_mem

# 5. Chạy riêng bài kiểm thử nghiệp vụ Khách hàng & Giao dịch tài chính Member A (51 test cases)
make test_phase_3_a

# 6. Kiểm tra an toàn bộ nhớ với AddressSanitizer (ASan) & UndefinedBehaviorSanitizer (UBSan)
make test_asan

# 7. Chạy kiểm tra rò rỉ bộ nhớ chuyên sâu với Valgrind (trên Linux)
bash scripts/valgrind_check.sh

# 8. Dọn dẹp sạch sẽ toàn bộ tệp đối tượng (.o) và tệp thực thi trong thư mục build/
make clean
```

---

### 🟣 PHƯƠNG ÁN 4: Chạy file thực thi đã biên dịch sẵn (Pre-compiled Binaries)
Dự án đã tích hợp sẵn file nhị phân được biên dịch từ phiên bản mới nhất, có thể chạy thử nghiệm ngay:
```bash
./atm_project.exe
# Hoặc:
./build/atm_app
```

---

### 🔄 Hướng dẫn Khôi phục Dữ liệu Mẫu sau khi Kiểm thử
Trong quá trình test các tính năng như Rút tiền, Chuyển tiền, Khóa thẻ hay Đổi PIN, hệ thống sẽ ghi thay đổi trực tiếp vào các file trong thư mục `data/`. Để đưa toàn bộ dữ liệu mẫu về trạng thái ban đầu sạch sẽ, bạn chỉ cần gõ 2 lệnh sau:
```bash
git restore data/
git clean -fd data/
```

---

## 📂 CẤU TRÚC THƯ MỤC DỰ ÁN

```text
DataStructure_ATM-Project/
├── Makefile                          # Script biên dịch tự động (g++ -std=c++17 -Wall -Wextra)
├── run.sh                            # Script 1-Click tự động hóa cho Linux / macOS (có hoặc không có make)
├── run.bat                           # Script 1-Click tự động hóa cho Windows (click đúp là chạy ngay)
├── atm_project.exe                   # File thực thi nhị phân đã biên dịch sẵn
├── README.md                         # Tài liệu giới thiệu tổng quan dự án
├── CHANGELOG.md                      # Nhật ký chi tiết toàn bộ các phiên bản và đóng góp
├── Project/                          # Đề bài và biểu mẫu đồ án gốc từ giảng viên
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
│   ├── 01_tieu_chi_cham.md           # Thang điểm chi tiết & 4 bẫy kỹ thuật C++
│   ├── 02_yeu_cau_va_pham_vi.md      # Phạm vi nghiệp vụ & Đặc tả yêu cầu
│   ├── 03_kien_truc_he_thong.md      # Kiến trúc phân tầng Layered Architecture
│   ├── 04_ctdl_va_thuat_toan.md      # Thiết kế Template LinkedList & Phân tích Big-O
│   ├── 05_thiet_ke_chi_tiet.md       # Thiết kế chi tiết từng Class & Method
│   ├── 06_ke_hoach_trien_khai.md     # Phân công công việc & Lộ trình 14 ngày
│   ├── 07_ke_hoach_kiem_thu.md       # Kế hoạch kiểm thử & Ma trận test cases
│   └── 08_so_do_uml_va_luong_du_lieu.md # Sơ đồ UML Class, DFD và Sequence Diagram
├── include/                          # Tệp tin Header (*.h)
│   ├── Common.h                      # Hằng số, Enum, ErrorCode dùng chung
│   ├── LinkedList.h                  # Template Class Cấu trúc dữ liệu tự tạo
│   ├── Admin.h                       # Model Admin
│   ├── Card.h                        # Model Thẻ từ
│   ├── Account.h                     # Model Tài khoản
│   ├── Transaction.h                 # Model Lịch sử giao dịch
│   ├── FileService.h                 # Dịch vụ thao tác tệp tin vật lý
│   ├── ConsoleView.h                 # Tiện ích giao diện Console & ANSI UX
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
│   ├── test_phase_1_AC.cpp           # Test tích hợp Member A & C (Phase 1)
│   ├── test_phase_2_a.cpp            # Test Member A (Phase 2)
│   ├── test_phase_2_b.cpp            # Test Member B (Phase 2)
│   ├── test_phase_2_c.cpp            # Test Member C (Phase 2)
│   ├── test_phase_3.cpp              # Test tích hợp toàn trình (Phase 3)
│   ├── test_phase_3_a.cpp            # Test nghiệp vụ User & Giao dịch (Phase 3 Member A)
│   └── test_memory_leak.cpp          # Kiểm định rò rỉ bộ nhớ (Phase 4 Member B)
└── scripts/                          # Kịch bản tự động hóa
    └── valgrind_check.sh             # Script kiểm tra Valgrind Memcheck
```

---

## 📖 HỆ THỐNG TÀI LIỆU KỸ THUẬT (DOCS)

Nhóm đã xây dựng tài liệu chi tiết cho từng giai đoạn phát triển tại thư mục [`docs/`](docs/):

- [**01. Tiêu chí chấm điểm & Bẫy kỹ thuật**](docs/01_tieu_chi_cham.md): Phân bổ điểm số (10.0đ), các bẫy `std::getline` khoảng trắng, bẫy `cin.fail()`, thời gian thực và quản lý bộ nhớ.
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

## 🌿 QUY CHUẨN LÀM VIỆC VỚI GIT

### 1. Cấu trúc nhánh

- `main`: Nhánh phát hành chính thức (Release), mã nguồn ổn định tuyệt đối đã qua nghiệm thu.
- `fix`: Nhánh tích hợp & kiểm thử liên phân hệ (Integration & QA).
- `phat`: Nhánh phát triển của Thành viên Phát (Member C - Tech Lead).
- `tri`: Nhánh phát triển của Thành viên Trí (Member B - Data Engineer).
- `tuan`: Nhánh phát triển của Thành viên Tuấn (Member A - Business & UI).

### 2. Tiêu chuẩn viết Commit Message

Nội dung commit sử dụng tiếng Anh chuẩn mực theo chuẩn Conventional Commits:

- `feat:` Bổ sung tính năng mới.
- `fix:` Sửa lỗi logic hoặc xử lý ngoại lệ.
- `docs:` Cập nhật tài liệu kỹ thuật.
- `style:` Căn chỉnh định dạng mã nguồn theo chuẩn Hungarian Notation.
- `refactor:` Tái cấu trúc mã nguồn mà không làm thay đổi hành vi.
- `test:` Bổ sung hoặc cập nhật kịch bản kiểm thử tự động.

---

## 📜 GIẤY PHÉP & BẢN QUYỀN

Dự án được thực hiện phục vụ mục đích học tập và nghiên cứu trong khuôn khổ môn học Cấu trúc Dữ liệu & Giải thuật tại Trường Đại học Sư phạm TP. Hồ Chí Minh (HCMUE).
