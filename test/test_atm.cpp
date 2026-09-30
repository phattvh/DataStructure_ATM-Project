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

int main() {
    std::cout << "======================================================\n";
    std::cout << "  BO KIEM THU TU DONG - TUAN 1 (MEMBER A TEST SUITE)\n";
    std::cout << "======================================================\n";

    testLinkedList();
    testAdminLoginLogic();
    testCardLockAndUnlockAlgorithm();
    testIdFormatValidation();
    testAdminAccountLifecycleFiles();

    std::cout << "======================================================\n";
    std::cout << "  TAT CA KIEM THU TUAN 1 DEU DA PASS 100%!\n";
    std::cout << "======================================================\n";
    return 0;
}
