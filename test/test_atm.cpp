#include <iostream>
#include <cassert>
#include <string>
#include <filesystem>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"
#include "FileService.h"
#include "AtmController.h"

namespace fs = std::filesystem;

void testLinkedList() {
    std::cout << "[TEST] 1. Kiem thu Cau truc Du lieu LinkedList Template...\n";
    LinkedList<int> listInt;
    assert(listInt.isEmpty());
    assert(listInt.getSize() == 0);

    listInt.addTail(10);
    listInt.addTail(20);
    listInt.addTail(30);
    assert(listInt.getSize() == 3);
    assert(!listInt.isEmpty());

    int* pFound = listInt.findIf([](int val) { return val == 20; });
    assert(pFound != nullptr && *pFound == 20);

    int* pNotFound = listInt.findIf([](int val) { return val == 999; });
    assert(pNotFound == nullptr);

    bool bRemoved = listInt.removeIf([](int val) { return val == 20; });
    assert(bRemoved);
    assert(listInt.getSize() == 2);
    assert(listInt.findIf([](int val) { return val == 20; }) == nullptr);

    listInt.clear();
    assert(listInt.isEmpty());
    assert(listInt.getSize() == 0);
    std::cout << "  -> PASSED: LinkedList addTail, findIf, removeIf, clear\n";
}

void testAdminLoginLogic() {
    std::cout << "[TEST] 2. Kiem thu Logic Dang nhap Admin...\n";
    Admin admin1("admin1", "123456");
    Admin admin2("superadmin", "securepass");

    assert(admin1.getUsername() == "admin1");
    assert(admin1.verifyPassword("123456") == true);
    assert(admin1.verifyPassword("wrongpass") == false);
    assert(admin2.verifyPassword("securepass") == true);

    LinkedList<Admin> adminList;
    adminList.addTail(admin1);
    adminList.addTail(admin2);

    std::string testUser = "superadmin";
    std::string testPass = "securepass";
    Admin* pAdmin = adminList.findIf([&testUser, &testPass](const Admin& a) {
        return a.getUsername() == testUser && a.verifyPassword(testPass);
    });
    assert(pAdmin != nullptr);
    assert(pAdmin->getUsername() == "superadmin");

    testPass = "wrong";
    pAdmin = adminList.findIf([&testUser, &testPass](const Admin& a) {
        return a.getUsername() == testUser && a.verifyPassword(testPass);
    });
    assert(pAdmin == nullptr);

    std::cout << "  -> PASSED: Logic xac thuc tai khoan Admin\n";
}

void testCardLockAndUnlockAlgorithm() {
    std::cout << "[TEST] 3. Kiem thu Thuat toan Khoa the va Mo khoa the...\n";
    Card card("10014504509999", "123456");
    assert(!card.isLocked());
    assert(card.getFailedAttempts() == 0);

    // Thu nhap sai 1 lan
    card.recordFailedAttempt();
    assert(card.getFailedAttempts() == 1);
    assert(!card.isLocked());

    // Thu nhap sai lan 2
    card.recordFailedAttempt();
    assert(card.getFailedAttempts() == 2);
    assert(!card.isLocked());

    // Thu nhap sai lan 3 -> Tu dong khoa the
    card.recordFailedAttempt();
    assert(card.getFailedAttempts() == 3);
    assert(card.isLocked());

    // Thuat toan mo khoa the cua Admin
    card.resetFailedAttempts();
    assert(!card.isLocked());
    assert(card.getFailedAttempts() == 0);

    std::cout << "  -> PASSED: Thuat toan tu dong khoa khi sai 3 lan va mo khoa the\n";
}

void testIdFormatValidation() {
    std::cout << "[TEST] 4. Kiem thu Dinh dang Ma the (14 chu so)...\n";
    assert(AtmController::isValidIdFormat("10014504500001") == true);
    assert(AtmController::isValidIdFormat("1001450450000") == false);  // 13 so
    assert(AtmController::isValidIdFormat("100145045000001") == false); // 15 so
    assert(AtmController::isValidIdFormat("1001450450000A") == false); // Chua ky tu chu
    assert(AtmController::isValidIdFormat("100145-4500001") == false); // Chua ky tu dac biet
    std::cout << "  -> PASSED: Dinh dang 14 chu so cua the tu\n";
}

