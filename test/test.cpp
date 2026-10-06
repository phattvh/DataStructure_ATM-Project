/******************************************************************************
 * @file: test.cpp
 * @description: Bộ kiểm thử đơn vị & tích hợp toàn diện (QA Test Suite)
 *               Dành cho kiểm thử viên kỹ thuật (Thành viên B) đánh giá nhánh fix
 *               của dự án DataStructure_ATM-Project (Tuần 1).
 *
 * Tiêu chí bao phủ:
 *   1. Happy path: Luồng xử lý chuẩn của Card, Account, UserController.
 *   2. Boundary: Các giá trị biên (50k, bội số 50k, số dư duy trì 50k, tràn số long max).
 *   3. Negative: Input sai định dạng, sai số tiền, sai PIN, khóa thẻ 3 lần.
 *   4. Stress test: Vòng lặp 100,000 lượt kiểm tra hiệu năng tính toán.
 *   5. Memory & Invariants: Kiểm tra tính toàn vẹn khi sao chép và quản lý bộ nhớ.
 *
 * Cách biên dịch & chạy:
 *   g++ -std=c++17 -Wall -Wextra -I include test/test.cpp src/Card.cpp \
 *       src/Account.cpp src/ConsoleView.cpp src/UserController.cpp -o build/test_runner.exe
 *   ./build/test_runner.exe
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>
#include <limits>
#include <chrono>
#include <vector>

#include "Common.h"
#include "Card.h"
#include "Account.h"
#include "ConsoleView.h"
#include "UserController.h"

// Biến toàn cục ghi nhận thống kê
static int g_nTotalTests = 0;
static int g_nPassedTests = 0;
static int g_nFailedTests = 0;

/******************************************************************************
 * Framework kiểm thử nhẹ (Lightweight Assertion Framework)
 ******************************************************************************/
#define ASSERT_TRUE(condition, message) \
    do { \
        g_nTotalTests++; \
        if (condition) { \
            g_nPassedTests++; \
            std::cout << "  \033[32m[PASS]\033[0m " << (message) << "\n"; \
        } else { \
            g_nFailedTests++; \
            std::cout << "  \033[31m[FAIL]\033[0m " << (message) \
                      << " (Line " << __LINE__ << ")\n"; \
        } \
    } while (0)

#define ASSERT_FALSE(condition, message) ASSERT_TRUE(!(condition), message)

#define ASSERT_EQUAL(actual, expected, message) \
    do { \
        g_nTotalTests++; \
        if ((actual) == (expected)) { \
            g_nPassedTests++; \
            std::cout << "  \033[32m[PASS]\033[0m " << (message) << "\n"; \
        } else { \
            g_nFailedTests++; \
            std::cout << "  \033[31m[FAIL]\033[0m " << (message) \
                      << " [Expected: " << (expected) << ", Got: " << (actual) \
                      << "] (Line " << __LINE__ << ")\n"; \
        } \
    } while (0)

/******************************************************************************
 * NHÓM 1: KIỂM THỬ MODEL CARD (Thực thể thẻ từ)
 ******************************************************************************/
