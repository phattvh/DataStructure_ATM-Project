#ifndef FILESERVICE_H_INCLUDED_
#define FILESERVICE_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Admin.h"
#include "Card.h"
#include "Account.h"
#include "Transaction.h"

/**
 * @Description: Lop tien ich tinh xu ly doc, ghi, khoi tao va cap nhat tap tin flat-file
 */
class FileService {
public:
    static bool loadAdmins(LinkedList<Admin>& listAdmins);
    static bool loadCards(LinkedList<Card>& listCards, const LinkedList<std::string>& listLockedIds);
    static bool loadLockedIds(LinkedList<std::string>& listLockedIds);

    static bool saveCards(const LinkedList<Card>& listCards);
    static bool saveLockedIds(const LinkedList<std::string>& listLockedIds);

    static ErrorCode loadAccount(const std::string& strId, Account& acc);
    static bool saveAccount(const Account& acc);
    static bool deleteAccountFile(const std::string& strId);

    static void createAccountFiles(const std::string& strId, 
                                   const std::string& strName = "Chua Cap Nhat", 
                                   long lInitialBalance = 50000, 
                                   const std::string& strCurrency = "VND");

    static bool appendTransaction(const std::string& strId, const Transaction& trans);
    static bool loadTransactions(const std::string& strId, LinkedList<Transaction>& listTrans);

    static void initMockDataIfMissing();
};

#endif // FILESERVICE_H_INCLUDED_
