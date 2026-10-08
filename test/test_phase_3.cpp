/******************************************************************************
 * @file: test_phase_3.cpp
 * @description: Bo kiem thu tich hop chuyen sau Phase 3 (Phan he Khach hang & Giao dich)
 *               Kiem thu toan dien luong dang nhap Khach hang, khoa the sau 3 lan sai,
 *               ep doi ma PIN mac dinh, nghiep vu Rut tien, Chuyen tien 2 chieu
 *               nguyen tu, va bao toan toan ven du lieu tap tin.
 *
 * Cach chay:
 *   make test_phase_3
 ******************************************************************************/

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <filesystem>
#include <fstream>

#include "Common.h"
#include "LinkedList.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "UserController.h"
#include "AtmController.h"

static int g_nPass = 0;
static int g_nFail = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            g_nPass++; \
            std::cout << "  \033[32m[PASS]\033[0m " << (msg) << "\n"; \
        } else { \
            g_nFail++; \
            std::cout << "  \033[31m[FAIL]\033[0m " << (msg) \
                      << " (Line: " << __LINE__ << ")\n"; \
        } \
    } while (0)

/******************************************************************************
 * 1. KIEM THU XAC THUC NGUOI DUNG & KHOA THE SAU 3 LAN SAI (PHASE 3)
 ******************************************************************************/
void testUserAuthenticationAndLockout() {
    std::cout << "\n======================================================\n";
    std::cout << " 1. XAC THUC NGUOI DUNG & KHOA THE SAU 3 LAN SAI (PHASE 3)\n";
    std::cout << "======================================================\n";

    Card testCard("10014504509901", "123456", false);
    bool bLocked = false;

    // Nhap dung PIN ngay lan dau
    bool bOk = UserController::authenticate(testCard, "123456", bLocked);
    TEST_ASSERT(bOk == true, "Dang nhap dung PIN tra ve true");
    TEST_ASSERT(bLocked == false, "The khong bi khoa sau dang nhap thanh cong");
    TEST_ASSERT(testCard.getFailedAttempts() == 0, "So lan nhap sai bang 0");

    // Nhap sai lan 1
    bOk = UserController::authenticate(testCard, "000000", bLocked);
    TEST_ASSERT(bOk == false, "Nhap sai PIN lan 1 tra ve false");
    TEST_ASSERT(testCard.getFailedAttempts() == 1, "So lan nhap sai la 1");
    TEST_ASSERT(testCard.isLocked() == false, "The chua bi khoa o lan 1");

    // Nhap sai lan 2
    bOk = UserController::authenticate(testCard, "111111", bLocked);
    TEST_ASSERT(bOk == false, "Nhap sai PIN lan 2 tra ve false");
    TEST_ASSERT(testCard.getFailedAttempts() == 2, "So lan nhap sai la 2");
    TEST_ASSERT(testCard.isLocked() == false, "The chua bi khoa o lan 2");

    // Nhap dung lai o lan 3: reset failed count
    bOk = UserController::authenticate(testCard, "123456", bLocked);
    TEST_ASSERT(bOk == true, "Dang nhap dung PIN reset chuoi sai");
    TEST_ASSERT(testCard.getFailedAttempts() == 0, "So lan sai duoc reset ve 0");

    // Nhap sai 3 lan lien tiep
    UserController::authenticate(testCard, "999999", bLocked);
    UserController::authenticate(testCard, "888888", bLocked);
    bOk = UserController::authenticate(testCard, "777777", bLocked);

    TEST_ASSERT(bOk == false, "Nhap sai PIN lan 3 lien tiep bi tu choi");
    TEST_ASSERT(bLocked == true, "Co bao khoa the bLocked bat len true");
    TEST_ASSERT(testCard.isLocked() == true, "The da bi KHOA hoan toan (isLocked == true)");

    // Lan thu 4 khi da bi khoa
    bOk = UserController::authenticate(testCard, "123456", bLocked);
    TEST_ASSERT(bOk == false, "The bi khoa khong cho phep dang nhap ke ca dung PIN");
    TEST_ASSERT(bLocked == true, "Trang thai the van la khoa");
}

