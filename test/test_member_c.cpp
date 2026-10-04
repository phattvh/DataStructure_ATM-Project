#include "Account.h"
#include "Card.h"
#include "Common.h"
#include "ConsoleView.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <limits>

/**********************************************************
 * @Description: Macro kiem thu doc lap, khong bi loai bo boi -DNDEBUG
 **********************************************************/
#define TEST_CHECK(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "\n[TEST FAILED] " << message \
                      << "\n  File: " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::exit(1); \
        } \
    } while (0)

/**********************************************************
 * @Description Kiem thu toan dien Model Card
 * Bao gom ca Happy Path va cac Edge Cases (PIN sai dinh dang, mo khoa)
 **********************************************************/
void testCardModel() {
    std::cout << "[RUNNING] testCardModel...\n";

    // 1. Khoi tao mac dinh
    Card cardDefault;
    TEST_CHECK(cardDefault.getId() == "", "Card default ID phai rong");
    TEST_CHECK(cardDefault.getPin() == DEFAULT_PIN, "Card default PIN phai la 123456");
    TEST_CHECK(cardDefault.isDefaultPin() == true, "Card default PIN phai nhan dien la true");
    TEST_CHECK(cardDefault.isLocked() == false, "Card mac dinh khong duoc bi khoa");
    TEST_CHECK(cardDefault.getFailedAttempts() == 0, "So lan sai ban dau phai bang 0");

    // 2. Khoi tao co tham so
    Card cardUser("10014504500001", "654321", false);
    TEST_CHECK(cardUser.getId() == "10014504500001", "ID khoi tao phai dung");
    TEST_CHECK(cardUser.getPin() == "654321", "PIN khoi tao phai dung");
    TEST_CHECK(cardUser.isDefaultPin() == false, "PIN 654321 khong phai PIN mac dinh");
    TEST_CHECK(cardUser.checkPin("654321") == true, "checkPin dung phai tra ve true");
    TEST_CHECK(cardUser.checkPin("000000") == false, "checkPin sai phai tra ve false");

    // 3. Kiem tra co che dem sai va tu dong khoa the
    cardUser.recordFailedAttempt();
    TEST_CHECK(cardUser.getFailedAttempts() == 1, "Failed attempts phai bang 1");
    TEST_CHECK(cardUser.isLocked() == false, "The chua bi khoa o lan 1");

    cardUser.recordFailedAttempt();
    TEST_CHECK(cardUser.getFailedAttempts() == 2, "Failed attempts phai bang 2");
    TEST_CHECK(cardUser.isLocked() == false, "The chua bi khoa o lan 2");

    cardUser.recordFailedAttempt(); // Lan thu 3
    TEST_CHECK(cardUser.getFailedAttempts() == 3, "Failed attempts phai bang 3");
    TEST_CHECK(cardUser.isLocked() == true, "The phai bi khoa o lan 3");

    // Goi them lan nua khi da khoa: khong duoc tang vo han
    cardUser.recordFailedAttempt();
    TEST_CHECK(cardUser.isLocked() == true, "The van phai bi khoa");

    // 4. Kiem tra tach biet giua resetFailedAttempts va unlockCard (nguyen ly SRP)
    cardUser.resetFailedAttempts();
    TEST_CHECK(cardUser.getFailedAttempts() == 0, "Reset phai ve 0");
    TEST_CHECK(cardUser.isLocked() == true, "resetFailedAttempts KHONG duoc tu tien mo khoa the!");

    cardUser.unlockCard();
    TEST_CHECK(cardUser.isLocked() == false, "unlockCard phai mo khoa the thanh cong");
    TEST_CHECK(cardUser.getFailedAttempts() == 0, "unlockCard phai reset so lan sai ve 0");

    // 5. Kiem tra tinh chat bao ve du lieu (Validation) cua changePin
    // 5.1. Thu doi PIN sai do dai (< 6 ky tu)
    TEST_CHECK(cardUser.changePin("12345") == false, "PIN 5 ky tu phai bi tu choi");
    TEST_CHECK(cardUser.getPin() == "654321", "PIN khong duoc thay doi khi nhap sai");

    // 5.2. Thu doi PIN qua dai (> 6 ky tu)
    TEST_CHECK(cardUser.changePin("1234567") == false, "PIN 7 ky tu phai bi tu choi");

    // 5.3. Thu doi PIN chua chu cai hoac ky tu dac biet
    TEST_CHECK(cardUser.changePin("12a456") == false, "PIN chua chu cai phai bi tu choi");
    TEST_CHECK(cardUser.changePin("12 456") == false, "PIN chua dau cach phai bi tu choi");
    TEST_CHECK(cardUser.changePin("") == false, "PIN rong phai bi tu choi");

    // 5.4. Doi PIN hop le (dung 6 chu so)
    TEST_CHECK(cardUser.changePin("999888") == true, "PIN 6 so hop le phai thanh cong");
    TEST_CHECK(cardUser.getPin() == "999888", "PIN da duoc cap nhat dung");

    std::cout << "[PASSED] testCardModel\n";
}

