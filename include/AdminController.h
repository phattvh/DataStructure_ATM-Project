#ifndef ADMINCONTROLLER_H_INCLUDED_
#define ADMINCONTROLLER_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"

/******************************************************************************
 * @Description: Bo dieu phoi Phan he Quan tri vien (Admin Module)
 * Nhiem vu Phase 2 (Ngay 6-7) - Member A:
 *   - Dang nhap Admin
 *   - Xem danh sach tai khoan
 *   - Them tai khoan (DoD: sinh ra dung va du 2 file [ID].txt va [LichSuID].txt)
 *   - Xoa tai khoan (xoa [ID].txt, giu LichSu[ID].txt)
 *   - Mo khoa the (xoa khoi KhoaThe.txt, reset failed attempts)
 ******************************************************************************/
class AdminController {
private:
    LinkedList<Admin>       _listAdmins;       // DS quan tri vien (tu Admin.txt)
    LinkedList<Card>        _listCards;        // DS the tu (tu TheTu.txt)
    LinkedList<std::string> _listLockedIds;    // DS ID the bi khoa (tu KhoaThe.txt)

    /**********************************************************
     * @Description Xac thuc dang nhap Admin
     * @param strUser Ten dang nhap Admin
     * @param strPass Mat khau Admin
     * @return true neu thong tin khop voi Admin.txt
     **********************************************************/
    bool verifyAdmin(const std::string& strUser, const std::string& strPass) const;

    /**********************************************************
     * @Description Chuc nang 1: Hien thi danh sach toan bo the tu
     **********************************************************/
    void viewCardList() const;

    /**********************************************************
     * @Description Chuc nang 2: Them tai khoan the tu moi
     * - Nhap ID 14 so chua ton tai
     * - PIN mac dinh 123456
     * - Nhap ten chu the, so du ban dau
     * - Tao 2 file [ID].txt va LichSu[ID].txt
     * - Cap nhat TheTu.txt
     **********************************************************/
    void addNewCard();

    /**********************************************************
     * @Description Chuc nang 3: Xoa tai khoan the tu theo ID
     * - Xac nhan tu Admin
     * - Xoa khoi RAM + TheTu.txt + [ID].txt
     * - Giu lai LichSu[ID].txt (muc dich kiem toan)
     **********************************************************/
    void deleteCard();

    /**********************************************************
     * @Description Chuc nang 4: Mo khoa the dang bi khoa
     * - Hien thi DS the bi khoa tu KhoaThe.txt
     * - Admin chon ID can mo khoa
     * - Reset so lan sai, xoa khoi KhoaThe.txt, cap nhat TheTu.txt
     **********************************************************/
    void unlockCard();

public:
    /**********************************************************
     * @Description Constructor: Khoi tao cac LinkedList rong
     **********************************************************/
    AdminController();

    /**********************************************************
     * @Description Tai lai toan bo du lieu tu dia vao RAM
     * @return true neu tai thanh cong
     **********************************************************/
    bool loadAllData();

    /**********************************************************
     * @Description Xu ly luong dang nhap Admin
     * @return true neu Admin dang nhap thanh cong
     **********************************************************/
    bool processAdminLogin();

    /**********************************************************
     * @Description Vong lap menu quan tri Admin (Xem DS, Them, Xoa, Mo khoa)
     **********************************************************/
    void processAdminMenu();

    // Getters phuc vu kiem thu
    const LinkedList<Admin>& getAdmins() const { return _listAdmins; }
    const LinkedList<Card>& getCards() const { return _listCards; }
    const LinkedList<std::string>& getLockedIds() const { return _listLockedIds; }
};

#endif // ADMINCONTROLLER_H_INCLUDED_