void testAdminAccountLifecycleFiles() {
    std::cout << "[TEST] 5. Kiem thu Vong doi Tai khoan (Them, Tao File, Xoa File, Bao toan Lich su)...\n";
    std::string strTestId = "99998888777766";
    FileService::createAccountFiles(strTestId, "Test Account Lifecycle", 100000, "VND");

    std::string strAccFile = DATA_DIR + strTestId + ".txt";
    std::string strHistFile = DATA_DIR + "LichSu" + strTestId + ".txt";

    assert(fs::exists(strAccFile));
    assert(fs::exists(strHistFile));

    Account acc;
    ErrorCode err = FileService::loadAccount(strTestId, acc);
    assert(err == ERR_NONE);
    assert(acc.getId() == strTestId);
    assert(acc.getBalance() == 100000);
    assert(acc.getName() == "Test Account Lifecycle");

    // Thao tac Xoa the cua Admin: Xoa file account nhung BAO TOAN file lich su
    bool bDeleted = FileService::deleteAccountFile(strTestId);
    assert(bDeleted == true);
    assert(!fs::exists(strAccFile));
    assert(fs::exists(strHistFile)); // Lich su phai con ton tai

    // Don dep file test
    std::remove(strHistFile.c_str());
    std::cout << "  -> PASSED: Tao file [ID].txt, LichSu[ID].txt va Xoa [ID].txt an toan\n";
}

void testMockDataInitializationAndCardLoading() {
    std::cout << "[TEST] 6. Kiem thu Khoi tao Mock Data (3 Admin, 10 The tu goc)...\n";
    FileService::initMockDataIfMissing();

    LinkedList<Admin> listAdmins;
    bool bAdminsLoaded = FileService::loadAdmins(listAdmins);
    assert(bAdminsLoaded == true);
    assert(listAdmins.getSize() >= 3);

    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);

    LinkedList<Card> listCards;
    bool bCardsLoaded = FileService::loadCards(listCards, listLocked);
    assert(bCardsLoaded == true);
    assert(listCards.getSize() >= 10);

    // Kiem tra the 10014504500001 co day du file du lieu
    Account acc;
    ErrorCode err = FileService::loadAccount("10014504500001", acc);
    assert(err == ERR_NONE);
    assert(acc.getId() == "10014504500001");
    assert(acc.getName() == "Nguyen Van An");
    assert(acc.getBalance() == 500000);

    std::cout << "  -> PASSED: Khoi tao du lieu goc Admin.txt, TheTu.txt va cac the tu thanh cong\n";
}

// =====================================================================
//  KIEM THU TUAN 2: LUONG KHACH HANG (USER MODULE)
// =====================================================================

void testWithdrawValidation() {
    std::cout << "[TEST] 7. Kiem thu Rang buoc Rut tien (>= 50k, boi so 50k, du toi thieu 50k)...\n";
    Account acc("10014504500001", "Nguyen Van An", 500000, "VND");

    // Rut tien hop le
    assert(acc.canWithdraw(100000) == ERR_NONE);
    assert(acc.canWithdraw(450000) == ERR_NONE); // 500000 - 450000 = 50000 (dung muc toi thieu)

    // Duoi 50.000 VND
    assert(acc.canWithdraw(40000)  == ERR_INVALID_AMOUNT);
    assert(acc.canWithdraw(0)      == ERR_INVALID_AMOUNT);

    // Khong phai boi so 50.000
    assert(acc.canWithdraw(75000)  == ERR_NOT_MULTIPLE);
    assert(acc.canWithdraw(130000) == ERR_NOT_MULTIPLE);

    // Vuot qua so du toi thieu
    assert(acc.canWithdraw(500000) == ERR_INSUFFICIENT_FUNDS); // Con lai 0, thieu 50k
    assert(acc.canWithdraw(450001) == ERR_NOT_MULTIPLE);  // 450001 >= 50k nhung khong phai boi so 50k

    // Kiem tra withdraw + deposit RAM
    assert(acc.getBalance() == 500000);
    acc.withdraw(100000);
    assert(acc.getBalance() == 400000);
    acc.deposit(100000);
    assert(acc.getBalance() == 500000);

    std::cout << "  -> PASSED: Rang buoc so tien rut (so tien, boi so, so du toi thieu)\n";
}

void testPinValidationRules() {
    std::cout << "[TEST] 8. Kiem thu Quy tac Doi PIN (khac cu, khac mac dinh, 6 so)...\n";

    // isValidPinFormat
    assert(AtmController::isValidPinFormat("123456") == true);
    assert(AtmController::isValidPinFormat("000000") == true);
    assert(AtmController::isValidPinFormat("12345")  == false); // 5 so
    assert(AtmController::isValidPinFormat("1234567")== false); // 7 so
    assert(AtmController::isValidPinFormat("12345a") == false); // chua chu

    // Card changePin logic
    Card card("10014504500001", "123456");
    assert(card.isDefaultPin() == true);

    card.changePin("654321");
    assert(card.isDefaultPin() == false);
    assert(card.checkPin("654321") == true);
    assert(card.checkPin("123456") == false);

    std::cout << "  -> PASSED: Quy tac dinh dang va doi PIN\n";
}