void testSuiteCardModel() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 1] MODEL CARD: HAPPY PATH, BOUNDARY, NEGATIVE\n";
    std::cout << "======================================================\n";

    // 1. Happy Path - Khởi tạo mặc định
    Card cardDefault;
    ASSERT_EQUAL(cardDefault.getId(), std::string(""), "Card mac dinh co ID rong");
    ASSERT_EQUAL(cardDefault.getPin(), DEFAULT_PIN, "Card mac dinh mang PIN mac dinh 123456");
    ASSERT_TRUE(cardDefault.isDefaultPin(), "Phat hien ma PIN mac dinh la true");
    ASSERT_FALSE(cardDefault.isLocked(), "The mac dinh khong bi khoa");
    ASSERT_EQUAL(cardDefault.getFailedAttempts(), 0, "So lan sai ban dau bang 0");

    // 2. Happy Path - Khởi tạo có tham số hợp lệ
    Card cardUser("10014504500001", "654321");
    ASSERT_EQUAL(cardUser.getId(), std::string("10014504500001"), "Khoi tao dung ID the");
    ASSERT_EQUAL(cardUser.getPin(), std::string("654321"), "Khoi tao dung PIN ban dau");
    ASSERT_FALSE(cardUser.isDefaultPin(), "The khong mang PIN mac dinh");
    ASSERT_TRUE(cardUser.checkPin("654321"), "Xac thuc ma PIN chinh xac tra ve true");
    ASSERT_FALSE(cardUser.checkPin("123456"), "Xac thuc ma PIN sai tra ve false");

    // 3. Boundary - Kiểm tra mã PIN biên
    ASSERT_TRUE(Card::isValidPinFormat("000000"), "PIN bien nho nhat '000000' hop le");
    ASSERT_TRUE(Card::isValidPinFormat("999999"), "PIN bien lon nhat '999999' hop le");

    // 4. Negative - Kiểm tra định dạng mã PIN bất hợp lệ
    ASSERT_FALSE(Card::isValidPinFormat(""), "PIN rong khong hop le");
    ASSERT_FALSE(Card::isValidPinFormat("12345"), "PIN 5 chu so khong hop le");
    ASSERT_FALSE(Card::isValidPinFormat("1234567"), "PIN 7 chu so khong hop le");
    ASSERT_FALSE(Card::isValidPinFormat("12345a"), "PIN chua chu cai khong hop le");
    ASSERT_FALSE(Card::isValidPinFormat("12 456"), "PIN chua dau cach khong hop le");
    ASSERT_FALSE(Card::isValidPinFormat("123-56"), "PIN chua ky tu dac biet khong hop le");

    // 5. Logic khóa thẻ sau 3 lần sai
    cardUser.recordFailedAttempt();
    ASSERT_EQUAL(cardUser.getFailedAttempts(), 1, "Sai lan 1: Bien dem bang 1");
    ASSERT_FALSE(cardUser.isLocked(), "Sai lan 1: The chua bi khoa");

    cardUser.recordFailedAttempt();
    ASSERT_EQUAL(cardUser.getFailedAttempts(), 2, "Sai lan 2: Bien dem bang 2");
    ASSERT_FALSE(cardUser.isLocked(), "Sai lan 2: The chua bi khoa");

    cardUser.recordFailedAttempt();
    ASSERT_EQUAL(cardUser.getFailedAttempts(), 3, "Sai lan 3: Bien dem bang 3");
    ASSERT_TRUE(cardUser.isLocked(), "Sai lan 3: The tu dong chuyen sang trang thai BI KHOA");

    // Thử sai tiếp khi đã khóa: Không được tăng vô hạn
    cardUser.recordFailedAttempt();
    ASSERT_TRUE(cardUser.isLocked(), "The van bi khoa khi goi tiep recordFailedAttempt");

    // 6. Nguyên lý SRP: Phân biệt resetFailedAttempts vs unlockCard
    cardUser.resetFailedAttempts();
    ASSERT_EQUAL(cardUser.getFailedAttempts(), 0, "resetFailedAttempts: Reset bien dem ve 0");
    ASSERT_TRUE(cardUser.isLocked(), "resetFailedAttempts: KHONG duoc tu mo khoa the");

    cardUser.unlockCard();
    ASSERT_FALSE(cardUser.isLocked(), "unlockCard: Mo khoa the thanh cong");
    ASSERT_EQUAL(cardUser.getFailedAttempts(), 0, "unlockCard: Dat lai so lan sai ve 0");

    // 7. Đổi mã PIN có kiểm tra validation
    ASSERT_FALSE(cardUser.changePin("abc"), "Tu choi doi PIN khi sai dinh dang chu");
    ASSERT_FALSE(cardUser.changePin("12345"), "Tu choi doi PIN khi thieu ky tu");
    ASSERT_TRUE(cardUser.changePin("888888"), "Doi PIN hop le '888888' thanh cong");
    ASSERT_EQUAL(cardUser.getPin(), std::string("888888"), "Ma PIN da duoc cap nhat moi");
}

/******************************************************************************
 * NHÓM 2: KIỂM THỬ MODEL ACCOUNT (Tài khoản & Ràng buộc tài chính)
 ******************************************************************************/