/**********************************************************
 * @Description Kiem thu toan dien Model Account
 * Bao gom chan so am trong deposit, chan rut vuot so du, chan tran so
 **********************************************************/
void testAccountModel() {
    std::cout << "[RUNNING] testAccountModel...\n";

    Account acc("10014504500001", "Nguyen Trung Kien", 200000, "VND");
    TEST_CHECK(acc.getId() == "10014504500001", "ID account phai dung");
    TEST_CHECK(acc.getName() == "Nguyen Trung Kien", "Ten chu the phai dung");
    TEST_CHECK(acc.getBalance() == 200000, "So du ban dau phai bang 200k");
    TEST_CHECK(acc.getCurrency() == "VND", "Tien te phai la VND");

    // 1. Kiem tra cac ma loi canWithdraw
    // Rut duoi 50.000 VND
    TEST_CHECK(acc.canWithdraw(30000) == ERR_INVALID_AMOUNT, "Rut 30k phai bao ERR_INVALID_AMOUNT");
    // Rut khong chia het cho 50.000 VND
    TEST_CHECK(acc.canWithdraw(75000) == ERR_NOT_MULTIPLE, "Rut 75k phai bao ERR_NOT_MULTIPLE");
    // Rut lam so du con lai < 50k
    TEST_CHECK(acc.canWithdraw(200000) == ERR_INSUFFICIENT_FUNDS, "Rut 200k phai bao ERR_INSUFFICIENT_FUNDS");

    // 2. Kiem tra withdraw() co tich hop canWithdraw
    // Thu rut khong hop le -> phai tra ve false, so du khong doi
    TEST_CHECK(acc.withdraw(200000) == false, "withdraw 200k phai tra ve false");
    TEST_CHECK(acc.getBalance() == 200000, "So du khong duoc bi tru khi rut loi");

    // Rut hop le 100.000 VND
    TEST_CHECK(acc.withdraw(100000) == true, "withdraw 100k phai thanh cong");
    TEST_CHECK(acc.getBalance() == 100000, "So du con lai phai la 100k");

    // 3. Kiem tra bao ve toan ven du lieu trong deposit()
    // 3.1. Chan tuyet doi so tien nap am (Loi backdoor)
    TEST_CHECK(acc.deposit(-50000) == false, "deposit so am phai tra ve false");
    TEST_CHECK(acc.getBalance() == 100000, "So du khong duoc thay doi khi nap so am");

    // 3.2. Chan nap 0 dong
    TEST_CHECK(acc.deposit(0) == false, "deposit 0 dong phai tra ve false");

    // 3.3. Nap tien hop le
    TEST_CHECK(acc.deposit(300000) == true, "deposit 300k hop le phai thanh cong");
    TEST_CHECK(acc.getBalance() == 400000, "So du phai tang len 400k");

    // 3.4. Chong tran so nguyen (Overflow protection)
    TEST_CHECK(acc.deposit(std::numeric_limits<long>::max()) == false, "deposit tran so phai tra ve false");

    std::cout << "[PASSED] testAccountModel\n";
}

