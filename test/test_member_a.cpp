#include <iostream>
#include <limits>
#include "Common.h"
#include "Card.h"
#include "Account.h"
#include "ConsoleView.h"
#include "UserController.h"

// Macro kiem tra dieu kien test
#define ASSERT_TEST(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\033[31m[FAIL] " << msg << "\033[0m\n"; \
            return false; \
        } else { \
            std::cout << "\033[32m[PASS] " << msg << "\033[0m\n"; \
        } \
    } while (0)

bool testCardModel() {
    std::cout << "\n=== KIEM THU 1: MODEL CARD (MEMBER A) ===\n";
    Card card("10014504500001", "123456");
    ASSERT_TEST(card.getId() == "10014504500001", "Khoi tao dung ID the");
    ASSERT_TEST(card.getPin() == "123456", "Khoi tao dung PIN ban dau");
    ASSERT_TEST(card.isDefaultPin() == true, "Phat hien ma PIN mac dinh 123456");
    ASSERT_TEST(!card.isLocked(), "The moi khoi tao o trang thai Mo");
    ASSERT_TEST(card.getFailedAttempts() == 0, "So lan sai ban dau bang 0");

    ASSERT_TEST(card.checkPin("123456"), "Kiem tra dung ma PIN");
    ASSERT_TEST(!card.checkPin("999999"), "Kiem tra sai ma PIN");

    // Test dem sai va khoa the
    card.recordFailedAttempt();
    ASSERT_TEST(card.getFailedAttempts() == 1 && !card.isLocked(), "Sai 1 lan chua bi khoa");
    card.recordFailedAttempt();
    ASSERT_TEST(card.getFailedAttempts() == 2 && !card.isLocked(), "Sai 2 lan chua bi khoa");
    card.recordFailedAttempt();
    ASSERT_TEST(card.getFailedAttempts() == 3 && card.isLocked(), "Sai 3 lan tu dong khoa the");

    // Test reset va doi PIN (dung unlockCard chuyen biet theo nguyen ly SRP)
    card.unlockCard();
    ASSERT_TEST(card.getFailedAttempts() == 0 && !card.isLocked(), "Mo khoa the va reset so lan sai thanh cong");
    card.changePin("888888");
    ASSERT_TEST(card.getPin() == "888888", "Doi ma PIN thanh cong");
    ASSERT_TEST(!card.isDefaultPin(), "The khong con mang PIN mac dinh");

    return true;
}

bool testAccountModel() {
    std::cout << "\n=== KIEM THU 2: MODEL ACCOUNT & RANG BUOC TAI CHINH (MEMBER A) ===\n";
    Account acc("10014504500001", "NGUYEN VAN A", 500000, "VND");
    ASSERT_TEST(acc.getBalance() == 500000, "So du ban dau 500,000 VND");

    // 1. So tien < 50,000 VND
    ASSERT_TEST(acc.canWithdraw(20000) == ERR_INVALID_AMOUNT, "Chan rut duoi 50,000 VND (20k -> ERR_INVALID_AMOUNT)");

    // 2. So tien khong phai boi so 50,000 VND
    ASSERT_TEST(acc.canWithdraw(75000) == ERR_NOT_MULTIPLE, "Chan rut khong phai boi so 50k (75k -> ERR_NOT_MULTIPLE)");

    // 3. Vi pham so du toi thieu duy tri (50,000 VND)
    ASSERT_TEST(acc.canWithdraw(480000) == ERR_NOT_MULTIPLE, "480k khong phai boi so 50k");
    ASSERT_TEST(acc.canWithdraw(500000) == ERR_INSUFFICIENT_FUNDS, "Rut het 500k de lai so du < 50k -> ERR_INSUFFICIENT_FUNDS");
    ASSERT_TEST(acc.canWithdraw(460000) == ERR_NOT_MULTIPLE, "460k khong phai boi so 50k");
    ASSERT_TEST(acc.canWithdraw(450000) == ERR_NONE, "Rut 450k giu lai dung 50k duy tri hop le -> ERR_NONE");

    // 4. Thuc hien rut tien va nap tien
    acc.withdraw(200000);
    ASSERT_TEST(acc.getBalance() == 300000, "Rut 200,000 con lai 300,000 VND");
    acc.deposit(100000);
    ASSERT_TEST(acc.getBalance() == 400000, "Nap 100,000 so du len 400,000 VND");

    return true;
}