/******************************************************************************
 * 2. KIEM THU EP BUOC DOI MA PIN MAC DINH (PHASE 3)
 ******************************************************************************/
void testDefaultPinEnforcement() {
    std::cout << "\n======================================================\n";
    std::cout << " 2. KIEM THU EP BUOC DOI MA PIN MAC DINH (PHASE 3)\n";
    std::cout << "======================================================\n";

    Card defaultCard("10014504509902", DEFAULT_PIN, false);
    TEST_ASSERT(defaultCard.isDefaultPin() == true, "The moi mang ma PIN mac dinh 123456");

    // Kiem tra dinh dang PIN
    TEST_ASSERT(Card::isValidPinFormat("123456") == true, "Ma 6 chu so la hop le");
    TEST_ASSERT(Card::isValidPinFormat("12345") == false, "Ma 5 chu so bi tu choi");
    TEST_ASSERT(Card::isValidPinFormat("1234567") == false, "Ma 7 chu so bi tu choi");
    TEST_ASSERT(Card::isValidPinFormat("12345a") == false, "Ma chua chu bi tu choi");

    // Doi sang PIN moi hop le
    bool bChanged = defaultCard.changePin("888999");
    TEST_ASSERT(bChanged == true, "Doi sang PIN 888999 thanh cong");
    TEST_ASSERT(defaultCard.getPin() == "888999", "Ma PIN moi la 888999");
    TEST_ASSERT(defaultCard.isDefaultPin() == false, "The khong con la PIN mac dinh");

    // Chan doi sang PIN rong hoac sai do dai
    TEST_ASSERT(defaultCard.changePin("") == false, "Chan doi PIN rong");
    TEST_ASSERT(defaultCard.changePin("1234") == false, "Chan doi PIN khong du 6 so");
}

/******************************************************************************
 * 3. KIEM THU RANG BUOC TAI CHINH RUT TIEN (PHASE 3)
 ******************************************************************************/
void testWithdrawalLogic() {
    std::cout << "\n======================================================\n";
    std::cout << " 3. KIEM THU RANG BUOC TAI CHINH RUT TIEN (PHASE 3)\n";
    std::cout << "======================================================\n";

    Account acc("10014504509903", "Test User Withdraw", 500000, "VND");

    // 1. So tien < 50,000 VND
    TEST_ASSERT(acc.canWithdraw(49000) == ERR_INVALID_AMOUNT, "Chan rut duoi 50k (ERR_INVALID_AMOUNT)");
    TEST_ASSERT(acc.canWithdraw(0) == ERR_INVALID_AMOUNT, "Chan rut 0 dong (ERR_INVALID_AMOUNT)");
    TEST_ASSERT(acc.canWithdraw(-50000) == ERR_INVALID_AMOUNT, "Chan rut so tien am (ERR_INVALID_AMOUNT)");

    // 2. So tien khong phai boi so cua 50,000 VND
    TEST_ASSERT(acc.canWithdraw(60000) == ERR_NOT_MULTIPLE, "Chan rut 60k (ERR_NOT_MULTIPLE)");
    TEST_ASSERT(acc.canWithdraw(125000) == ERR_NOT_MULTIPLE, "Chan rut 125k (ERR_NOT_MULTIPLE)");

    // 3. Khong giu lai du 50,000 VND so du toi thieu
    TEST_ASSERT(acc.canWithdraw(500000) == ERR_INSUFFICIENT_FUNDS, "Chan rut het 500k khong con du 50k (ERR_INSUFFICIENT_FUNDS)");
    TEST_ASSERT(acc.canWithdraw(460000) == ERR_NOT_MULTIPLE, "Chan rut 460k sai boi so");
    TEST_ASSERT(acc.canWithdraw(600000) == ERR_INSUFFICIENT_FUNDS, "Chan rut vuot qua so du");

    // 4. Rut tien hop le: toi da 450,000 VND
    TEST_ASSERT(acc.canWithdraw(450000) == ERR_NONE, "Chap nhan rut 450k (con lai 50k)");

    // Thuc hien rut tien that
    bool bWithdrawn = acc.withdraw(200000);
    TEST_ASSERT(bWithdrawn == true, "Rut 200,000 VND thanh cong");
    TEST_ASSERT(acc.getBalance() == 300000, "So du con lai chinh xac la 300,000 VND");

    // Rut them 250,000 VND (con lai 50k)
    bWithdrawn = acc.withdraw(250000);
    TEST_ASSERT(bWithdrawn == true, "Rut them 250,000 VND thanh cong");
    TEST_ASSERT(acc.getBalance() == 50000, "So du con lai dung 50,000 VND");

    // Rut tiep 50,000 VND -> Bi chan vi can giu lai 50k
    TEST_ASSERT(acc.canWithdraw(50000) == ERR_INSUFFICIENT_FUNDS, "Chan rut tiep khi chi con 50k");
}

