# 📋 DANH SÁCH CÔNG VIỆC ĐÃ HOÀN THÀNH — MEMBER A (Tuấn)
> Nhánh: `tuan` | Cập nhật: 2026-10-03

Tài liệu này liệt kê **toàn bộ công việc đã được thực hiện** trên nhánh `tuan`, phục vụ việc review, bàn giao và các thành viên khác theo dõi tiến độ.

---

## 🟢 I. CÁC COMMIT ĐÃ ĐẨY LÊN NHÁNH

| # | Hash | Mô tả commit | Ngày thực hiện |
|---|------|-------------|----------------|
| 1 | `3eed0d8` | `feat(models): implement Card and Account models with financial rules` | 02/10/2026 |
| 2 | `a1a1c36` | `feat(ui): implement ConsoleView with ANSI color, masked password, and cin validation` | 02/10/2026 |
| 3 | `f6c8bde` | `feat(user): implement User module business logic and session handler` | 02/10/2026 |
| 4 | `15bd3a8` | `test & docs: add comprehensive unit test suite and demo script for Member A` | 02/10/2026 |

---

## 📁 II. FILE ĐÃ TẠO / CHỈNH SỬA

### 📌 Models — Tầng Domain (`include/` + `src/`)

#### `include/Card.h` & `src/Card.cpp`
- Định nghĩa class `Card` theo Hungarian Notation (`_strId`, `_strPin`, `_bIsLocked`).
- Getter/Setter đầy đủ với `const` correctness.
- Phương thức kiểm tra trạng thái khóa `isLocked()`.

#### `include/Account.h` & `src/Account.cpp`
- Định nghĩa class `Account` với `_lBalance` (long), `_strOwnerName`, `_strCurrency`.
- Hàm kiểm tra ràng buộc tài chính:
  - Rút tiền tối thiểu **50.000 VNĐ**
  - Rút phải là **bội số 50.000 VNĐ**
  - Tài khoản phải **giữ lại tối thiểu 50.000 VNĐ** sau giao dịch

### 🎨 UI/View — Tầng Presentation (`include/` + `src/`)

#### `include/ConsoleView.h` & `src/ConsoleView.cpp`
- Hiển thị màu ANSI theo ngữ cảnh:
  - 🟢 Xanh lá (`\033[32m`) → Thành công
  - 🔴 Đỏ (`\033[31m`) → Lỗi, cảnh báo
  - 🟡 Vàng (`\033[33m`) → Thông báo
  - 🔵 Cyan (`\033[36m`) → Bảng thông tin
- Hàm `inputPassword()`: ẩn ký tự nhập thành `*`, xử lý phím **Backspace**.
- Bẫy lỗi `cin.fail()`: ngăn vòng lặp vô hạn khi user nhập chữ vào trường số tiền.
- Khung viền giao diện dành cho menu Admin và User.

### 🧠 Business Logic — Tầng Controller

#### `include/UserController.h` & `src/UserController.cpp`
Hiện thực toàn bộ luồng nghiệp vụ **Phân hệ User**:

| Chức năng | Mô tả chi tiết |
|-----------|---------------|
| Đăng nhập | Xác thực ID thẻ + mã PIN |
| Khóa thẻ | Khóa tự động sau **3 lần nhập sai PIN**, ghi vào `KhoaThe.txt` |
| Đổi PIN lần đầu | Ép buộc đổi PIN mặc định `123456` ngay khi đăng nhập lần đầu |
| Xem thông tin | Hiển thị tên, số dư, loại tiền tệ |
| Rút tiền | Kiểm tra 3 ràng buộc tài chính (min, bội số, giữ lại) |
| Chuyển tiền | Tra cứu tài khoản nhận, thực hiện giao dịch 2 chiều (nguyên tử) |
| Xem lịch sử | Đọc file `LichSu[ID].txt` và hiển thị log giao dịch |
| Đổi mã PIN | Xác thực PIN cũ trước khi cho phép đổi |
| Đăng xuất | Dọn dẹp session, trở về màn hình chính |

### ✅ Kiểm thử & Tài liệu

#### `test/test_member_a.cpp`
- Bộ Unit Test kiểm tra các ràng buộc tài chính của `Account`.
- Test bẫy lỗi nhập liệu của `ConsoleView`.
- Compile độc lập, không phụ thuộc vào `main.cpp`.

#### `docs/DemoScript_User.md`
- Kịch bản demo step-by-step cho **Phân hệ User**.
- Bao quát tất cả use-case để thuyết minh khi quay video báo cáo.

---

## ⚙️ III. KỸ THUẬT ĐÃ ÁP DỤNG

```
✔ Hungarian Notation: strId, lBalance, bIsLocked, _strPin (thành viên class)
✔ this-> rõ ràng khi truy xuất thành viên nội bộ
✔ Tách header .h / implementation .cpp hoàn toàn
✔ Comment chuẩn @Description / @return / @attention
✔ ANSI Color codes cho CLI UX
✔ Masked password input (dấu * + Backspace)
✔ cin.fail() guard để chống crash khi nhập sai kiểu
✔ Atomic transfer: ghi log đồng thời 2 tài khoản
```

---

## 🔗 IV. PHỤ THUỘC CẦN TỪ THÀNH VIÊN KHÁC

> Các phần dưới đây cần được Phát/Trí hoàn thành để tích hợp:

| Cần từ | Nội dung cần | Trạng thái |
|--------|-------------|-----------|
| **Trí** | `LinkedList<T>` template | ⬜ Đang chờ |
| **Trí** | `FileService` đọc/ghi 5 loại file | ⬜ Đang chờ |
| **Phát** | `AtmController` vòng lặp menu chính | ⬜ Đang chờ |
| **Phát** | `Common.h` (enum, hằng số) | ⬜ Đang chờ |

---

## 📌 V. CÁCH MERGE VỀ NHÁNH `main`

```bash
# Bước 1: Cập nhật nhánh cá nhân
git checkout tuan
git pull origin tuan

# Bước 2: Lấy code mới nhất từ main
git pull origin main

# Bước 3: Giải quyết conflict (nếu có), sau đó commit
git add .
git commit -m "merge: sync tuan branch with latest main"

# Bước 4: Tạo Pull Request trên GitHub từ tuan → main
# (Không push trực tiếp lên main)
```

---

## 📞 VI. LIÊN HỆ / REVIEW

- Nếu cần hỏi về logic **ConsoleView / UserController** → ping **Tuấn**
- Review code trước khi merge: ít nhất **1 thành viên khác** phải approve
- Blocker quá 4 giờ → báo nhóm ngay theo quy định `docs/06_ke_hoach_trien_khai.md`
