# 07. KẾ HOẠCH KIỂM THỬ & KIỂM THỬ BỘ NHỚ (TEST PLAN & QA)

---

## I. CHIẾN LƯỢC KIỂM THỬ (TEST STRATEGY)

Hệ thống áp dụng các cấp độ kiểm thử sau:

1. **Unit Testing (Kiểm thử đơn vị)**: Kiểm tra hoạt động của Cấu trúc dữ liệu `LinkedList<T>` (các hàm thêm, tìm, xóa, dọn dẹp) và các hàm kiểm tra ràng buộc tài chính (`canWithdraw`).
2. **Integration Testing (Kiểm thử tích hợp)**: Kiểm tra sự phối hợp giữa `AtmController`, `FileService` và hệ thống tệp tin `.txt` trên đĩa.
3. **Input Stress Testing (Tấn công dữ liệu đầu vào)**: Nhập chuỗi ký tự vào trường số (`cin.fail()`), gõ phím Backspace liên tục khi nhập mã PIN.
4. **Memory Leak Testing (Kiểm định rò rỉ bộ nhớ)**: Chạy ứng dụng dưới sự giám sát của công cụ **Valgrind** để đảm bảo giải phóng 100% vùng nhớ Heap.

---

## II. MA TRẬN KỊCH BẢN KIỂM THỬ (TEST CASES MATRIX)

### 1. Phân hệ Quản trị viên (Admin Module)

|    Test ID    | Tên ca kiểm thử                    | Dữ liệu đầu vào (Input)                                       | Kết quả mong đợi (Expected Output)                                                                                                                   | Đánh giá |
| :-----------: | :--------------------------------- | :------------------------------------------------------------ | :--------------------------------------------------------------------------------------------------------------------------------------------------- | :------: |
| **TC_ADM_01** | Đăng nhập Admin thành công         | User: `admin1`<br>Pass: `123456`                              | Mật khẩu hiện dấu `*`. Đăng nhập thành công, chuyển vào Menu Admin.                                                                                  |   Pass   |
| **TC_ADM_02** | Đăng nhập Admin thất bại           | User: `admin1`<br>Pass: `sai_pass`                            | Báo lỗi màu đỏ: "Sai user hoac mat khau!". Cho phép nhập lại.                                                                                        |   Pass   |
| **TC_ADM_03** | Xem danh sách thẻ từ               | Chọn Menu `1`                                                 | Hiển thị bảng danh sách toàn bộ thẻ trong `TheTu.txt` với đầy đủ ID và PIN.                                                                          |   Pass   |
| **TC_ADM_04** | Thêm thẻ từ mới hợp lệ             | ID: `10014504509999`<br>Tên: `Tran Van An`<br>Số dư: `200000` | • Thêm thành công.<br>• File `TheTu.txt` có thêm dòng `10014504509999 123456`.<br>• Tự sinh file `10014504509999.txt` và `LichSu10014504509999.txt`. |   Pass   |
| **TC_ADM_05** | Thêm thẻ thất bại do trùng ID      | ID: `10014504500001` (đã có)                                  | Báo lỗi màu đỏ: "ID the da ton tai trong he thong!". Hủy thao tác.                                                                                   |   Pass   |
| **TC_ADM_06** | Thêm thẻ thất bại do sai định dạng | ID: `12345` (không đủ 14 số) hoặc `100145abc00001`            | Báo lỗi: "ID phai bao gom dung 14 chu so!". Cho nhập lại.                                                                                            |   Pass   |
| **TC_ADM_07** | Xóa thẻ thành công                 | ID: `10014504509999`                                          | • Xác nhận Yes.<br>• File `data/10014504509999.txt` bị xóa.<br>• Dòng thẻ bị gỡ khỏi `TheTu.txt`.<br>• File lịch sử vẫn được giữ lại.                |   Pass   |
| **TC_ADM_08** | Mở khóa tài khoản                  | Chọn ID trong danh sách bị khóa                               | Thẻ được mở khóa, xóa ID khỏi `KhoaThe.txt`, số lần nhập sai về 0.                                                                                   |   Pass   |