void testSuiteAccountModel() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 2] MODEL ACCOUNT: TAI CHINH & RANG BUOC\n";
    std::cout << "======================================================\n";

    // 1. Happy Path - Khởi tạo và truy xuất
    Account acc("10014504500001", "NGUYEN VAN AN", 500000, "VND");
    ASSERT_EQUAL(acc.getId(), std::string("10014504500001"), "Khoi tao dung ID tai khoan");
    ASSERT_EQUAL(acc.getName(), std::string("NGUYEN VAN AN"), "Khoi tao dung ten chu tai khoan");
    ASSERT_EQUAL(acc.getBalance(), 500000L, "Khoi tao dung so du ban dau 500,000 VND");
    ASSERT_EQUAL(acc.getCurrency(), std::string("VND"), "Don vi tien te mac dinh VND");

    // 2. Negative & Boundary - canWithdraw Ràng buộc tài chính
    // 2.1 Rút dưới mức tối thiểu 50,000 VND
    ASSERT_EQUAL(acc.canWithdraw(0), ERR_INVALID_AMOUNT, "Rut 0 VND -> ERR_INVALID_AMOUNT");
    ASSERT_EQUAL(acc.canWithdraw(-50000), ERR_INVALID_AMOUNT, "Rut so am -> ERR_INVALID_AMOUNT");
    ASSERT_EQUAL(acc.canWithdraw(20000), ERR_INVALID_AMOUNT, "Rut 20k (< 50k) -> ERR_INVALID_AMOUNT");
    ASSERT_EQUAL(acc.canWithdraw(49999), ERR_INVALID_AMOUNT, "Rut 49,999 VND (< 50k) -> ERR_INVALID_AMOUNT");

    // 2.2 Rút không phải bội số của 50,000 VND
    ASSERT_EQUAL(acc.canWithdraw(75000), ERR_NOT_MULTIPLE, "Rut 75k khong la boi so 50k -> ERR_NOT_MULTIPLE");
    ASSERT_EQUAL(acc.canWithdraw(120000), ERR_NOT_MULTIPLE, "Rut 120k khong la boi so 50k -> ERR_NOT_MULTIPLE");
    ASSERT_EQUAL(acc.canWithdraw(100001), ERR_NOT_MULTIPLE, "Rut 100,001 VND -> ERR_NOT_MULTIPLE");

    // 2.3 Rút vi phạm số dư duy trì tối thiểu (50,000 VND)
    ASSERT_EQUAL(acc.canWithdraw(500000), ERR_INSUFFICIENT_FUNDS, "Rut het 500k khong con 50k duy tri -> ERR_INSUFFICIENT_FUNDS");
    ASSERT_EQUAL(acc.canWithdraw(460000), ERR_NOT_MULTIPLE, "460k khong la boi so");
    ASSERT_EQUAL(acc.canWithdraw(600000), ERR_INSUFFICIENT_FUNDS, "Rut vuot so du 600k -> ERR_INSUFFICIENT_FUNDS");

    // 2.4 Boundary Hợp lệ - Rút đúng mức biên giữ lại đúng 50,000 VND
    ASSERT_EQUAL(acc.canWithdraw(450000), ERR_NONE, "Rut 450k giu lai dung 50k duy tri -> ERR_NONE");
    ASSERT_EQUAL(acc.canWithdraw(50000), ERR_NONE, "Rut dung han muc toi thieu 50k -> ERR_NONE");

    // 3. Thực thi rút tiền và nạp tiền
    ASSERT_TRUE(acc.withdraw(200000), "Rut 200,000 VND thanh cong");
    ASSERT_EQUAL(acc.getBalance(), 300000L, "So du con lai chinh xac 300,000 VND");

    ASSERT_FALSE(acc.withdraw(300000), "Rut tiep 300k that bai do vi pham so du duy tri");
    ASSERT_EQUAL(acc.getBalance(), 300000L, "So du khong bi anh huong khi giao dich loi");

    ASSERT_TRUE(acc.deposit(100000), "Nap 100,000 VND thanh cong");
    ASSERT_EQUAL(acc.getBalance(), 400000L, "So du sau nap tang len 400,000 VND");

    // 4. Negative - Nạp tiền số âm, số 0
    ASSERT_FALSE(acc.deposit(0), "Chan nap 0 VND (tra ve false)");
    ASSERT_FALSE(acc.deposit(-100000), "Chan nap so tien am (tra ve false)");
    ASSERT_EQUAL(acc.getBalance(), 400000L, "So du van giu nguyen 400,000 VND");

    // 5. Boundary - Chống tràn số nguyên (Integer Overflow Guard)
    long lMaxLong = std::numeric_limits<long>::max();
    ASSERT_FALSE(acc.deposit(lMaxLong), "Chan nap so tien gay tran gioi han long::max()");
    ASSERT_EQUAL(acc.getBalance(), 400000L, "So du van bao toan khi co hanh vi tran so");
}

