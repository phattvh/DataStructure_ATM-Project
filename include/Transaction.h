#ifndef TRANSACTION_H_INCLUDED_
#define TRANSACTION_H_INCLUDED_

#include <string>
#include "Common.h"

/**
 * @Description: Lop dai dien cho ban ghi lich su giao dich
 */
class Transaction {
private:
    std::string _strId;
    TransactionType _eType;
    long _lAmount;
    std::string _strTimestamp;
    std::string _strDetail;

public:
    Transaction();
    Transaction(const std::string& strId, TransactionType eType, long lAmount,
                const std::string& strTimestamp, const std::string& strDetail);

    std::string getId() const;
    TransactionType getType() const;
    long getAmount() const;
    std::string getTimestamp() const;
    std::string getDetail() const;

    std::string formatForFile() const;
    std::string toString() const;
};

#endif // TRANSACTION_H_INCLUDED_