---

### 2. Phân hệ Khách hàng (User Module)

|    Test ID    | Tên ca kiểm thử                                      | Dữ liệu đầu vào (Input)                           | Kết quả mong đợi (Expected Output)                                                                                                                 | Đánh giá |
| :-----------: | :--------------------------------------------------- | :------------------------------------------------ | :------------------------------------------------------------------------------------------------------------------------------------------------- | :------: |
| **TC_USR_01** | Đăng nhập thành công với PIN thường                  | ID: `10014504500001`<br>PIN: `654321`             | PIN che thành `*`. Đăng nhập thành công, hiển thị Menu User.                                                                                       |   Pass   |
| **TC_USR_02** | Đăng nhập lần đầu với PIN mặc định                   | ID: `10014504500002`<br>PIN: `123456`             | Hệ thống chặn lại, yêu cầu đổi mã PIN mới ngay lập tức. Sau khi đổi thành công mới vào Menu chính.                                                 |   Pass   |
| **TC_USR_03** | Đăng nhập sai PIN lần 1 và 2                         | Nhập sai PIN                                      | Báo lỗi: "Sai ma PIN. Ban con X lan thu!".                                                                                                         |   Pass   |
| **TC_USR_04** | Đăng nhập sai PIN quá 3 lần                          | Nhập sai PIN 3 lần liên tiếp                      | Báo lỗi: "The da bi khoa do nhap sai 3 lan!". Ghi ID vào `KhoaThe.txt`, thoát ứng dụng.                                                            |   Pass   |
| **TC_USR_05** | Đăng nhập bằng thẻ đã bị khóa                        | ID thẻ đang trong `KhoaThe.txt`                   | Báo lỗi: "Tai khoan da bi khoa. Vui long lien he Admin!". Từ chối đăng nhập.                                                                       |   Pass   |
| **TC_USR_06** | Xem thông tin & số dư                                | Chọn Menu `1`                                     | Hiển thị ID, Tên, Số dư (định dạng rõ ràng), Loại tiền tệ từ file `[ID].txt`.                                                                      |   Pass   |
| **TC_USR_07** | Rút tiền hợp lệ                                      | Số dư: 200.000<br>Rút: 100.000                    | • Xác nhận Yes.<br>• Trừ số dư còn 100.000 trong `[ID].txt`.<br>• Ghi dòng rút tiền vào `[LichSuID].txt`.                                          |   Pass   |
| **TC_USR_08** | Rút tiền thất bại (< 50k hoặc không phải bội số 50k) | Rút: `30000` hoặc `75000`                         | Báo lỗi màu đỏ: "So tien phai >= 50.000 VND va la boi so cua 50.000 VND!".                                                                         |   Pass   |
| **TC_USR_09** | Rút tiền thất bại do vi phạm số dư tối thiểu         | Số dư: 100.000<br>Rút: 100.000                    | Báo lỗi: "So du khong du. Phai duy tri toi thieu 50.000 VND trong tai khoan!".                                                                     |   Pass   |
| **TC_USR_10** | Chuyển tiền hợp lệ                                   | Người nhận: `10014504500003`<br>Số tiền: `100000` | • Hiển thị đúng tên người nhận để xác nhận.<br>• Trừ 100k tài khoản gửi, cộng 100k tài khoản nhận.<br>• Ghi nhận log ở cả 2 file `[LichSuID].txt`. |   Pass   |
| **TC_USR_11** | Chuyển tiền thất bại do STK nhận sai                 | STK nhận không tồn tại, hoặc trùng STK gửi        | Báo lỗi: "Tai khoan thu huong khong hop le!".                                                                                                      |   Pass   |
| **TC_USR_12** | Đổi mã PIN thành công                                | PIN cũ đúng, PIN mới: `888999` (nhập 2 lần)       | Đổi thành công, cập nhật PIN mới vào `TheTu.txt`.                                                                                                  |   Pass   |
| **TC_USR_13** | Đổi mã PIN thất bại                                  | PIN mới trùng PIN cũ hoặc trùng `123456`          | Báo lỗi: "PIN moi khong duoc trung voi PIN cu hoac PIN mac dinh!".                                                                                 |   Pass   |