/******************************************************************************
 * NHÓM 3: KIỂM THỬ GIAO DIỆN CONSOLEVIEW (I/O Streams & Bẫy lỗi)
 ******************************************************************************/
void testSuiteConsoleViewIO() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 3] CONSOLEVIEW: I/O STREAMS & BAY LOI\n";
    std::cout << "======================================================\n";

    // 1. Test inputMoney với chuỗi chuẩn
    {
        std::istringstream streamValid("  150000  \n");
        long lAmount = ConsoleView::inputMoney("Nhap tien: ", streamValid);
        ASSERT_EQUAL(lAmount, 150000L, "inputMoney doc dung so tien co khoang trang");
    }

    // 2. Test inputMoney bẫy lỗi chuỗi chữ, số âm, số thập phân rồi mới đến số đúng
    {
        std::istringstream streamRecovery("abc\n-50000\n50000.75\n200000\n");
        long lAmount = ConsoleView::inputMoney("Nhap tien: ", streamRecovery);
        ASSERT_EQUAL(lAmount, 200000L, "inputMoney bo qua chu, so am, so thap phan va lay dung so nguyen");
    }

    // 3. Test inputMoney phát hiện EOF không bị treo vô hạn
    {
        std::istringstream streamEOF("");
        long lAmount = ConsoleView::inputMoney("Nhap tien: ", streamEOF);
        ASSERT_EQUAL(lAmount, 0L, "inputMoney phat hien EOF thoat an toan tra ve 0");
    }

    // 4. Test inputPassword giả lập qua Stream
    {
        std::istringstream streamPass("123456\n");
        std::string strPass = ConsoleView::inputPassword("Nhap PIN: ", &streamPass);
        ASSERT_EQUAL(strPass, std::string("123456"), "inputPassword doc dung chuoi qua stream mock");
    }
}

/******************************************************************************
 * NHÓM 4: KIỂM THỬ USERCONTROLLER (Xác thực, Chuyển khoản ACID, Đổi PIN)
 ******************************************************************************/
