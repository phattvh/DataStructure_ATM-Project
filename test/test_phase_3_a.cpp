/******************************************************************************
 * @file: test_phase_3_a.cpp
 * @description: Bo kiem thu chuyen sau Phase 3 danh cho Thanh vien A (Member A - Tuan)
 *               Nhiem vu:
 *                 - A08: Va loi #1 (Bo tru tien ao, nap tai khoan nhan that tu FileService)
 *                 - A09: Va loi #2 (Format thoi gian thuc tren bien lai va giao dich)
 *                 - A10: Ket noi FileService vao Rut tien (cap nhat [ID].txt va LichSu[ID].txt)
 *                 - A11: Ket noi FileService vao Doi PIN (TheTu.txt) va Xem lich su giao dich
 *                 - B10: Pair-programming Tuan & Tri (Chuyen tien nguyen tu 2 dau tren dia)
 *
 * Cach chay:
 *   g++ -std=c++17 -Wall -Wextra -Iinclude src/ConsoleView.cpp src/Card.cpp src/Account.cpp \
 *       src/UserController.cpp src/Admin.cpp src/Transaction.cpp src/FileService.cpp \
 *       src/AtmController.cpp src/AdminController.cpp test/test_phase_3_a.cpp -o build/test_phase_3_a
 *   ./build/test_phase_3_a
 ******************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <filesystem>
#include <limits>

#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "UserController.h"
#include "ConsoleView.h"

namespace fs = std::filesystem;

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

static void resetSampleData() {
    FileService::initSampleData();

    struct SampleCard {
        const char* szId;
        const char* szPin;
        const char* szName;
        long lBalance;
    };

    SampleCard sampleCards[] = {
        {"10014504500001", "123456", "Nguyen Trung Kien", 5000000},
        {"10014504500002", "123456", "Tran Thi Hoa",     10000000},
        {"10014504500003", "654321", "Le Van Cuong",      2500000},
        {"10014504500004", "123456", "Pham Minh Duc",      500000},
        {"10014504500005", "888888", "Hoang Quoc Bao",   12000000},
        {"10014504500006", "123456", "Vo Thi Mai",         800000},
        {"10014504500007", "123456", "Dang Tuan Anh",     3000000},
        {"10014504500008", "123456", "Bui Thi Lan",       1500000},
        {"10014504500009", "123456", "Doan Ngoc Hai",     7200000},
        {"10014504500010", "123456", "Truong Gia Binh",  20000000}
    };

    std::string strTheTuPath = DATA_DIR + "TheTu.txt";
    std::ofstream foutTheTu(strTheTuPath, std::ios::trunc);
    if (foutTheTu.is_open()) {
        for (const auto& card : sampleCards) {
            foutTheTu << card.szId << " " << card.szPin << "\n";
            FileService::createAccountFiles(card.szId, card.szName, card.lBalance, "VND");
        }
        foutTheTu.close();
    }

    LinkedList<std::string> emptyLocked;
    FileService::saveLockedIds(emptyLocked);
}

/******************************************************************************
 * 1. KIEM THU A10: RUT TIEN KET NOI FILESERVICE & LICH SU GIAO DICH
 ******************************************************************************/