/******************************************************************************
 * 4. KIEM THU CHUYEN TIEN NGUYEN TU 2 CHIEU (PHASE 3)
 ******************************************************************************/
void testAtomicTransferLogic() {
    std::cout << "\n======================================================\n";
    std::cout << " 4. KIEM THU CHUYEN TIEN NGUYEN TU 2 CHIEU (PHASE 3)\n";
    std::cout << "======================================================\n";

    Account senderAcc("10014504509904", "Sender Nguyen", 1000000, "VND");
    Account receiverAcc("10014504509905", "Receiver Tran", 500000, "VND");
    Account usdAcc("10014504509906", "USD User", 1000, "USD");

    long lTotalBefore = senderAcc.getBalance() + receiverAcc.getBalance();

    // 1. Chuyen khac don vi tien te
    ErrorCode errDiffCurrency = UserController::processTransfer(senderAcc, usdAcc, 100000);
    TEST_ASSERT(errDiffCurrency == ERR_INVALID_FORMAT, "Chan chuyen tien khac loai tien te (VND -> USD)");
    TEST_ASSERT(senderAcc.getBalance() == 1000000, "So du nguoi gui khong doi");
    TEST_ASSERT(usdAcc.getBalance() == 1000, "So du tai khoan USD khong doi");

    // 2. Chuyen so tien sai boi so hoac qua han muc
    TEST_ASSERT(UserController::processTransfer(senderAcc, receiverAcc, 75000) == ERR_NOT_MULTIPLE, "Chan chuyen 75k khong chia het 50k");
    TEST_ASSERT(UserController::processTransfer(senderAcc, receiverAcc, 1000000) == ERR_INSUFFICIENT_FUNDS, "Chan chuyen vuot han muc du tru 50k");

    // 3. Chuyen tien hop le: 300,000 VND
    ErrorCode errTransfer = UserController::processTransfer(senderAcc, receiverAcc, 300000);
    TEST_ASSERT(errTransfer == ERR_NONE, "Chuyen tien hop le thanh cong (ERR_NONE)");
    TEST_ASSERT(senderAcc.getBalance() == 700000, "Nguoi gui bi tru dung 300,000 VND (con 700k)");
    TEST_ASSERT(receiverAcc.getBalance() == 800000, "Nguoi nhan duoc cong dung 300,000 VND (len 800k)");

    long lTotalAfter = senderAcc.getBalance() + receiverAcc.getBalance();
    TEST_ASSERT(lTotalBefore == lTotalAfter, "Tong tien giua 2 tai khoan duoc bao toan tuyet doi");

    // 4. Kiem tra luu tru tap tin va ghi log
    FileService::saveAccount(senderAcc);
    FileService::saveAccount(receiverAcc);

    std::string strTime = getNowTimestamp();
    Transaction txSender(senderAcc.getId(), TRANSFER, 300000, strTime, "Chuyen den " + receiverAcc.getId());
    Transaction txReceiver(receiverAcc.getId(), RECEIVE, 300000, strTime, "Nhan tu " + senderAcc.getId());

    FileService::appendTransaction(senderAcc.getId(), txSender);
    FileService::appendTransaction(receiverAcc.getId(), txReceiver);

    LinkedList<Transaction> listSenderTx;
    FileService::loadTransactions(senderAcc.getId(), listSenderTx);
    TEST_ASSERT(!listSenderTx.isEmpty(), "File LichSu nguoi gui da ghi nhan giao dich");

    LinkedList<Transaction> listReceiverTx;
    FileService::loadTransactions(receiverAcc.getId(), listReceiverTx);
    TEST_ASSERT(!listReceiverTx.isEmpty(), "File LichSu nguoi nhan da ghi nhan giao dich");
}

