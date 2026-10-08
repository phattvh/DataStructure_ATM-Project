#ifndef ATMCONTROLLER_H_INCLUDED_
#define ATMCONTROLLER_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "FileService.h"
#include "ConsoleView.h"
#include "UserController.h"
#include "AdminController.h"

/******************************************************************************
 * @Description: Bo dieu phoi trung tam he thong ATM (AtmController)
 * Quan ly trang thai phien lam viec toan cuc, du lieu RAM (LinkedList),
 * dieu huong menu chinh, phan he Quan tri Admin va phan he Khach hang User.
 * (Tuan thu C++ Coding Standard V2 - Nhiem vu Phase 2 Member C - Phat)
 ******************************************************************************/
class AtmController {
private:
    LinkedList<Admin>       _listAdmins;
    LinkedList<Card>        _listCards;
    LinkedList<std::string> _listLockedIds;

    Account*                _pCurrentAccount;
    Card*                   _pCurrentCard;
    UserRole                _eCurrentRole;

public:
    /**********************************************************
     * @Description Constructor khoi tao he thong ATM
     **********************************************************/
    AtmController();

    /**********************************************************
     * @Description Destructor thu hoi vung nho phien lam viec (RAII)
     **********************************************************/
    ~AtmController();

    // Vo hieu hoa copy semantics de dam bao an toan bo nho
    AtmController(const AtmController&) = delete;
    AtmController& operator=(const AtmController&) = delete;

    /**********************************************************
     * @Description Khoi tao va nap du lieu he thong tu dia vao RAM
     * @return true neu nap thanh cong cac tap tin co ban
     **********************************************************/
    bool initData();

    /**********************************************************
     * @Description Vong lap dieu phoi chinh cua ung dung ATM
     * @return void
     **********************************************************/
    void run();

    // --- PHAN HE QUAN TRI (ADMIN MODULE) ---

    /**********************************************************
     * @Description Xu ly quy trinh dang nhap quan tri vien
     * @return void
     **********************************************************/
    void processAdminLogin();

    /**********************************************************
     * @Description Vong lap menu 4 chuc nang quan tri vien
     * @return void
     **********************************************************/
    void processAdminMenu();

    /**********************************************************
     * @Description Admin chuc nang 1: Xem danh sach the tu
     * @return void
     **********************************************************/
    void adminViewCards();

    /**********************************************************
     * @Description Admin chuc nang 2: Them the tu va tai khoan moi
     * @return void
     **********************************************************/
    void adminAddCard();

    /**********************************************************
     * @Description Admin chuc nang 3: Xoa the tu va tai khoan tren dia
     * @return void
     **********************************************************/
    void adminDeleteCard();

    /**********************************************************
     * @Description Admin chuc nang 4: Mo khoa the bi khoa
     * @return void
     **********************************************************/
    void adminUnlockCard();

    // --- PHAN HE KHACH HANG (USER MODULE) ---

    /**********************************************************
     * @Description Xu ly quy trinh dang nhap khach hang
     * @return void
     **********************************************************/
    void processUserLogin();

    /**********************************************************
     * @Description Dieu huong menu va phien lam viec khach hang
     * @return void
     **********************************************************/
    void processUserMenu();

    /**********************************************************
     * @Description Don dep du lieu phien lam viec khi dang xuat
     * @return void
     **********************************************************/
    void cleanupSession();

    // --- CAC PHUONG THUC NGHIEP VU COT LOI & KIEM THU (CORE LOGIC) ---

    /**********************************************************
     * @Description Kiem tra tai khoan Admin co hop le khong
     * @param strUser Ten dang nhap
     * @param strPass Mat khau
     * @return true neu hop le, nguoc lai false
     **********************************************************/
    bool authenticateAdmin(const std::string& strUser, const std::string& strPass) const;

    /**********************************************************
     * @Description Nghiep vu them the va tai khoan moi vao RAM & Disk
     * @param strId Ma the 14 chu so
     * @param strName Ho ten chu tai khoan
     * @param lInitialBalance So du ban dau
     * @param strCurrency Don vi tien te (mac dinh "VND")
     * @return ErrorCode (ERR_NONE neu thanh cong)
     **********************************************************/
    ErrorCode addCardAccount(const std::string& strId,
                             const std::string& strName,
                             long lInitialBalance,
                             const std::string& strCurrency = "VND");

    /**********************************************************
     * @Description Nghiep vu xoa the khoi RAM va xoa file [ID].txt
     * @param strId Ma the can xoa
     * @return ErrorCode (ERR_NONE neu thanh cong)
     **********************************************************/
    ErrorCode deleteCardAccount(const std::string& strId);

    /**********************************************************
     * @Description Nghiep vu mo khoa the bi khoa
     * @param strId Ma the can mo khoa
     * @return ErrorCode (ERR_NONE neu thanh cong)
     **********************************************************/
    ErrorCode unlockCardAccount(const std::string& strId);

    // Getters phuc vu kiem thu
    const LinkedList<Admin>& getAdmins() const;
    LinkedList<Admin>& getAdmins();

    const LinkedList<Card>& getCards() const;
    LinkedList<Card>& getCards();

    const LinkedList<std::string>& getLockedIds() const;
    LinkedList<std::string>& getLockedIds();

    UserRole getCurrentRole() const;
    const Account* getCurrentAccount() const;
    const Card* getCurrentCard() const;
};

#endif // ATMCONTROLLER_H_INCLUDED_