void testWithdrawWithPersistence() {
    std::cout << "\n======================================================\n";
    std::cout << " 1. KIEM THU RUT TIEN KET NOI FILESERVICE (A10)\n";
    std::cout << "======================================================\n";

    resetSampleData();

    const std::string strId = "10014504500001";
    Account acc;
    ErrorCode errLoad = FileService::loadAccount(strId, acc);
    TEST_ASSERT(errLoad == ERR_NONE, "Nap thanh cong tai khoan 10014504500001 tu dia");

    long lBalanceBefore = acc.getBalance();
    long lWithdrawAmount = 500000; // 500k hop le

    // 1.1 Rut tien hop le
    std::string strTimestamp;
    ErrorCode errWithdraw = UserController::processWithdrawAndPersist(acc, lWithdrawAmount, strTimestamp);
    TEST_ASSERT(errWithdraw == ERR_NONE, "Rut 500,000 VND hop le tra ve ERR_NONE");
    TEST_ASSERT(acc.getBalance() == lBalanceBefore - lWithdrawAmount, "So du trong RAM giam dung 500,000 VND");
    TEST_ASSERT(!strTimestamp.empty(), "Chuoi thoi gian thuc cua giao dich duoc sinh ra");

    // 1.2 Kiem tra ghi ben vung tren dia ([ID].txt)
    Account accOnDisk;
    FileService::loadAccount(strId, accOnDisk);
    TEST_ASSERT(accOnDisk.getBalance() == acc.getBalance(), "So du trong file [ID].txt tren dia khop voi RAM");

    // 1.3 Kiem tra ghi lich su giao dich (LichSu[ID].txt)
    LinkedList<Transaction> listTrans;
    bool bLoadedTrans = FileService::loadTransactions(strId, listTrans);
    TEST_ASSERT(bLoadedTrans, "Doc thanh cong file LichSu[ID].txt tu dia");
    TEST_ASSERT(!listTrans.isEmpty(), "File LichSu[ID].txt da co du lieu ghi nhan");

    const Transaction* pLastTx = nullptr;
    auto pCur = listTrans.getHead();
    while (pCur != nullptr) {
        pLastTx = &(pCur->_data);
        pCur = pCur->_pNext;
    }
    TEST_ASSERT(pLastTx != nullptr, "Tim thay giao dich cuoi cung vua ghi");
    TEST_ASSERT(pLastTx != nullptr && pLastTx->getType() == WITHDRAW, "Loai giao dich cuoi cung la WITHDRAW");
    TEST_ASSERT(pLastTx != nullptr && pLastTx->getAmount() == lWithdrawAmount, "So tien ghi trong lich su dung 500,000 VND");

    // 1.4 Cac truong hop vi pham rang buoc (Khong duoc thay doi file hay ghi log)
    int iTransCountBefore = listTrans.getSize();

    // Rut duoi 50k
    ErrorCode errMin = UserController::processWithdrawAndPersist(acc, 20000, strTimestamp);
    TEST_ASSERT(errMin == ERR_INVALID_AMOUNT, "Chan rut duoi 50,000 VND (ERR_INVALID_AMOUNT)");

    // Rut khong phai boi so 50k
    ErrorCode errMult = UserController::processWithdrawAndPersist(acc, 75000, strTimestamp);
    TEST_ASSERT(errMult == ERR_NOT_MULTIPLE, "Chan rut khong phai boi so 50,000 VND (ERR_NOT_MULTIPLE)");

    // Rut qua so du duy tri
    long lOverAmount = acc.getBalance(); // De lai 0 VND < 50,000 VND
    ErrorCode errFunds = UserController::processWithdrawAndPersist(acc, lOverAmount, strTimestamp);
    TEST_ASSERT(errFunds == ERR_INSUFFICIENT_FUNDS, "Chan rut vi pham so du duy tri 50,000 VND (ERR_INSUFFICIENT_FUNDS)");

    // Kiem tra khong bi ghi log rac khi giao dich that bai
    LinkedList<Transaction> listTransAfterFail;
    FileService::loadTransactions(strId, listTransAfterFail);
    TEST_ASSERT(listTransAfterFail.getSize() == iTransCountBefore, "Giao dich vi pham khong bi ghi log rac vao LichSu[ID].txt");
}

/******************************************************************************
 * 2. KIEM THU A08 & B10: CHUYEN TIEN NGUYEN TU KET NOI FILESERVICE 2 DAU
 ******************************************************************************/