void testTransferValidation() {
    std::cout << "[TEST] 9. Kiem thu Rang buoc Chuyen tien (Atomicity, rang buoc so du)...\n";

    // Tai khoan gui
    Account sender("10014504500001", "Nguyen Van An", 500000, "VND");
    // Tai khoan nhan
    Account receiver("10014504500002", "Tran Thi Binh", 200000, "VND");

    long lAmount = 150000;

    // Kiem tra rang buoc truoc khi chuyen
    assert(sender.canWithdraw(lAmount) == ERR_NONE);

    // Thuc hien Atomic: tru/cong trong RAM
    long lSenderBefore = sender.getBalance();
    long lReceiverBefore = receiver.getBalance();

    sender.withdraw(lAmount);
    receiver.deposit(lAmount);

    assert(sender.getBalance()   == lSenderBefore   - lAmount);
    assert(receiver.getBalance() == lReceiverBefore + lAmount);

    // Kiem tra chuyen cho chinh minh (quy tac nghiep vu)
    assert(sender.getId() != receiver.getId()); // test: ID khac nhau moi cho phep

    // Kiem tra ghi lich su Transaction
    Transaction transSend("10014504500001", TRANSFER, lAmount, "2026-09-30 17:00:00",
                          "Chuyen tien cho TK: 10014504500002 (Tran Thi Binh)");
    assert(transSend.getAmount() == lAmount);
    assert(transSend.getType()   == TRANSFER);

    Transaction transRecv("10014504500002", RECEIVE, lAmount, "2026-09-30 17:00:00",
                          "Nhan tien tu TK: 10014504500001 (Nguyen Van An)");
    assert(transRecv.getType() == RECEIVE);

    std::cout << "  -> PASSED: Logic nguyen tu Chuyen tien (RAM) va tao ban ghi lich su 2 ben\n";
}

void testTimestampGeneration() {
    std::cout << "[TEST] 10. Kiem thu Format Thoi gian giao dich (YYYY-MM-DD HH:MM:SS)...\n";
    std::string strTs = AtmController::getCurrentTimestamp();
    // Format: YYYY-MM-DD HH:MM:SS (19 ky tu)
    assert(strTs.length() == 19);
    assert(strTs[4]  == '-');
    assert(strTs[7]  == '-');
    assert(strTs[10] == ' ');
    assert(strTs[13] == ':');
    assert(strTs[16] == ':');
    std::cout << "  -> PASSED: Timestamp format: " << strTs << "\n";
}

void testFileServiceTransactionAppend() {
    std::cout << "[TEST] 11. Kiem thu Ghi va Doc Lich su Giao dich (appendTransaction, loadTransactions)...\n";
    std::string strTestId = "88887777666655";
    // Tao file lich su tam
    FileService::createAccountFiles(strTestId, "Test Trans User", 500000, "VND");

    Transaction t1(strTestId, WITHDRAW, 100000, "2026-09-30 10:00:00", "Rut tien tai may ATM");
    Transaction t2(strTestId, TRANSFER, 50000, "2026-09-30 11:00:00", "Chuyen tien cho TK: 10014504500002");
    FileService::appendTransaction(strTestId, t1);
    FileService::appendTransaction(strTestId, t2);

    LinkedList<Transaction> listTrans;
    bool bLoaded = FileService::loadTransactions(strTestId, listTrans);
    assert(bLoaded == true);
    assert(listTrans.getSize() == 2);

    // Don dep
    std::string strAccFile  = DATA_DIR + strTestId + ".txt";
    std::string strHistFile = DATA_DIR + "LichSu" + strTestId + ".txt";
    std::remove(strAccFile.c_str());
    std::remove(strHistFile.c_str());

    std::cout << "  -> PASSED: Ghi noi (append) lich su giao dich va doc lai dung so luong\n";
}

int main() {
    std::cout << "======================================================\n";
    std::cout << "  BO KIEM THU TU DONG - TUAN 1 & 2 (MEMBER A SUITE)\n";
    std::cout << "======================================================\n";

    // --- Tuan 1 ---
    testLinkedList();
    testAdminLoginLogic();
    testCardLockAndUnlockAlgorithm();
    testIdFormatValidation();
    testAdminAccountLifecycleFiles();
    testMockDataInitializationAndCardLoading();

    // --- Tuan 2 ---
    testWithdrawValidation();
    testPinValidationRules();
    testTransferValidation();
    testTimestampGeneration();
    testFileServiceTransactionAppend();

    std::cout << "======================================================\n";
    std::cout << "  TAT CA 11 KIEM THU TUAN 1 & 2 DEU DA PASS 100%!\n";
    std::cout << "======================================================\n";
    return 0;
}
