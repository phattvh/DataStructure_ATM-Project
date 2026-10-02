#ifndef ACCOUNT_H_INCLUDED_
#define ACCOUNT_H_INCLUDED_

#include <string>
#include "Common.h"

/**
 * @Description: Lop dai dien cho thong tin tai khoan ngan hang chi tiet
 */
class Account {
private:
    std::string _strId;
    std::string _strName;
    long _lBalance;
    std::string _strCurrency;

public:
    Account();
    Account(const std::string& strId, const std::string& strName, long lBalance, const std::string& strCurrency);

    std::string getId() const;
    std::string getName() const;
    long getBalance() const;
    std::string getCurrency() const;

    void setName(const std::string& strName);
    void setBalance(long lBalance);
    void setCurrency(const std::string& strCurrency);

    ErrorCode canWithdraw(long lAmount) const;
    void withdraw(long lAmount);
    void deposit(long lAmount);
};

#endif // ACCOUNT_H_INCLUDED_
