# 📋 BÁO CÁO NGHIỆM THU KỸ THUẬT & PHÁT HÀNH HỆ THỐNG ATM
## PHÂN HỆ PHASE 2 — THÀNH VIÊN C: PHÁT (CORE CONTROLLER & SECURITY)
**Dự án:** DataStructure_ATM-Project  
**Trường:** Đại học Sư phạm TP. Hồ Chí Minh (HCMUE) — Khoa Công nghệ Thông tin  
**Học phần:** Cấu trúc Dữ liệu & Giải thuật (C++)  
**Nhánh Git:** `phat`  
**Trạng thái nghiệm thu:** **`APPROVED - READY FOR RELEASE`**

---

## I. TỔNG QUAN CÔNG VIỆC HOÀN THÀNH

Thành viên C (Phát) đã hoàn thành 100% khối lượng công việc được phân công trong Phase 2, đồng thời thực hiện gia cố an ninh, độ bền dữ liệu và hoàn thiện theo chuẩn công nghiệp (Production-Ready):

| Mã CV | Hạng mục thực hiện | Tệp mã nguồn | Trạng thái |
| :---: | :--- | :--- | :---: |
| **C08** | Hàm lấy thời gian thực ISO `YYYY-MM-DD HH:MM:SS` | `include/Common.h` | **DONE** |
| **C09** | Khởi tạo lớp điều phối `AtmController` & Vòng đời phiên | `include/AtmController.h`<br>`src/AtmController.cpp` | **DONE** |
| **C10** | Vòng lặp điều hướng ứng dụng và menu đa cấp | `src/AtmController.cpp` | **DONE** |
| **C11** | Phân hệ Quản trị Admin (Xem, Thêm, Xóa, Mở khóa thẻ) | `src/AtmController.cpp` | **DONE** |
| **C12** | Điểm vào chương trình và Auto-Recovery | `src/main.cpp` | **DONE** |
| **C14** | Gia cố an ninh, Atomic File Write & Tách biệt lịch sử | `src/FileService.cpp`<br>`src/AtmController.cpp` | **DONE** |
| **C15** | Menu số nghiêm ngặt, Phân trang sao kê, Admin Audit Trail | `src/ConsoleView.cpp`<br>`src/UserController.cpp` | **DONE** |

---

## II. KIẾN TRÚC & ĐIỂM SÁNG KỸ THUẬT

### 1. Phân Tầng Hệ Thống (Clean Architecture)
* **Model Layer:** `Account`, `Card`, `Admin`, `Transaction` - Đại diện thực thể, đóng gói logic tự thân.
* **Storage Layer:** `LinkedList<T>` (Generic không dùng STL), `FileService` (Quản lý 5 loại tệp tin).
* **View Layer:** `ConsoleView` - Giao diện dòng lệnh chuyên nghiệp với ANSI Escape Codes, che giấu mật khẩu `*`, Signal Handler bảo vệ Terminal.
* **Controller Layer:** `AtmController` (Bộ điều phối trung tâm) và `UserController` (Nghiệp vụ tài chính khách hàng).

### 2. Các Giải Pháp Gia Cố Tiêu Biểu
1. **Atomic File Write Pattern:** Ghi sổ cái qua file trung gian `.tmp` và gọi `std::filesystem::rename()` nguyên tử, loại bỏ hoàn toàn nguy cơ mất dữ liệu khi sập nguồn hoặc crash đột ngột.
2. **Triệt tiêu Zero-Balance Trap:** Kiểm tra nghiêm ngặt định dạng số dư khi đọc file, từ chối nạp file hỏng thay vì gán về 0 VND làm mất tiền khách hàng.
3. **PIN Durability & Stale Cache Defense:** Cập nhật mã PIN mới xuống đĩa ngay tức thì; chỉ ghi đè danh sách thẻ khi có sự biến động trạng thái.
4. **Anti-Leak History Archive:** Tự động nén và đổi tên sao kê thẻ cũ thành `.bak` khi cấp lại ID cho chủ thẻ mới.
5. **Admin Audit Trail:** Ghi vết tự động mọi hành vi nhạy cảm của Admin (đăng nhập, thêm thẻ, xóa thẻ kèm số dư, mở khóa) vào `data/AdminLog.txt`.
6. **Strict Menu Validation & Non-tty Fallback:** Chặn đứng việc nhập chuỗi rác (`"1abc"`) và tự động tương thích với cả người dùng tương tác thực tế lẫn luồng kiểm thử tự động CI/CD.

---

## III. KẾT QUẢ KIỂM THỬ TOÀN DIỆN

### 1. Bộ Kiểm Thử Chuyên Sâu Member C (`test_phase_2_c`)
* **Tổng số bài test:** **79 / 79 Test Cases**
* **Kết quả:** **`100% PASS`** (0 thất bại)
* Bao quát toàn bộ:
  - Thời gian hệ thống ISO (C08)
  - Khởi tạo và nạp dữ liệu Controller (C09)
  - Xác thực Admin (C11)
  - 4 nghiệp vụ Admin cốt lõi (C11)
  - Quản lý phiên làm việc & RAII (C09)
  - Kiểm thử hồi quy các bản vá bảo mật (Adversarial Hardening)
  - Kiểm thử 5 hạng mục hoàn thiện cuối cùng (Production Polishing)

### 2. Kiểm Thử Hồi Quy Toàn Bộ Dự Án (`make test`)
* `build/test_c`: **PASS**
* `build/test_a`: **PASS**
* `build/test_ac`: **PASS**
* `build/test_runner`: **101/101 PASS**
* `build/test_phase_2`: **65/65 PASS**
* `build/test_phase_2_c`: **79/79 PASS**

### 3. Kiểm Thử An Toàn Bộ Nhớ (AddressSanitizer & UBSan)
* Biên dịch: `g++ -std=c++17 -Wall -Wextra -fsanitize=address,undefined -g ...`
* **Kết quả:** **0 memory leak, 0 heap-buffer-overflow, 0 use-after-free, 0 undefined behavior**.

---

## IV. HƯỚNG DẪN BIÊN DỊCH VÀ VẬN HÀNH

```bash
# 1. Biên dịch toàn bộ ứng dụng chính thức
make clean && make app

# 2. Khởi chạy ứng dụng ATM
./build/atm_app

# 3. Chạy toàn bộ 6 bộ test suite kiểm thử hồi quy
make test
```

### Tài khoản mặc định hệ thống:
* **Quản trị viên (Admin):** `admin1` / Mật khẩu: `123456`
* **Khách hàng mẫu (User):** `10014504500003` / Mã PIN: `654321` (Đã đổi PIN, sẵn sàng giao dịch)
* **Khách hàng mới (User):** `10014504500001` / Mã PIN: `123456` (PIN mặc định, hệ thống sẽ yêu cầu đổi PIN lần đầu)

---
*Báo cáo được lập tự động dựa trên kết quả kiểm thử thực tế trên nhánh `phat`.*