/**********************************************************
 * @Description Kiem thu tu dong logic xu ly I/O cua ConsoleView
 * bang cach tiem gia lap luong stringstream (khong can nguoi go phim)
 **********************************************************/
void testConsoleViewIO() {
    std::cout << "[RUNNING] testConsoleViewIO (Automated Stream Testing)...\n";

    // 1. Kiem thu inputMoney chong chuoi ky tu, so thuc, so am, va chap nhan so hop le
    // Chuoi gia lap: "abc\n-50000\n50000.75\n100000abc\n250000\n"
    std::istringstream streamMoney("abc\n-50000\n50000.75\n100000abc\n250000\n");
    long lMoney = ConsoleView::inputMoney("Nhap tien test: ", streamMoney);
    TEST_CHECK(lMoney == 250000, "inputMoney phai loai bo cac du lieu loi va chi lay 250000");

    // 2. Kiem thu inputMoney xu ly EOF (Ctrl+D) khong bi treo loop vo han
    std::istringstream streamEmpty("");
    long lEofResult = ConsoleView::inputMoney("Nhap tien EOF test: ", streamEmpty);
    TEST_CHECK(lEofResult == 0, "inputMoney khi gap EOF phai tra ve 0 ngay lap tuc ma khong treo");

    // 3. Kiem thu inputPassword qua stream injection
    std::istringstream streamPass("888999\n");
    std::string strPass = ConsoleView::inputPassword("Nhap PIN test: ", &streamPass);
    TEST_CHECK(strPass == "888999", "inputPassword qua stream phai doc dung 888999");

    std::cout << "[PASSED] testConsoleViewIO\n";
}

/**********************************************************
 * @Description Kiem thu cac giao dien xuat man hinh
 **********************************************************/
void testConsoleViewLayout() {
    std::cout << "[RUNNING] testConsoleViewLayout...\n";

    ConsoleView::printHeader("TEST CONSOLE VIEW TIÊU CHUẨN");
    ConsoleView::printSuccess("Kiem thu thong bao thanh cong");
    ConsoleView::printWarning("Kiem thu thong bao canh bao");
    ConsoleView::printError("Kiem thu thong bao loi");

    // Kiem tra ca Menu Admin va Menu User (nhiem vu cua Member C)
    ConsoleView::printAdminMenu();
    ConsoleView::printUserMenu();

    ConsoleView::printCardTableHeader();
    ConsoleView::printCardRow("10014504500001", "123456", false);
    ConsoleView::printCardRow("10014504500002", "888888", true);
    ConsoleView::printCardTableFooter();

    ConsoleView::printReceipt("10014504500001", "RUT TIEN", 100000, 400000, "2026-10-04 12:00:00");

    Account acc("10014504500001", "Nguyen Trung Kien", 500000, "VND");
    ConsoleView::displayAccountInfo(acc);
    ConsoleView::displayAccountDetails("10014504500002", "Tran Van B", 350000, "VND");

    std::cout << "[PASSED] testConsoleViewLayout\n";
}

int main() {
    std::cout << "==========================================================\n";
    std::cout << "  BO KIEM THU TU DONG CHAT LUONG CAO - MEMBER C (TUAN 1)  \n";
    std::cout << "  (Kiem thu ca Happy Path, Edge Cases & I/O Streams)      \n";
    std::cout << "==========================================================\n";

    testCardModel();
    testAccountModel();
    testConsoleViewIO();
    testConsoleViewLayout();

    std::cout << "\n>>> 100% KIEM THU DA VUOT QUA CHUAN XAC TUYET DOI! <<<\n";
    return 0;
}