void testTransferWithPersistence() {
    std::cout << "\n======================================================\n";
    std::cout << " 2. KIEM THU CHUYEN TIEN NGUYEN TU 2 DAU (A08 & B10)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    const std::string strSenderId   = "10014504500002"; // Co 5,000,000 VND
    const std::string strReceiverId = "10014504500003"; // Co 5,000,000 VND

    Account senderAcc, receiverAcc;
    FileService::loadAccount(strSenderId, senderAcc);
    FileService::loadAccount(strReceiverId, receiverAcc);

    long lSenderBalBefore   = senderAcc.getBalance();
    long lReceiverBalBefore = receiverAcc.getBalance();
    long lTransferAmount    = 1000000; // 1,000,000 VND

    // 2.1 Chuyen tien thanh cong giua 2 tai khoan thuc
    std::string strTimestamp;
    ErrorCode errTransfer = UserController::processTransferAndPersist(senderAcc, strReceiverId, lTransferAmount, strTimestamp);
    TEST_ASSERT(errTransfer == ERR_NONE, "Chuyen 1,000,000 VND giua 2 tai khoan thuc tra ve ERR_NONE");
    TEST_ASSERT(senderAcc.getBalance() == lSenderBalBefore - lTransferAmount, "So du nguoi gui trong RAM giam 1,000,000 VND");

    // 2.2 Kiem tra ghi ben vung tren dia cua ca 2 file tai khoan
    Account senderOnDisk, receiverOnDisk;
    FileService::loadAccount(strSenderId, senderOnDisk);
    FileService::loadAccount(strReceiverId, receiverOnDisk);
    TEST_ASSERT(senderOnDisk.getBalance() == lSenderBalBefore - lTransferAmount, "File nguoi gui tren dia cap nhat dung so du");
    TEST_ASSERT(receiverOnDisk.getBalance() == lReceiverBalBefore + lTransferAmount, "File nguoi nhan tren dia cap nhat dung so du");
    TEST_ASSERT(senderOnDisk.getBalance() + receiverOnDisk.getBalance() == lSenderBalBefore + lReceiverBalBefore,
                "Tong so du he thong duoc bao toan tuyet doi (Khong mat tien)");

    // 2.3 Kiem tra ghi 2 file lich su: nguoi gui (TRANSFER) va nguoi nhan (RECEIVE)
    LinkedList<Transaction> senderLogs, receiverLogs;
    FileService::loadTransactions(strSenderId, senderLogs);
    FileService::loadTransactions(strReceiverId, receiverLogs);

    const Transaction* pSenderLast = nullptr;
    auto pCur1 = senderLogs.getHead();
    while (pCur1 != nullptr) { pSenderLast = &(pCur1->_data); pCur1 = pCur1->_pNext; }

    const Transaction* pReceiverLast = nullptr;
    auto pCur2 = receiverLogs.getHead();
    while (pCur2 != nullptr) { pReceiverLast = &(pCur2->_data); pCur2 = pCur2->_pNext; }

    TEST_ASSERT(pSenderLast != nullptr && pSenderLast->getType() == TRANSFER, "Lich su nguoi gui ghi nhan giao dich TRANSFER");
    TEST_ASSERT(pSenderLast != nullptr && pSenderLast->getAmount() == lTransferAmount, "Lich su nguoi gui ghi dung so tien chuyen");
    TEST_ASSERT(pReceiverLast != nullptr && pReceiverLast->getType() == RECEIVE, "Lich su nguoi nhan ghi nhan giao dich RECEIVE");
    TEST_ASSERT(pReceiverLast != nullptr && pReceiverLast->getAmount() == lTransferAmount, "Lich su nguoi nhan ghi dung so tien nhan");

    // 2.4 Va Loi #1 (A08): Chuyen tien den tai khoan khong ton tai khong duoc tru tien ao
    long lSenderBalCurrent = senderAcc.getBalance();
    const std::string strGhostId = "10014504509999";
    ErrorCode errGhost = UserController::processTransferAndPersist(senderAcc, strGhostId, 500000, strTimestamp);
    TEST_ASSERT(errGhost == ERR_RECIPIENT_NOT_FOUND, "Chuyen tien den tai khoan khong ton tai tra ve ERR_RECIPIENT_NOT_FOUND");
    TEST_ASSERT(senderAcc.getBalance() == lSenderBalCurrent, "[Va loi #1] So du nguoi gui trong RAM KHONG bi tru tien ao");

    Account senderCheckGhost;
    FileService::loadAccount(strSenderId, senderCheckGhost);
    TEST_ASSERT(senderCheckGhost.getBalance() == lSenderBalCurrent, "[Va loi #1] So du nguoi gui tren dia KHONG bi tru tien ao");

    // 2.5 Chan chuyen tien cho chinh minh
    ErrorCode errSelf = UserController::processTransferAndPersist(senderAcc, strSenderId, 200000, strTimestamp);
    TEST_ASSERT(errSelf == ERR_SAME_ACCOUNT, "Chan chuyen tien cho chinh minh tra ve ERR_SAME_ACCOUNT");

    // 2.6 Chan chuyen tien cho tai khoan dang bi khoa (10014504500005)
    const std::string strLockedId = "10014504500005";
    FileService::appendLockedCard(strLockedId);
    ErrorCode errLocked = UserController::processTransferAndPersist(senderAcc, strLockedId, 200000, strTimestamp);
    TEST_ASSERT(errLocked == ERR_CARD_LOCKED, "Chan chuyen tien cho the dang bi khoa tra ve ERR_CARD_LOCKED");
    LinkedList<std::string> clearLocked;
    FileService::saveLockedIds(clearLocked);

    // 2.7 Chan Path Traversal va ma nguoi nhan sai dinh dang trong API
    ErrorCode errTraversal = UserController::processTransferAndPersist(senderAcc, "../../malicious", 50000, strTimestamp);
    TEST_ASSERT(errTraversal == ERR_INVALID_FORMAT, "processTransferAndPersist chan path traversal va ma nhan sai dinh dang");
}

