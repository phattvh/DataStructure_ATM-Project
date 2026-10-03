#ifndef ACCOUNT_H_INCLUDED_
#define ACCOUNT_H_INCLUDED_

#include "Common.h"
#include <string>

/**********************************************************
 * @Description: Lop dai dien cho Thuc the Tai khoan khach hang
 * (Phu trach boi Member C - Task 3.2)
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
     * @param lBalance So du ban dau (kieu long)
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
     * @Description Kiem tra xem co the rut so tien lAmount khong
     * theo cac rang buoc: >= 50k, boi so 50k, so du con lai >= 50k
     * @param lAmount So tien muon rut
     * @return ErrorCode (ERR_NONE neu hop le)
     **********************************************************/
    ErrorCode canWithdraw(long lAmount) const;

    /**********************************************************
     * @Description Thuc hien tru so du tai khoan
     * @param lAmount So tien rut
     * @return void
     **********************************************************/
    void withdraw(long lAmount);

    /**********************************************************
     * @Description Thuc hien cong them tien vao so du tai khoan
     * @param lAmount So tien nap hoac nhan
     * @return void
     **********************************************************/
    void deposit(long lAmount);
};

#endif // ACCOUNT_H_INCLUDED_
