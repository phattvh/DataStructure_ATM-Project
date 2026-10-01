# 01. TIÊU CHÍ ĐÁNH GIÁ & CÁC LƯU Ý KỸ THUẬT

Tài liệu này tổng hợp tiêu chuẩn đánh giá chính thức của học phần **Cấu trúc Dữ liệu & Giải thuật (HCMUE)** cho **Project 1: Mô phỏng hệ thống máy ATM**, cùng các bẫy kỹ thuật lập trình C++ thường gặp cần tránh để không bị trừ điểm.

---

## I. THANG ĐIỂM CHÍNH THỨC (TỔNG: 10.0 ĐIỂM)

Căn cứ theo văn bản *Yêu cầu thực hiện Project* của bộ môn:

| STT | Hạng mục đánh giá | Điểm số | Yêu cầu cốt lõi & Quy định trừ điểm |
| :---: | :--- | :---: | :--- |
| **1** | **Hướng đối tượng (OOP) & Template** | **2.0 đ** | • Bắt buộc cài đặt mô hình lớp (OOP) chuẩn mực.<br>• Bắt buộc tự xây dựng và sử dụng **Template Cấu trúc dữ liệu** (như `LinkedList<T>`).<br>*(Làm theo hướng khác như lập trình thủ tục hoặc không dùng template sẽ bị **trừ 2.0 điểm**)*. |
| **2** | **Tuân thủ C++ Coding Standard V2** | **1.0 đ** | • Tuân thủ triệt để tài liệu *C++ Coding Standard Version 2* của khoa CNTT.<br>• Quy tắc Hungarian Notation cho biến (`strName`, `iCount`, `_lBalance`).<br>• Tách riêng file `.h` và `.cpp`, sử dụng `this->`, không rò rỉ bộ nhớ (`new`/`delete`), đóng file sau khi mở.<br>*(Vi phạm quy chuẩn sẽ bị **trừ 1.0 điểm**)*. |
| **3** | **Chức năng & Ràng buộc Dữ liệu** | **5.0 đ** | • Thực hiện đầy đủ 100% chức năng của Phân hệ Admin và Phân hệ User.<br>• Đáp ứng đầy đủ các ràng buộc: khóa thẻ sau 3 lần sai, ép đổi PIN mặc định lần đầu, rút/chuyển $\ge 50.000$ VNĐ, là bội số của $50.000$ VNĐ, giữ lại tối thiểu $50.000$ VNĐ, tự sinh và cập nhật file.<br>*(Thiếu mỗi chức năng hoặc 1 ràng buộc dữ liệu sẽ bị **trừ 0.5 – 1.0 điểm**)*. |
| **4** | **Báo cáo Word** | **1.0 đ** | • Soạn thảo đầy đủ, đúng quy cách theo mẫu `Mau_BaoCao_DoAn.docx` của trường. |
| **5** | **Video Báo cáo Demo** | **1.0 đ** | • Quay màn hình chạy thực tế chương trình, thuyết minh rõ ràng các kịch bản test. |

---

## II. CÁC "BẪY KỸ THUẬT" QUAN TRỌNG KHI LẬP TRÌNH C++

Để bảo đảm chương trình không phát sinh lỗi bất thường (undefined behavior), crash hoặc rò rỉ bộ nhớ khi chấm thi, các thành viên cần tuân thủ các giải pháp kỹ thuật sau:

### 1. Bẫy đọc file `[ID].txt` (Họ tên có khoảng trắng)
* **Vấn đề**: File thông tin tài khoản gồm 4 dòng:
  ```
  Dòng 1: ID (ví dụ: 10014504501111)
  Dòng 2: Họ và tên (ví dụ: Nguyen Trung Kien)
  Dòng 3: Số dư (ví dụ: 100000)
  Dòng 4: Loại tiền tệ (ví dụ: VND)
  ```
  Nếu đọc bằng toán tử `file >> id >> balance` xen kẽ với `std::getline(file, name)`, ký tự xuống dòng `\n` còn sót lại trong bộ đệm sẽ khiến trường Họ tên bị đọc rỗng và số dư bị nhảy dòng sai lệch.