/******************************************************************************
 * 3. KIEM THU A11: DOI PIN LUU THETU.TXT & XEM LICH SU GIAO DICH
 ******************************************************************************/
void testChangePinAndHistory() {
    std::cout << "\n======================================================\n";
    std::cout << " 3. KIEM THU DOI PIN LUU THETU.TXT & XEM LICH SU (A11)\n";
    std::cout << "======================================================\n";

    FileService::initSampleData();

    const std::string strId = "10014504500004";
    Card card(strId, "123456");

    // 3.1 Doi PIN that bai do sai PIN cu
    std::string strMsg;
    bool bFailOld = UserController::processChangePinAndPersist(card, "000000", "888888", "888888", strMsg);
    TEST_ASSERT(!bFailOld, "Doi PIN that bai khi nhap sai PIN cu");
    TEST_ASSERT(card.getPin() == "123456", "PIN trong RAM giu nguyen khi loi");

    // 3.2 Doi PIN that bai do trung PIN mac dinh (Rule bao mat)
    bool bFailDef = UserController::processChangePinAndPersist(card, "123456", "123456", "123456", strMsg);
    TEST_ASSERT(!bFailDef, "Chan doi PIN moi trung PIN mac dinh 123456");

    // 3.3 Doi PIN that bai do 2 lan xac nhan khong khop
    bool bFailMismatch = UserController::processChangePinAndPersist(card, "123456", "654321", "111222", strMsg);
    TEST_ASSERT(!bFailMismatch, "Doi PIN that bai khi xac nhan khong khop");

    // 3.4 Doi PIN hop le va kiem tra cap nhat ngay xuong dia TheTu.txt
    bool bSuccess = UserController::processChangePinAndPersist(card, "123456", "778899", "778899", strMsg);
    TEST_ASSERT(bSuccess, "Doi ma PIN hop le sang 778899 tra ve true");
    TEST_ASSERT(card.getPin() == "778899", "PIN cua Card trong RAM cap nhat thanh 778899");

    // Doc lai TheTu.txt tren dia
    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);
    LinkedList<Card> listCardsOnDisk;
    FileService::loadCards(listCardsOnDisk, listLocked);

    const Card* pCardOnDisk = listCardsOnDisk.findIf([&strId](const Card& c) {
        return c.getId() == strId;
    });
    TEST_ASSERT(pCardOnDisk != nullptr, "Tim thay the 10014504500004 trong TheTu.txt tren dia");
    TEST_ASSERT(pCardOnDisk != nullptr && pCardOnDisk->getPin() == "778899",
                "[A11] TheTu.txt tren dia da luu ben vung ma PIN moi 778899");

    // 3.5 Xem lich su giao dich
    bool bDisplayHistory = UserController::displayTransactionHistory("10014504500001", false);
    TEST_ASSERT(bDisplayHistory, "displayTransactionHistory() chay thanh cong va doc duoc du lieu file");

    // Kiem tra voi tai khoan chua co giao dich
    bool bDisplayEmpty = UserController::displayTransactionHistory("10014504500010", false);
    TEST_ASSERT(bDisplayEmpty, "displayTransactionHistory() xu ly an toan voi tai khoan chua co lich su");
}

/******************************************************************************
 * 4. KIEM THU A09: DINH DANG THOI GIAN THUC TREN BIEN LAI & LOG
 ******************************************************************************/
void testRealtimeTimestampFormat() {
    std::cout << "\n======================================================\n";
    std::cout << " 4. KIEM THU FORMAT THOI GIAN THUC BIEN LAI (A09)\n";
    std::cout << "======================================================\n";

    std::string strTime = Transaction::getCurrentTimestamp();
    TEST_ASSERT(strTime.length() == 19, "Do dai chuoi thoi gian getCurrentTimestamp() dung 19 ky tu");
    TEST_ASSERT(strTime[4] == '-' && strTime[7] == '-', "Dinh dang nam-thang-ngay co dau '-'");
    TEST_ASSERT(strTime[10] == ' ', "Phan tach ngay gio bang khoang trang");
    TEST_ASSERT(strTime[13] == ':' && strTime[16] == ':', "Dinh dang gio:phut:giay co dau ':'");

    std::string strNow = getNowTimestamp();
    TEST_ASSERT(strNow.length() == 19, "Do dai getNowTimestamp() dung 19 ky tu");
}