void testSuiteUserController() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 4] USERCONTROLLER: LOGIC & ATOMIC TRANSFER\n";
    std::cout << "======================================================\n";

    Card card("10014504500002", "123456");
    Account sender("10014504500002", "TRAN THI HOA", 1000000, "VND");
    Account receiver("10014504500003", "LE VAN CUONG", 200000, "VND");

    // 1. Kiểm tra định dạng ID và PIN
    ASSERT_TRUE(UserController::isValidIdFormat("10014504500002"), "ID 14 chu so hop le");
    ASSERT_FALSE(UserController::isValidIdFormat("100145"), "ID ngan khong hop le");
    ASSERT_FALSE(UserController::isValidIdFormat("1001450450000a"), "ID chua chu khong hop le");
    ASSERT_TRUE(UserController::isValidPinFormat("123456"), "PIN 6 chu so hop le");
    ASSERT_FALSE(UserController::isValidPinFormat("1234"), "PIN ngan khong hop le");

    // 2. Xác thực đăng nhập qua authenticate
    bool bCardLocked = false;
    ASSERT_FALSE(UserController::authenticate(card, "000000", bCardLocked), "Nhap sai PIN khong dang nhap duoc");
    ASSERT_FALSE(bCardLocked, "Moi sai 1 lan chua khoa the");

    UserController::authenticate(card, "000000", bCardLocked);
    UserController::authenticate(card, "000000", bCardLocked);
    ASSERT_TRUE(bCardLocked, "Sai PIN 3 lan: Controller thong bao the bi khoa");
    ASSERT_TRUE(card.isLocked(), "Trang thai the chuyen thanh khoa");

    // Không cho phép đăng nhập khi thẻ đã khóa dù nhập đúng PIN
    ASSERT_FALSE(UserController::authenticate(card, "123456", bCardLocked), "The bi khoa khong cho dang nhap du dung PIN");
    ASSERT_TRUE(bCardLocked, "Co bao khoa van bat");

    // Mở khóa và đăng nhập thành công
    card.unlockCard();
    ASSERT_TRUE(UserController::authenticate(card, "123456", bCardLocked), "Mo khoa the thi dang nhap dung PIN thanh cong");

    // 3. Rút tiền qua Controller
    ErrorCode errWithdraw = UserController::processWithdraw(sender, 200000);
    ASSERT_EQUAL(errWithdraw, ERR_NONE, "processWithdraw rut 200k hop le");
    ASSERT_EQUAL(sender.getBalance(), 800000L, "So du nguoi gui con 800k");

    // 4. Chuyển tiền tự chuyển cho chính mình
    ErrorCode errSelf = UserController::processTransfer(sender, sender, 100000);
    ASSERT_EQUAL(errSelf, ERR_SAME_ACCOUNT, "Chan chuyen tien cho chinh minh -> ERR_SAME_ACCOUNT");

    // 5. Chuyển tiền thành công 2 chiều
    ErrorCode errTransfer = UserController::processTransfer(sender, receiver, 300000);
    ASSERT_EQUAL(errTransfer, ERR_NONE, "processTransfer chuyen 300k hop le");
    ASSERT_EQUAL(sender.getBalance(), 500000L, "Nguoi gui giam xuong 500k");
    ASSERT_EQUAL(receiver.getBalance(), 500000L, "Nguoi nhan tang len 500k");

    // 6. Tính nguyên tử ACID & Rollback khi người nhận bị tràn số
    receiver.setBalance(std::numeric_limits<long>::max() - 50000);
    ErrorCode errOverflow = UserController::processTransfer(sender, receiver, 100000);
    ASSERT_EQUAL(errOverflow, ERR_SYSTEM_OVERFLOW, "Phat hien tran so nguoi nhan -> ERR_SYSTEM_OVERFLOW");
    ASSERT_EQUAL(sender.getBalance(), 500000L, "Rollback thanh cong: So du nguoi gui duoc hoan nguyen ven");

    // Reset lại số dư người nhận
    receiver.setBalance(500000);

    // 7. Đổi mã PIN chủ động
    std::string strMsg;
    ASSERT_FALSE(UserController::processChangePin(card, "999999", "654321", "654321", strMsg), "Sai PIN cu khong cho doi");
    ASSERT_FALSE(UserController::processChangePin(card, "123456", "123456", "123456", strMsg), "PIN moi trung PIN cu bi tu choi");
    ASSERT_FALSE(UserController::processChangePin(card, "123456", "654321", "654320", strMsg), "Xac nhan khong khop bi tu choi");
    ASSERT_TRUE(UserController::processChangePin(card, "123456", "654321", "654321", strMsg), "Doi PIN hop le thanh cong");
    ASSERT_EQUAL(card.getPin(), std::string("654321"), "PIN moi da duoc luu");
}

/******************************************************************************
 * NHÓM 5: KIỂM THỬ STRESS & HIỆU NĂNG (100,000 LƯỢT THAO TÁC)
 ******************************************************************************/
void testSuiteStressPerformance() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 5] STRESS TEST: 100,000 LUOT THAO TAC\n";
    std::cout << "======================================================\n";

    auto tStart = std::chrono::high_resolution_clock::now();

    Account accTest("10014504500099", "STRESS TESTER", 100000000L);
    Card cardTest("10014504500099", "123456");

    const int ITERATIONS = 100000;
    int nPassOps = 0;

    for (int i = 0; i < ITERATIONS; ++i) {
        // Kiểm tra ràng buộc rút tiền
        if (accTest.canWithdraw(50000) == ERR_NONE) {
            nPassOps++;
        }
        // Kiểm tra mã PIN
        if (cardTest.checkPin("123456")) {
            nPassOps++;
        }
    }

    auto tEnd = std::chrono::high_resolution_clock::now();
    double dElapsedMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

    std::cout << "  Thoi gian thuc thi " << ITERATIONS << " vong lap: "
              << std::fixed << std::setprecision(2) << dElapsedMs << " ms\n";

    ASSERT_EQUAL(nPassOps, ITERATIONS * 2, "100,000 vong lap xu ly chinh xac 100%");
    ASSERT_TRUE(dElapsedMs < 500.0, "Hieu nang xu ly dat chuan (< 500 ms cho 100k ops)");
}

/******************************************************************************
 * NHÓM 6: QUẢN LÝ BỘ NHỚ & TÍNH TOÀN VẸN ĐỐI TƯỢNG (INVARIANTS & LIFECYCLE)
 ******************************************************************************/