* **Giải pháp**:
  Luôn luôn dùng `std::getline()` để đọc trọn vẹn từng dòng dưới dạng chuỗi `std::string`, sau đó dùng hàm chuẩn `std::stol()` hoặc `std::stoll()` để chuyển đổi số dư:
  ```cpp
  string strLine;
  // Dòng 1: ID
  getline(file, strId);
  // Dòng 2: Họ tên (có khoảng trắng)
  getline(file, strName);
  // Dòng 3: Số dư
  if (getline(file, strLine)) {
      lBalance = std::stol(strLine);
  }
  // Dòng 4: Tiền tệ
  getline(file, strCurrency);
  ```

---

### 2. Bẫy trôi dòng lệnh khi nhập số tiền (`cin.fail()`)
* **Vấn đề**: Khi chương trình yêu cầu nhập số tiền rút hoặc chuyển (kiểu `long`), nếu người dùng hoặc giảng viên vô tình nhập chữ cái (ví dụ: `abc` hoặc `50k`), luồng `std::cin` sẽ chuyển sang trạng thái lỗi (`failbit`), bỏ qua mọi lệnh nhập tiếp theo và rơi vào vòng lặp vô hạn khiến máy treo.
* **Giải pháp**:
  Xây dựng hàm chuyên biệt `inputMoney()` hoặc `inputNumber()` có bẫy lỗi và xóa sạch bộ đệm:
  ```cpp
  long lAmount = 0;
  while (!(std::cin >> lAmount)) {
      std::cin.clear(); // Xóa trạng thái lỗi
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Xóa sạch dữ liệu rác trong buffer
      ConsoleView::printError("Dinh dang khong hop le. Vui long nhap lai so tien: ");
  }
  ```

---

### 3. Định dạng Thời gian Giao dịch chuẩn xác
* **Vấn đề**: File lịch sử giao dịch `[LichSuID].txt` yêu cầu lưu chính xác thời điểm thực hiện giao dịch. Nếu tính toán giây thủ công sẽ dễ sai lệch và không chuyên nghiệp.
* **Giải pháp**:
  Sử dụng thư viện `<chrono>` và `<iomanip>` của chuẩn C++11 trở lên để sinh chuỗi thời gian định dạng chuẩn `YYYY-MM-DD HH:MM:SS`:
  ```cpp
  #include <chrono>
  #include <iomanip>
  #include <sstream>

  std::string getCurrentTimestamp() {
      auto now = std::chrono::system_clock::now();
      std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
      std::tm tmBuffer;
      localtime_r(&nowTime, &tmBuffer); // Linux POSIX an toàn luồng

      std::ostringstream oss;
      oss << std::put_time(&tmBuffer, "%Y-%m-%d %H:%M:%S");
      return oss.str();
  }
  ```

---

### 4. Template & Quản lý Bộ nhớ (Tránh lỗi Double Free)
* **Vấn đề**: Class Template `LinkedList<T>` tự quản lý cấp phát động các node bộ nhớ bằng `new`. Nếu truyền tham trị `LinkedList<T> list` vào các hàm (như trong `FileService`), cơ chế Copy ngầm định (Shallow Copy) sẽ sao chép con trỏ `_pHead`. Khi hàm kết thúc, Destructor sẽ gọi `delete` vùng nhớ đó. Sau đó danh sách gốc tiếp tục gọi Destructor khi hủy, gây ra lỗi nghiêm trọng `Double Free / Core Dumped`.
* **Giải pháp**:
  1. Xóa bỏ Copy Constructor và Assignment Operator của `LinkedList` để ép buộc kiểm tra lúc biên dịch:
     ```cpp
     LinkedList(const LinkedList<T>&) = delete;
     LinkedList<T>& operator=(const LinkedList<T>&) = delete;
     ```
  2. Luôn truyền đối tượng bằng tham chiếu: `LinkedList<T>& list` hoặc tham chiếu hằng `const LinkedList<T>& list`.

---

### 5. Quản lý Tài nguyên Tệp tin (File I/O)
* **Quy tắc**: Tuân thủ Rule 18 của Coding Standard: *"Có open thì phải có close"*.
* Luôn kiểm tra `if (!file.is_open())` trước khi thao tác.
* Đóng tệp ngay khi hoàn tất thao tác để bảo toàn dữ liệu trên đĩa, tránh xung đột khóa file.