---

### 3. Tấn công Luồng nhập liệu (Input Robustness Testing)

|    Test ID    | Kịch bản kiểm thử           | Hành động kiểm thử                                  | Kết quả mong đợi                                                                                                            |
| :-----------: | :-------------------------- | :-------------------------------------------------- | :-------------------------------------------------------------------------------------------------------------------------- |
| **TC_INP_01** | Nhập chữ vào trường số tiền | Nhập chuỗi `abc`, `50k`, `$#@` vào ô nhập tiền      | Hàm `inputMoney()` bẫy được `cin.fail()`, xóa bộ đệm, in chữ đỏ nhắc nhập lại, **tuyệt đối không bị treo vòng lặp vô hạn**. |
| **TC_INP_02** | Xóa lùi phím Backspace      | Khi gõ mật khẩu, gõ 4 phím rồi nhấn Backspace 2 lần | Trên màn hình xóa lùi 2 dấu `*`, chuỗi mật khẩu trong RAM còn đúng 2 ký tự.                                                 |
| **TC_INP_03** | Nhập khoảng trắng cho Tên   | Admin nhập tên `Nguyen Trung Kien`                  | File lưu đúng chuỗi có khoảng trắng, khi đọc lại không làm lệch số dư dòng dưới.                                            |

---

## III. KIỂM ĐỊNH RÒ RỈ BỘ NHỚ VỚI VALGRIND

Để đảm bảo đáp ứng chuẩn C++ Coding Standard V2 (Rule 17: Có `new` phải có `delete`), nhóm sử dụng công cụ **Valgrind** trên Linux để kiểm tra:

### 1. Lệnh thực thi:

```bash
make clean
make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/atm_app
```

### 2. Tiêu chuẩn Chấp thuận (Acceptance Criteria):

- Mọi phân vùng nhớ Heap được cấp phát cho `Node<T>` của `LinkedList` hoặc đối tượng `Account* _pCurrentAccount` phải được giải phóng hoàn toàn khi tắt chương trình.
- Kết quả phân tích của Valgrind bắt buộc phải trả về:
  ```text
  ==HEAP SUMMARY:==
  ==     in use at exit: 0 bytes in 0 blocks
  ==   total heap usage: X allocs, X frees, Y bytes allocated
  == All heap blocks were freed -- no leaks are possible
  == ERROR SUMMARY: 0 errors from 0 contexts
  ```

---

## IV. DANH MỤC NGHIỆM THU CUỐI CÙNG (DEFINITION OF DONE - DOD)

Trước khi đóng gói mã nguồn và nộp bài lên hệ thống Classroom, nhóm rà soát theo checklist sau:

- [ ] **Biên dịch**: Lệnh `make` chạy thành công không có bất kỳ warning nào (`-Wall -Wextra`).
- [ ] **Bộ nhớ**: Chạy Valgrind thông báo `0 bytes leaked`.
- [ ] **Thư mục dữ liệu**: Thư mục `data/` có sẵn ít nhất 3 Admin trong `Admin.txt` và 10 thẻ từ trong `TheTu.txt`.
- [ ] **Bảo mật**: Mật khẩu Admin và mã PIN User được ẩn dấu `*` hoàn toàn.
- [ ] **Ràng buộc**: Pass 100% các Test Case trong ma trận kiểm thử ở trên.
- [ ] **Tài liệu bàn giao**: File báo cáo Word hoàn thiện theo mẫu `Mau_BaoCao_DoAn.docx` và Video demo sẵn sàng nộp.