void testSuiteMemoryAndInvariants() {
    std::cout << "\n======================================================\n";
    std::cout << " [TEST SUITE 6] MEMORY INVARIANTS: COPY & LIFECYCLE\n";
    std::cout << "======================================================\n";

    // 1. Kiểm tra Copy Constructor của Account
    Account accOrig("10014504500001", "NGUYEN VAN AN", 500000);
    Account accCopy = accOrig;
    ASSERT_EQUAL(accCopy.getId(), accOrig.getId(), "Copy ctor sao chep dung ID");
    ASSERT_EQUAL(accCopy.getBalance(), accOrig.getBalance(), "Copy ctor sao chep dung so du");

    // Thay đổi trên bản copy không ảnh hưởng bản gốc
    accCopy.withdraw(100000);
    ASSERT_EQUAL(accCopy.getBalance(), 400000L, "Ban sao bi tru tien");
    ASSERT_EQUAL(accOrig.getBalance(), 500000L, "Ban goc khong bi anh huong (Doc lap bo nho)");

    // 2. Kiểm tra Copy Assignment Operator của Card
    Card cardOrig("10014504500001", "123456");
    cardOrig.recordFailedAttempt();
    Card cardAssigned;
    cardAssigned = cardOrig;
    ASSERT_EQUAL(cardAssigned.getId(), cardOrig.getId(), "Assignment sao chep dung ID");
    ASSERT_EQUAL(cardAssigned.getFailedAttempts(), 1, "Assignment sao chep dung so lan sai");

    cardAssigned.unlockCard();
    ASSERT_EQUAL(cardAssigned.getFailedAttempts(), 0, "Ban sao duoc reset");
    ASSERT_EQUAL(cardOrig.getFailedAttempts(), 1, "Ban goc van giu nguyen so lan sai");

    // 3. Kiểm tra cấp phát động mảng đối tượng không bị rò rỉ hay lỗi huỷ
    {
        std::vector<Card> vecCards;
        for (int i = 0; i < 1000; ++i) {
            vecCards.emplace_back("1001450450" + std::to_string(1000 + i), "123456");
        }
        ASSERT_EQUAL(vecCards.size(), 1000ULL, "Cap phat dong danh sach 1000 doi tuong Card on dinh");
    } // vecCards tự động giải phóng sạch sẽ khi ra khỏi scope
    ASSERT_TRUE(true, "Giai phong bo nho RAII thanh cong, khong co loi crash");
}

/******************************************************************************
 * HÀM CHÍNH (TEST RUNNER ENTRY POINT)
 ******************************************************************************/
int main() {
    auto tTotalStart = std::chrono::high_resolution_clock::now();

    std::cout << "\n";
    std::cout << "##############################################################\n";
    std::cout << "#      BO KIEM THU TOAN DIEN HE THONG ATM - NHANH FIX        #\n";
    std::cout << "#      Thuc hien boi: Thanh vien B (QA / Code Reviewer)      #\n";
    std::cout << "##############################################################\n";

    // Thực thi 6 bộ kiểm thử
    testSuiteCardModel();
    testSuiteAccountModel();
    testSuiteConsoleViewIO();
    testSuiteUserController();
    testSuiteStressPerformance();
    testSuiteMemoryAndInvariants();

    auto tTotalEnd = std::chrono::high_resolution_clock::now();
    double dTotalTimeMs = std::chrono::duration<double, std::milli>(tTotalEnd - tTotalStart).count();

    std::cout << "\n======================================================\n";
    std::cout << "              TONG KET KET QUA KIEM THU               \n";
    std::cout << "======================================================\n";
    std::cout << "  Tong so kiem thu thuc hien : " << g_nTotalTests << "\n";
    std::cout << "  So test THANH CONG [PASS]  : \033[32m" << g_nPassedTests << "\033[0m\n";
    std::cout << "  So test THAT BAI   [FAIL]  : \033[31m" << g_nFailedTests << "\033[0m\n";
    std::cout << "  Tong thoi gian chay        : " << std::fixed << std::setprecision(2)
              << dTotalTimeMs << " ms\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFailedTests == 0) {
        std::cout << "  \033[32m>>> KET LUAN: TAT CA CAC KIEM THU DEU DAT (100% PASS)! <<<\033[0m\n";
    } else {
        std::cout << "  \033[31m>>> KET LUAN: CO " << g_nFailedTests << " KIEM THU THAT BAI! <<<\033[0m\n";
    }
    std::cout << "======================================================\n\n";

    return (g_nFailedTests == 0) ? 0 : 1;
}
