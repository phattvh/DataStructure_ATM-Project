#ifndef ATMCONTROLLER_H_INCLUDED_
#define ATMCONTROLLER_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"

/**
 * @Description: Bo dieu khien trung tam he thong ATM, quan ly luong nghiep vu Admin va he thong
 */
class AtmController {
private:
    LinkedList<Admin> _listAdmins;
    LinkedList<Card> _listCards;
    LinkedList<std::string> _listLockedIds;
    Account* _pCurrentAccount;
    UserRole _currentUserRole;

public:
    AtmController();
    ~AtmController();

    void initSystem();
    void run();

    // Luong Quan tri vien (Nhiem vu Tuan 1 cua Member A)
    void processAdminLogin();
    void processAdminMenu();
    void handleViewCards();
    void handleAddCard();
    void handleDeleteCard();
    void handleUnlockCard();

    // Luong Khach hang
    void processUserLogin();

    // Tien ich kiem tra hop le
    static bool isValidIdFormat(const std::string& strId);
};

#endif // ATMCONTROLLER_H_INCLUDED_
