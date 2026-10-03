#include "Account.h"
#include "Card.h"
#include "Common.h"
#include "ConsoleView.h"
#include <cassert>
#include <iostream>

/**********************************************************
 * @Description: Bo kiem thu tu dong cho cac thanh phan Tuan 1 cua Member C
 * - Model Card: khoi tao, dem sai 3 lan tu khoa the, mo khoa the, doi PIN
 * - Model Account: khoi tao, kiem tra rang buoc rut tien, cong tru so du
 * - ConsoleView: kiem tra dinh dang va layout
 **********************************************************/

void testCardModel() {
    std::cout << "[RUNNING] testCardModel...\n";

    // 1. Khoi tao mac dinh
    Card cardDefault;
    assert(cardDefault.getId() == "");
    assert(cardDefault.getPin() == DEFAULT_PIN);
    assert(cardDefault.isDefaultPin() == true);
    assert(cardDefault.isLocked() == false);
    assert(cardDefault.getFailedAttempts() == 0);

    // 2. Khoi tao co tham so
    Card cardUser("10014504500001", "654321", false);
    assert(cardUser.getId() == "10014504500001");
    assert(cardUser.getPin() == "654321");
    assert(cardUser.isDefaultPin() == false);
    assert(cardUser.checkPin("654321") == true);
    assert(cardUser.checkPin("000000") == false);

    // 3. Kiem tra co che dem sai va tu dong khoa the khi sai 3 lan
    cardUser.recordFailedAttempt();
    assert(cardUser.getFailedAttempts() == 1);
    assert(cardUser.isLocked() == false);

    cardUser.recordFailedAttempt();
    assert(cardUser.getFailedAttempts() == 2);
    assert(cardUser.isLocked() == false);

    cardUser.recordFailedAttempt(); // Lan thu 3
    assert(cardUser.getFailedAttempts() == 3);
    assert(cardUser.isLocked() == true); // The phai bi khoa

    // 4. Mo khoa the
    cardUser.resetFailedAttempts();
    assert(cardUser.getFailedAttempts() == 0);
    assert(cardUser.isLocked() == false);

    // 5. Doi ma PIN
    cardUser.changePin("999888");
    assert(cardUser.getPin() == "999888");
    assert(cardUser.checkPin("999888") == true);

    std::cout << "[PASSED] testCardModel\n";
}

void testAccountModel() {
    std::cout << "[RUNNING] testAccountModel...\n";

    Account acc("10014504500001", "Nguyen Trung Kien", 200000, "VND");
    assert(acc.getId() == "10014504500001");
    assert(acc.getName() == "Nguyen Trung Kien");
    assert(acc.getBalance() == 200000);
    assert(acc.getCurrency() == "VND");

    // Rang buoc 1: Rut duoi 50.000 VND
    assert(acc.canWithdraw(30000) == ERR_INVALID_AMOUNT);

    // Rang buoc 2: Rut khong phai boi so cua 50.000 VND
    assert(acc.canWithdraw(75000) == ERR_NOT_MULTIPLE);

    // Rang buoc 3: Rut lam so du con lai < 50.000 VND (vi du rut 200.000 -> con 0)
    assert(acc.canWithdraw(200000) == ERR_INSUFFICIENT_FUNDS);

    // Rut lam so du con lai < 50k (rut 160k -> khong hop le vi khong chia het, rut 170k -> boi so sai)
    // Thu rut 180.000 (boi so 50k la 150k -> so du con 50k -> hop le; rut 200k -> con 0 -> loi)
    assert(acc.canWithdraw(150000) == ERR_NONE); // Hop le, so du con lai 50.000 VND

    // Rut thu 100.000 VND
    assert(acc.canWithdraw(100000) == ERR_NONE);
    acc.withdraw(100000);
    assert(acc.getBalance() == 100000);

    // Bay gio so du con 100k, chi duoc rut toi da 50k (de lai 50k)
    assert(acc.canWithdraw(100000) == ERR_INSUFFICIENT_FUNDS);
    assert(acc.canWithdraw(50000) == ERR_NONE);

    // Nap/Nhan tien
    acc.deposit(300000);
    assert(acc.getBalance() == 400000);

    std::cout << "[PASSED] testAccountModel\n";
}

void testConsoleViewLayout() {
    std::cout << "[RUNNING] testConsoleViewLayout...\n";

    // Kiem tra cac ham in khong gay loi crash
    ConsoleView::printHeader("TEST CONSOLE VIEW");
    ConsoleView::printSuccess("Kiem thu thanh cong");
    ConsoleView::printWarning("Kiem thu canh bao");
    ConsoleView::printError("Kiem thu thong bao loi");
    ConsoleView::printAdminMenu();

    ConsoleView::printCardTableHeader();
    ConsoleView::printCardRow("10014504500001", "123456", false);
    ConsoleView::printCardRow("10014504500002", "888888", true);
    ConsoleView::printCardTableFooter();

    Account acc("10014504500001", "Nguyen Trung Kien", 500000, "VND");
    ConsoleView::displayAccountInfo(acc);

    std::cout << "[PASSED] testConsoleViewLayout\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  BAT DAU KIEM THU UNIT TEST - MEMBER C (TUAN 1)  \n";
    std::cout << "==================================================\n";

    testCardModel();
    testAccountModel();
    testConsoleViewLayout();

    std::cout << "\n>>> TAT CA TEST CUA MEMBER C TUAN 1 DEU PASS 100%! <<<\n";
    return 0;
}
