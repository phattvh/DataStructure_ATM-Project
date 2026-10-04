#ifndef ACCOUNT_H_INCLUDED_
#define ACCOUNT_H_INCLUDED_

#include "Common.h"
#include <string>

/**********************************************************
 * @Description: Lop dai dien cho Thuc the Tai khoan khach hang
 * (Tuan thu C++ Coding Standard V2, bao ve tinh toan ven tai chinh)
 **********************************************************/
class Account {
private:
    std::string _strId;
    std::string _strName;
    long _lBalance;
    std::string _strCurrency;

public:
    /**********************************************************
     * @Description Constructor mac dinh
     **********************************************************/
    Account();

    /**********************************************************
     * @Description Constructor khoi tao co tham so
     * @param strId Ma so tai khoan (14 chu so)
     * @param strName Ho va ten chu tai khoan (co khoang trang)
     * @param lBalance So du ban dau (kieu long, khong duoc am)
     * @param strCurrency Loai tien te (mac dinh: "VND")
     **********************************************************/
    Account(const std::string& strId,
            const std::string& strName,
            long lBalance,
            const std::string& strCurrency = "VND");

    /**********************************************************
     * @Description Lay ma so tai khoan
     * @return Chuoi ID
     **********************************************************/
    std::string getId() const;

    /**********************************************************
     * @Description Lay ten chu tai khoan
     * @return Chuoi ten
     **********************************************************/
    std::string getName() const;

    /**********************************************************
     * @Description Lay so du kha dung
     * @return So du kieu long
     **********************************************************/
    long getBalance() const;

    /**********************************************************
     * @Description Lay don vi tien te
     * @return Chuoi loai tien te
     **********************************************************/
    std::string getCurrency() const;

    /**********************************************************
     * @Description Cap nhat ho ten chu tai khoan
     * @param strName Ho ten moi
     * @return void
     **********************************************************/
    void setName(const std::string& strName);

    /**********************************************************
     * @Description Cap nhat so du tai khoan co kiem tra am
     * @param lBalance So du moi (>= 0)
     * @return void
     **********************************************************/
    void setBalance(long lBalance);

    /**********************************************************
     * @Description Cap nhat don vi tien te
     * @param strCurrency Don vi tien te moi
     * @return void
     **********************************************************/
    void setCurrency(const std::string& strCurrency);

    /**********************************************************
     * @Description Kiem tra xem co the rut so tien lAmount khong
     * theo cac rang buoc: >= 50k, boi so 50k, so du con lai >= 50k
     * @param lAmount So tien muon rut
     * @return ErrorCode (ERR_NONE neu hop le)
     **********************************************************/
    ErrorCode canWithdraw(long lAmount) const;

    /**********************************************************
     * @Description Thuc hien tru so du tai khoan co kiem tra rang buoc canWithdraw
     * @param lAmount So tien rut
     * @return true neu rut thanh cong, false neu khong du dieu kien
     **********************************************************/
    bool withdraw(long lAmount);

    /**********************************************************
     * @Description Thuc hien cong them tien vao so du tai khoan
     * Chan tuyet doi so am va chong tran so nguyen (integer overflow)
     * @param lAmount So tien nap hoac nhan (> 0)
     * @return true neu nap thanh cong, false neu so tien khong hop le
     **********************************************************/
    bool deposit(long lAmount);
};

#endif // ACCOUNT_H_INCLUDED_