bool testUserControllerLogic() {
    std::cout << "\n=== KIEM THU 3: NGHIEP VU PHAN HE USER (USERCONTROLLER) ===\n";
    Card card("10014504500002", "123456");
    Account sender("10014504500002", "TRAN THI B", 1000000, "VND");
    Account receiver("10014504500003", "LE VAN C", 200000, "VND");

    // 1. Kiem tra dinh dang ID & PIN
    ASSERT_TEST(UserController::isValidIdFormat("10014504500002"), "ID 14 chu so hop le");
    ASSERT_TEST(!UserController::isValidIdFormat("123"), "ID 3 so khong hop le");
    ASSERT_TEST(UserController::isValidPinFormat("123456"), "PIN 6 so hop le");
    ASSERT_TEST(!UserController::isValidPinFormat("1234a6"), "PIN chua chu cai khong hop le");

    // 2. Test dang nhap & khoa the qua UserController
    bool bLocked = false;
    ASSERT_TEST(!UserController::authenticate(card, "000000", bLocked), "Nhap sai PIN khong xac thuc duoc");
    ASSERT_TEST(!bLocked, "Moi sai 1 lan chua khoa");
    UserController::authenticate(card, "000000", bLocked);
    UserController::authenticate(card, "000000", bLocked);
    ASSERT_TEST(bLocked && card.isLocked(), "Sai PIN 3 lan: UserController bao khoa the thanh cong");

    card.unlockCard();
    ASSERT_TEST(UserController::authenticate(card, "123456", bLocked), "Nhap dung PIN xac thuc thanh cong");

    // 3. Test chuyen tien nguyen tu
    ErrorCode errSame = UserController::processTransfer(sender, sender, 100000);
    ASSERT_TEST(errSame == ERR_SAME_ACCOUNT, "Chan chuyen tien cho chinh minh -> ERR_SAME_ACCOUNT");

    ErrorCode errTransfer = UserController::processTransfer(sender, receiver, 300000);
    ASSERT_TEST(errTransfer == ERR_NONE, "Chuyen 300,000 VND thanh cong");
    ASSERT_TEST(sender.getBalance() == 700000, "Nguoi gui con 700,000 VND");
    ASSERT_TEST(receiver.getBalance() == 500000, "Nguoi nhan tang len 500,000 VND");

    // 4. Test co che Rollback dam bao nguyen tu khi nguoi nhan bi loi
    Account overflowAcc("10014504500004", "TRAN OVERFLOW", std::numeric_limits<long>::max() - 10000, "VND");
    long lSenderBefore = sender.getBalance();
    ErrorCode errOverflow = UserController::processTransfer(sender, overflowAcc, 50000);
    ASSERT_TEST(errOverflow == ERR_SYSTEM_OVERFLOW, "Phat hien tran so nguoi nhan -> ERR_SYSTEM_OVERFLOW");
    ASSERT_TEST(sender.getBalance() == lSenderBefore, "Rollback thanh cong: So du nguoi gui duoc bao toan nguyen ven");

    // 5. Test doi PIN
    std::string strMsg;
    bool bChangeFail = UserController::processChangePin(card, "wrongpin", "654321", "654321", strMsg);
    ASSERT_TEST(!bChangeFail, "Doi PIN that bai khi nhap sai PIN cu");

    bool bChangeMismatch = UserController::processChangePin(card, "123456", "654321", "111222", strMsg);
    ASSERT_TEST(!bChangeMismatch, "Doi PIN that bai khi xac nhan khong khop");

    bool bChangeOk = UserController::processChangePin(card, "123456", "654321", "654321", strMsg);
    ASSERT_TEST(bChangeOk && card.getPin() == "654321", "Doi PIN thanh cong voi day du xac nhan");

    return true;
}

int main() {
    ConsoleView::printHeader("TEST SUITE: PHAN HE MEMBER A (BUSINESS & UI)");
    bool bAllPassed = true;
    bAllPassed &= testCardModel();
    bAllPassed &= testAccountModel();
    bAllPassed &= testUserControllerLogic();

    std::cout << "\n======================================================\n";
    if (bAllPassed) {
        ConsoleView::printSuccess("TAT CA CAC KIEM THU PHAN HE MEMBER A DEU DAT (100% PASS)!");
    } else {
        ConsoleView::printError("CO MOT SO KIEM THU CHUA DAT!");
    }
    std::cout << "======================================================\n";
    return bAllPassed ? 0 : 1;
}
