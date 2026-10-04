#ifndef USERCONTROLLER_H_INCLUDED_
#define USERCONTROLLER_H_INCLUDED_

#include <string>
#include "Common.h"
#include "Card.h"
#include "Account.h"

/**********************************************************
 * @Description: Bo dieu khien nghiep vu Phan he Khach hang (User Module)
 * Quan ly xac thuc dang nhap, ep buoc doi PIN mac dinh, rut tien,
 * chuyen tien dam bao tinh nguyen tu va tuong tac phien giao dich.
 * (Tuan thu C++ Coding Standard V2)
 **********************************************************/
class UserController {
public:
    /**********************************************************
     * @Description Xac thuc dang nhap bang ma PIN va quan ly dem sai 3 lan
     * @param card Tham chieu doi tuong the can kiem tra
     * @param strInputPin Ma PIN nguoi dung nhap vao
     * @param bOutCardLocked Tham chieu tra ve co bao the da bi khoa hay chua
     * @return true neu dang nhap thanh cong, false neu that bai
     **********************************************************/
    static bool authenticate(Card& card, const std::string& strInputPin, bool& bOutCardLocked);

    /**********************************************************
     * @Description Kiem tra va ep buoc doi ma PIN neu the dang dung PIN mac dinh
     * @param card Tham chieu the can kiem tra va cap nhat PIN
     * @return true neu hop le hoac da doi PIN thanh cong
     **********************************************************/
    static bool enforceDefaultPinChange(Card& card);

    /**********************************************************
     * @Description Thuc hien nghiep vu rut tien theo day du cac rang buoc
     * @param acc Tham chieu tai khoan thuc hien rut tien
     * @param lAmount So tien muon rut
     * @return ErrorCode (ERR_NONE neu rut thanh cong)
     **********************************************************/
    static ErrorCode processWithdraw(Account& acc, long lAmount);

    /**********************************************************
     * @Description Thuc hien chuyen tien dam bao tinh nguyen tu va rollback
     * @param senderAcc Tai khoan nguoi chuyen
     * @param receiverAcc Tai khoan nguoi nhan
     * @param lAmount So tien can chuyen
     * @return ErrorCode (ERR_NONE neu chuyen thanh cong)
     **********************************************************/
    static ErrorCode processTransfer(Account& senderAcc, Account& receiverAcc, long lAmount);

    /**********************************************************
     * @Description Thuc hien doi ma PIN chu dong voi 2 lan xac nhan
     * @param card Tham chieu doi tuong the can doi PIN
     * @param strOldPin Ma PIN hien tai
     * @param strNewPin Ma PIN moi
     * @param strConfirmPin Xac nhan lai ma PIN moi
     * @param strOutMessage Thong bao ket qua tra ve
     * @return true neu doi PIN thanh cong, false neu loi
     **********************************************************/
    static bool processChangePin(Card& card, const std::string& strOldPin, 
                                 const std::string& strNewPin, const std::string& strConfirmPin,
                                 std::string& strOutMessage);

    /**********************************************************
     * @Description Hien thi thong tin tai khoan qua lop giao dien
     * @param acc Tai khoan can xem thong tin
     * @return void
     **********************************************************/
    static void displayAccountInfo(const Account& acc);

    /**********************************************************
     * @Description Vong lap dieu huong phien giao dich nguoi dung (Interactive Session)
     * @param card The tu cua nguoi dung dang dang nhap
     * @param acc Tai khoan cua nguoi dung dang dang nhap
     * @param pReceiverMock Tai khoan nguoi nhan gia lap (tuy chon cho test)
     * @return void
     **********************************************************/
    static void runUserSession(Card& card, Account& acc, Account* pReceiverMock = nullptr);

    /**********************************************************
     * @Description Kiem tra dinh dang ma PIN (dung 6 chu so)
     * @param strPin Chuoi PIN can kiem tra
     * @return true neu dung 6 chu so
     **********************************************************/
    static bool isValidPinFormat(const std::string& strPin);

    /**********************************************************
     * @Description Kiem tra dinh dang ma so tai khoan / the (dung 14 chu so)
     * @param strId Chuoi ID can kiem tra
     * @return true neu dung 14 chu so
     **********************************************************/
    static bool isValidIdFormat(const std::string& strId);
};

#endif // USERCONTROLLER_H_INCLUDED_