/******************************************************************************
 * 5. KIEM THU TICH HOP TOAN TRINH USER SESSION (END-TO-END FLOW)
 ******************************************************************************/
void testUserEndToEndIntegration() {
    std::cout << "\n======================================================\n";
    std::cout << " 5. KIEM THU TICH HOP TOAN TRINH USER FLOW (E2E)\n";
    std::cout << "======================================================\n";

    resetSampleData();

    const std::string strUser1 = "10014504500006";
    const std::string strUser2 = "10014504500007";

    Account acc1, acc2;
    FileService::loadAccount(strUser1, acc1);
    FileService::loadAccount(strUser2, acc2);

    long lBal1Before = acc1.getBalance();
    long lBal2Before = acc2.getBalance();

    Card card1(strUser1, "123456");

    // Bước 1: Đổi PIN
    std::string strMsg;
    bool bPinOk = UserController::processChangePinAndPersist(card1, "123456", "999888", "999888", strMsg);
    TEST_ASSERT(bPinOk, "E2E - Buoc 1: Doi PIN thanh cong va luu file");

    // Bước 2: Rút tiền 100k
    std::string strTime1;
    ErrorCode errWith = UserController::processWithdrawAndPersist(acc1, 100000, strTime1);
    TEST_ASSERT(errWith == ERR_NONE, "E2E - Buoc 2: Rut tien 100,000 VND thanh cong va luu file");

    // Bước 3: Chuyển tiền 100k sang User 2
    std::string strTime2;
    ErrorCode errTrans = UserController::processTransferAndPersist(acc1, strUser2, 100000, strTime2);
    TEST_ASSERT(errTrans == ERR_NONE, "E2E - Buoc 3: Chuyen tien 100,000 VND sang User 2 thanh cong");

    // Bước 4: Kiểm tra số dư cuối cùng trên đĩa
    Account finalAcc1, finalAcc2;
    FileService::loadAccount(strUser1, finalAcc1);
    FileService::loadAccount(strUser2, finalAcc2);

    TEST_ASSERT(finalAcc1.getBalance() == lBal1Before - 100000 - 100000, "E2E - So du cuoi cung cua User 1 tren dia chinh xac");
    TEST_ASSERT(finalAcc2.getBalance() == lBal2Before + 100000, "E2E - So du cuoi cung cua User 2 tren dia chinh xac");

    // Bước 5: Kiểm tra số lượng bản ghi lịch sử của User 1
    LinkedList<Transaction> finalLogs1;
    FileService::loadTransactions(strUser1, finalLogs1);
    TEST_ASSERT(finalLogs1.getSize() >= 2, "E2E - Lich su User 1 ghi nhan day du it nhat 2 giao dich (WITHDRAW va TRANSFER)");
}

/******************************************************************************
 * MAIN ENTRY POINT CHO TEST SUITE PHASE 3 - MEMBER A
 ******************************************************************************/
int main() {
    ConsoleView::printHeader("BO KIEM THU CHUYEN SAU PHASE 3 - MEMBER A (TUAN)");
    std::cout << "Kiem thu toan dien Giao dich Tai chinh, FileService Persistence,\n";
    std::cout << "Va loi #1, #2, Chuyen tien 2 dau nguyen tu, Doi PIN va Lich su GD.\n";

    testWithdrawWithPersistence();
    testTransferWithPersistence();
    testChangePinAndHistory();
    testRealtimeTimestampFormat();
    testUserEndToEndIntegration();

    std::cout << "\n======================================================\n";
    std::cout << "       TONG KET KIEM THU PHASE 3 - MEMBER A (TUAN)    \n";
    std::cout << "======================================================\n";
    std::cout << "  So test THANH CONG [PASS]: " << g_nPass << "\n";
    std::cout << "  So test THAT BAI   [FAIL]: " << g_nFail << "\n";
    std::cout << "------------------------------------------------------\n";

    if (g_nFail == 0) {
        std::cout << "  >>> KET LUAN: 100% KIEM THU PHASE 3 CUA MEMBER A DA DAT! <<<\n";
    } else {
        std::cout << "  >>> KET LUAN: CO " << g_nFail << " KIEM THU CHUA DAT! <<<\n";
    }
    std::cout << "======================================================\n";

    return (g_nFail == 0) ? 0 : 1;
}