/******************************************************************************
 * 5. KIEM THU DOI PIN CHU DONG & HUY AN TOAN (PHASE 3)
 ******************************************************************************/
void testChangePinAndCancellation() {
    std::cout << "\n======================================================\n";
    std::cout << " 5. KIEM THU DOI PIN CHU DONG & HUY AN TOAN (PHASE 3)\n";
    std::cout << "======================================================\n";

    Card userCard("10014504509907", "654321", false);
    std::string strMsg;

    // Sai PIN cu
    bool bOk = UserController::processChangePin(userCard, "111111", "999999", "999999", strMsg);
    TEST_ASSERT(bOk == false, "Sai PIN cu bi tu choi");
    TEST_ASSERT(userCard.getPin() == "654321", "PIN van duoc giu nguyen 654321");

    // Hai lan nhap PIN moi khong khop
    bOk = UserController::processChangePin(userCard, "654321", "999999", "888888", strMsg);
    TEST_ASSERT(bOk == false, "Hai lan nhap PIN moi khac nhau bi tu choi");

    // PIN moi trung PIN mac dinh 123456
    bOk = UserController::processChangePin(userCard, "654321", DEFAULT_PIN, DEFAULT_PIN, strMsg);
    TEST_ASSERT(bOk == false, "PIN moi trung voi DEFAULT_PIN bi chan");

    // PIN moi trung PIN cu
    bOk = UserController::processChangePin(userCard, "654321", "654321", "654321", strMsg);
    TEST_ASSERT(bOk == false, "PIN moi trung voi PIN cu bi chan");

    // Doi PIN hop le
    bOk = UserController::processChangePin(userCard, "654321", "987654", "987654", strMsg);
    TEST_ASSERT(bOk == true, "Doi PIN hop le thanh cong");
    TEST_ASSERT(userCard.getPin() == "987654", "PIN moi duoc cap nhat vao the");
}

/******************************************************************************
 * HAM CHAY CHINH CHO TEST SUITE PHASE 3
 ******************************************************************************/
int main() {
    std::cout << "##############################################################\n";
    std::cout << "#     CHUONG TRINH KIEM THU TICH HOP PHASE 3 (USER & TRANS)  #\n";
    std::cout << "##############################################################\n";

    FileService::initSampleData();

    testUserAuthenticationAndLockout();
    testDefaultPinEnforcement();
    testWithdrawalLogic();
    testAtomicTransferLogic();
    testChangePinAndCancellation();

    std::cout << "\n======================================================\n";
    std::cout << "             TONG KET KIEM THU PHASE 3                \n";
    std::cout << "======================================================\n";
    std::cout << "  So test THANH CONG [PASS]: " << g_nPass << "\n";
    std::cout << "  So test THAT BAI   [FAIL]: " << g_nFail << "\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFail == 0) {
        std::cout << "  >>> KET LUAN: 100% KIEM THU PHASE 3 DA DAT CHUAN! <<<\n";
    } else {
        std::cout << "  >>> KET LUAN: CO " << g_nFail << " TEST CASE BI LOI! <<<\n";
    }
    // Don dep cac tap tin test sinh ra trong qua trinh kiem thu
    std::filesystem::remove("data/10014504509904.txt");
    std::filesystem::remove("data/10014504509905.txt");
    std::filesystem::remove("data/LichSu10014504509904.txt");
    std::filesystem::remove("data/LichSu10014504509905.txt");

    return (g_nFail == 0) ? 0 : 1;
}
