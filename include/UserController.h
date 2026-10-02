#ifndef USERCONTROLLER_H_INCLUDED_
#define USERCONTROLLER_H_INCLUDED_

#include <string>
#include "Common.h"
#include "Card.h"
#include "Account.h"

/**
 * @Description: Bo dieu khien nghiep vu Phan he Khach hang (User Module)
 * Phan cong: Member A (Tuan) - Business Logic & User Flow
 * 
 * Pham vi trach nhiem theo docs/06_ke_hoach_trien_khai.md:
 * - Xac thuc dang nhap User, dem sai 3 lan khoa the
 * - Bat buoc doi ma PIN mac dinh (123456) o lan dau dang nhap
 * - Rut tien tuan thu rang buoc tai chinh (toi thieu 50k, boi so 50k, duy tri 50k)
 * - Chuyen tien dam bao tinh nguyen tu
 * - Xem thong tin tai khoan & Xem so du
 * - Doi ma PIN voi 2 lan xac nhan
 */
class UserController {
public:
    // Xac thuc dang nhap va co che khoa the
    static bool authenticate(Card& card, const std::string& strInputPin, bool& bOutCardLocked);

    // Kiem tra va yeu cau doi PIN mac dinh
    static bool enforceDefaultPinChange(Card& card);

    // Xu ly nghiep vu Rut tien
    static ErrorCode processWithdraw(Account& acc, long lAmount);

    // Xu ly nghiep vu Chuyen tien
    static ErrorCode processTransfer(Account& senderAcc, Account& receiverAcc, long lAmount);

    // Xu ly nghiep vu Doi ma PIN
    static bool processChangePin(Card& card, const std::string& strOldPin, 
                                 const std::string& strNewPin, const std::string& strConfirmPin,
                                 std::string& strOutMessage);

    // Hien thi thong tin tai khoan
    static void displayAccountInfo(const Account& acc);

    // Vong lap menu phien lam viec khach hang (Interactive Session)
    static void runUserSession(Card& card, Account& acc);

    // Tien ich kiem tra dinh dang
    static bool isValidPinFormat(const std::string& strPin);
    static bool isValidIdFormat(const std::string& strId);
};

#endif // USERCONTROLLER_H_INCLUDED_
