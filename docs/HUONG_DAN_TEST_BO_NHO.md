# 🧪 HƯỚNG DẪN KIỂM TRA RÒ RỈ BỘ NHỚ (WINDOWS & LINUX)

> **Mục tiêu:** Đạt tiêu chuẩn **0 byte rò rỉ bộ nhớ** (`0 bytes leaked`), lấy trọn vẹn **1.0 điểm** tiêu chí *C++ Coding Standard V2 (Rule 17)*.

---

## 🪟 I. TRÊN HỆ ĐIỀU HÀNH WINDOWS (CỰC KỲ ĐƠN GIẢN)

Bạn **không cần cài thêm bất kỳ phần mềm nào**, chỉ cần mở terminal tại thư mục dự án và gõ đúng **1 lệnh**:

### 1. Lệnh thực hiện:
```powershell
make test_mem
```

### 2. Cách đọc kết quả:
Khi chạy xong, nhìn vào bảng tổng kết ở cuối màn hình:

```text
==============================================================
                  TONG KET KIEM THU BO NHO                    
==============================================================
  Tong so Test Cases pass: 67
  Tong so Test Cases fail: 0
  Tong so lan cap phat new/new[]:   150265
  Tong so lan giai phong del/del[]: 150265
  So block nho con ton tai (Active): 0
  So byte bo nho bi ro ri (Leaked): 0 bytes   <--- [CHÍNH LÀ ĐÂY: 0 BYTES]
  So loi Double Free ghi nhan:       0
  Dinh bo nho su dung (Peak Memory): 781.29 KB

  ==============================================================
    [HOAN TOAN DAT CHUAN] 0 BYTES LEAKED - HE THONG SACH SE 100% 
  ==============================================================
```

* 👉 **Nếu thấy `0 bytes` và thông báo xanh lá:** Hệ thống giải phóng bộ nhớ sạch $100\%$, đạt chuẩn!

---

## 🐧 II. TRÊN HỆ ĐIỀU HÀNH LINUX / WSL2 (DÙNG ĐỂ NỘP BÀI & BẢO VỆ)

Đây là chuẩn của giảng viên khi chấm bài hoặc khi nhóm cần chụp ảnh đưa vào Báo cáo Word.

### Bước 1: Cài đặt Valgrind (Chỉ làm 1 lần nếu máy chưa có)
```bash
sudo apt update && sudo apt install -y valgrind
```

### Bước 2: Chạy kiểm tra tự động
Dự án đã có sẵn script tự động, bạn chỉ cần chạy:
```bash
chmod +x scripts/valgrind_check.sh
./scripts/valgrind_check.sh
```

*(Hoặc gõ lệnh thủ công nếu muốn)*:
```bash
make test_mem
valgrind --leak-check=full --show-leak-kinds=all ./build/test_mem
```

### 3. Cách đọc kết quả Valgrind:
Tìm đoạn thông tin `HEAP SUMMARY` ở cuối:

```text
==HEAP SUMMARY:==
==     in use at exit: 0 bytes in 0 blocks     <--- [KHÔNG CÒN BYTE NÀO TỒN ĐỌNG]
==   total heap usage: 150,265 allocs, 150,265 frees
== All heap blocks were freed -- no leaks are possible
== ERROR SUMMARY: 0 errors from 0 contexts     <--- [0 LỖI]
```

* 👉 **Nếu thấy `0 bytes in 0 blocks` và `0 errors`:** Bài làm của nhóm đạt điểm tối đa!

---

## 📊 BẢNG TỔNG HỢP NHANH

| Tiêu chí | Trên Windows | Trên Linux / WSL2 |
| :--- | :--- | :--- |
| **Công cụ sử dụng** | Custom C++ Memory Tracker (Tự động nạp chồng `new`/`delete`) | Valgrind Memcheck |
| **Cài đặt thêm** | **Không cần** (Chạy ngay lập tức) | Cần cài package `valgrind` |
| **Lệnh thực thi** | `make test_mem` | `./scripts/valgrind_check.sh` |
| **Mục đích sử dụng** | Kiểm tra nhanh hàng ngày khi lập trình | Lấy bằng chứng nộp Báo cáo & Video Demo |
| **Tiêu chuẩn đạt** | `0 bytes leaked` (Xanh lá) | `0 bytes in 0 blocks` & `0 errors` |

---

## 📸 HƯỚNG DẪN LẤY BẰNG CHỨNG CHO BÁO CÁO & VIDEO
1. **Cho Báo cáo Word:**
   * Chạy `make test_mem` trên Windows hoặc `./scripts/valgrind_check.sh` trên Linux.
   * Chụp ảnh màn hình bảng tổng kết terminal có hiển thị rõ `0 bytes leaked`.
   * Dán ảnh vào mục **"Kiểm thử rò rỉ bộ nhớ"** trong file `Mau_BaoCao_DoAn.docx`.
2. **Cho Video Demo:**
   * Mở terminal, gõ `make test_mem` và thuyết minh:  
     *"Nhóm đã triển khai bộ kiểm định bộ nhớ tự động, kiểm tra hơn 150,000 lượt cấp phát và giải phóng, kết quả đạt chuẩn 0 byte rò rỉ."*
