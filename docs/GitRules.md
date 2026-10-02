# 09. QUY TẮC PHÁT TRIỂN & QUẢN LÝ MÃ NGUỒN GIT
**Tác giả phụ trách**: Thành viên A (Tech Lead / Core System & Admin Flow)

---

## 1. Quy tắc Phân nhánh (Git Branching Strategy)

Nhóm áp dụng mô hình Git Flow rút gọn nhằm đảm bảo mã nguồn ổn định trên nhánh chính và cho phép các thành viên làm việc song song không gây xung đột mã (merge conflict):

- `main`: Nhánh chứa mã nguồn ổn định, đã qua kiểm thử và sẵn sàng nộp/đánh giá. Không commit trực tiếp lên `main`.
- `dev`: Nhánh tích hợp mã nguồn chung của nhóm. Tất cả các tính năng hoàn thành sẽ được tạo Pull Request (PR) về nhánh này.
- Nhánh tính năng cá nhân: Đặt tên theo cú pháp chuẩn:
  - `feature/<tên-tính-năng>`: Phát triển tính năng mới.
    - Ví dụ: `feature/admin-login`, `feature/card-management`, `feature/user-withdraw`.
  - `bugfix/<tên-lỗi>`: Khắc phục lỗi phát hiện trong quá trình tích hợp.
    - Ví dụ: `bugfix/cin-fail-loop`, `bugfix/file-leak`.
  - `refactor/<tên-module>`: Tái cấu trúc mã nguồn, tối ưu hóa thuật toán mà không đổi nghiệp vụ.
    - Ví dụ: `refactor/linked-list-memory`.
  - `test/<tên-test-suite>`: Bổ sung các bài kiểm thử tự động.
    - Ví dụ: `test/admin-flow`.

---

## 2. Quy tắc Viết Commit Message (Conventional Commits)

Nội dung commit message phải viết bằng tiếng Anh, rõ ràng, súc tích và tuân thủ định dạng:
```
<type>: <mô tả ngắn gọn về thay đổi>
```

Các tiền tố (`type`) bắt buộc:
- `feat`: Thêm tính năng mới (Feature).
  - Ví dụ: `feat: implement admin login and menu navigation`
- `fix`: Sửa lỗi chương trình (Bug fix).
  - Ví dụ: `fix: resolve cin fail infinite loop in money input`
- `docs`: Thêm hoặc cập nhật tài liệu dự án, báo cáo Word.
  - Ví dụ: `docs: add system architecture and git guidelines`
- `style`: Định dạng code, dấu cách, thụt lề, không thay đổi logic.
  - Ví dụ: `style: format code according to coding standard v2`
- `refactor`: Tái cấu trúc mã nguồn (không thêm tính năng, không sửa lỗi).
  - Ví dụ: `refactor: optimize linked list traversal and destructor`
- `test`: Bổ sung hoặc cập nhật bộ kiểm thử tự động.
  - Ví dụ: `test: add unit tests for card unlock algorithm`
- `chore`: Cập nhật cấu hình build, Makefile, .gitignore, thư viện.
  - Ví dụ: `chore: setup project structure and makefile`

---

## 3. Quy trình Phối hợp & Kiểm soát Chất lượng (Quality Gates)

Toàn bộ thành viên nhóm cam kết tuân thủ 3 nguyên tắc vận hành:

1. **Pull before Push**:
   - Trước khi bắt đầu viết code hoặc tạo commit để push, luôn chạy `git pull origin dev` để đồng bộ mã mới nhất.
   - Biên dịch và chạy toàn bộ unit tests trên máy cục bộ trước khi push.
2. **Blocker Check (Nguyên tắc gỡ lỗi dưới 4 giờ)**:
   - Nếu gặp lỗi hoặc thắc mắc về nghiệp vụ không giải quyết được trong vòng 4 tiếng, thành viên phải chủ động thông báo trên nhóm để Tech Lead (Member A) hoặc các thành viên còn lại hỗ trợ pair-debugging.
3. **Tuân thủ Chuẩn Coding Standard V2**:
   - Mọi Pull Request trước khi merge vào nhánh chung phải được kiểm tra (code review):
     - Không có biến thừa, không che lấp biến (shadowing).
     - Đặt tên theo Hungarian Notation và tiền tố `_` cho biến thành viên.
     - Sử dụng `this->` khi gọi phương thức và biến nội bộ.
     - Kiểm tra dọn dẹp bộ nhớ (new phải đi kèm delete).
     - Mở file phải có đóng file (`file.close()`).
