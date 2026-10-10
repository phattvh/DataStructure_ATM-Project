#include "UserController.h"
#include "ConsoleView.h"
#include "FileService.h"
#include "Transaction.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <algorithm>

bool UserController::isValidPinFormat(const std::string& strPin) {
    return Card::isValidPinFormat(strPin);
}

bool UserController::isValidIdFormat(const std::string& strId) {
    if (strId.length() != static_cast<size_t>(ID_LENGTH)) {
        return false;
    }
    for (char ch : strId) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

bool UserController::authenticate(Card& card, const std::string& strInputPin, bool& bOutCardLocked) {
    bOutCardLocked = false;
    if (card.isLocked()) {
        bOutCardLocked = true;
        return false;
    }

    if (card.checkPin(strInputPin)) {
        card.resetFailedAttempts();
        return true;
    }

    card.recordFailedAttempt();
    if (card.isLocked()) {
        bOutCardLocked = true;
    }
    return false;
}

bool UserController::enforceDefaultPinChange(Card& card) {
    if (!card.isDefaultPin()) {
        return true;
    }

    ConsoleView::printWarning("The dang dung ma PIN mac dinh (" + DEFAULT_PIN + ")!");
    ConsoleView::printWarning("Theo quy dinh bao mat, ban BAT BUOC phai doi ma PIN moi.");

    while (true) {
        std::string strNewPin = ConsoleView::inputPassword("Nhap ma PIN moi gom 6 chu so (Enter de huy): ");
        if (strNewPin.empty()) {
            ConsoleView::printWarning("Da huy thao tac doi ma PIN bat buoc.");
            return false;
        }

        if (!UserController::isValidPinFormat(strNewPin)) {
            ConsoleView::printError("Ma PIN moi phai chua dung 6 chu so!");
            continue;
        }

        if (strNewPin == DEFAULT_PIN) {
            ConsoleView::printError("Ma PIN moi khong duoc trung voi ma PIN mac dinh (" + DEFAULT_PIN + ")!");
            continue;
        }

        std::string strConfirmPin = ConsoleView::inputPassword("Xac nhan lai ma PIN moi: ");
        if (strConfirmPin.empty()) {
            ConsoleView::printWarning("Da huy thao tac doi ma PIN bat buoc.");
            return false;
        }

        if (strNewPin != strConfirmPin) {
            ConsoleView::printError("Hai lan nhap ma PIN khong khop nhau! Vui long thu lai.");
            continue;
        }

        if (!card.changePin(strNewPin)) {
            ConsoleView::printError("Cap nhat ma PIN that bai. Vui long thu lai!");
            continue;
        }

        FileService::updateCardPin(card.getId(), strNewPin);

        ConsoleView::printSuccess("Doi ma PIN lan dau thanh cong! Vui long ghi nho ma PIN moi.");
        ConsoleView::pauseScreen();
        return true;
    }
}

ErrorCode UserController::processWithdraw(Account& acc, long lAmount) {
    ErrorCode err = acc.canWithdraw(lAmount);
    if (err == ERR_NONE) {
        if (!acc.withdraw(lAmount)) {
            return ERR_INSUFFICIENT_FUNDS;
        }
    }
    return err;
}

ErrorCode UserController::processWithdrawAndPersist(Account& acc, long lAmount, std::string& strOutTimestamp) {
    // Dong bo so du thuc te tu dia truoc khi thuc hien rut tien (chong Stale Read / Lost Update)
    Account currentOnDisk;
    if (FileService::loadAccount(acc.getId(), currentOnDisk) == ERR_NONE) {
        acc.setBalance(currentOnDisk.getBalance());
    }

    ErrorCode err = processWithdraw(acc, lAmount);
    if (err != ERR_NONE) {
        return err;
    }

    bool bSaveOk = FileService::saveAccount(acc);
    if (!bSaveOk) {
        acc.deposit(lAmount); 
        return ERR_FILE_NOT_FOUND;
    }

    strOutTimestamp = getNowTimestamp();
    Transaction tx(acc.getId(), WITHDRAW, lAmount, strOutTimestamp, "Rut tien mat tai ATM");
    FileService::appendTransaction(acc.getId(), tx);

    return ERR_NONE;
}

ErrorCode UserController::processTransfer(Account& senderAcc, Account& receiverAcc, long lAmount) {
    if (senderAcc.getId() == receiverAcc.getId()) {
        return ERR_SAME_ACCOUNT;
    }

    if (senderAcc.getCurrency() != receiverAcc.getCurrency()) {
        return ERR_INVALID_FORMAT;
    }

    ErrorCode err = senderAcc.canWithdraw(lAmount);
    if (err != ERR_NONE) {
        return err;
    }

    // Rut tien nguoi gui
    if (!senderAcc.withdraw(lAmount)) {
        return ERR_INSUFFICIENT_FUNDS;
    }

    // Cong tien nguoi nhan
    if (!receiverAcc.deposit(lAmount)) {
        senderAcc.deposit(lAmount);
        return ERR_SYSTEM_OVERFLOW;
    }

    return ERR_NONE;
}

ErrorCode UserController::processTransferAndPersist(Account& senderAcc,
                                                    const std::string& strReceiverId,
                                                    long lAmount,
                                                    std::string& strOutTimestamp,
                                                    Account* pReceiverMock) {
    if (!UserController::isValidIdFormat(strReceiverId)) {
        return ERR_INVALID_FORMAT;
    }

    if (senderAcc.getId() == strReceiverId) {
        return ERR_SAME_ACCOUNT;
    }

    // Kiem tra nguoi nhan co bi khoa khong
    LinkedList<std::string> listLocked;
    FileService::loadLockedIds(listLocked);
    if (listLocked.findIf([&strReceiverId](const std::string& strId) { return strId == strReceiverId; }) != nullptr) {
        return ERR_CARD_LOCKED;
    }

    ErrorCode errCheck = senderAcc.canWithdraw(lAmount);
    if (errCheck != ERR_NONE) {
        return errCheck;
    }

    Account receiverAcc;
    bool bUseMock = (pReceiverMock != nullptr && pReceiverMock->getId() == strReceiverId);

    if (bUseMock) {
        receiverAcc = *pReceiverMock;
    } else {
        ErrorCode errLoad = FileService::loadAccount(strReceiverId, receiverAcc);
        if (errLoad != ERR_NONE) {
            return ERR_RECIPIENT_NOT_FOUND;
        }
    }

    if (senderAcc.getCurrency() != receiverAcc.getCurrency()) {
        return ERR_INVALID_FORMAT;
    }

    ErrorCode errTransfer = processTransfer(senderAcc, receiverAcc, lAmount);
    if (errTransfer != ERR_NONE) {
        return errTransfer;
    }

    if (bUseMock && pReceiverMock != nullptr) {
        *pReceiverMock = receiverAcc;
    }

    bool bSaveSender = FileService::saveAccount(senderAcc);
    bool bSaveReceiver = false;
    if (bSaveSender) {
        bSaveReceiver = FileService::saveAccount(receiverAcc);
    }

    if (!bSaveSender || !bSaveReceiver) {
        senderAcc.deposit(lAmount);
        receiverAcc.withdraw(lAmount);
        bool bRollbackSaved = FileService::saveAccount(senderAcc);
        if (bUseMock && pReceiverMock != nullptr) {
            *pReceiverMock = receiverAcc;
        }
        if (!bRollbackSaved) {
            FileService::appendAdminLog("CRITICAL_ERROR", "Rollback saveAccount for sender " + senderAcc.getId() + " failed! Data desync on disk!");
        }
        return ERR_FILE_NOT_FOUND;
    }

    strOutTimestamp = getNowTimestamp();

    Transaction senderTx(senderAcc.getId(), TRANSFER, lAmount, strOutTimestamp,
                         "Chuyen tien den " + receiverAcc.getId() + " - " + receiverAcc.getName());
    FileService::appendTransaction(senderAcc.getId(), senderTx);

    Transaction receiverTx(receiverAcc.getId(), RECEIVE, lAmount, strOutTimestamp,
                           "Nhan tien tu " + senderAcc.getId() + " - " + senderAcc.getName());
    FileService::appendTransaction(receiverAcc.getId(), receiverTx);

    return ERR_NONE;
}

bool UserController::processChangePin(Card& card, const std::string& strOldPin, 
                                      const std::string& strNewPin, const std::string& strConfirmPin,
                                      std::string& strOutMessage) {
    if (!card.checkPin(strOldPin)) {
        strOutMessage = "Ma PIN cu khong chinh xac!";
        return false;
    }

    if (!UserController::isValidPinFormat(strNewPin)) {
        strOutMessage = "Ma PIN moi phai bao gom dung 6 chu so!";
        return false;
    }

    if (strNewPin == DEFAULT_PIN) {
        strOutMessage = "Ma PIN moi khong duoc trung voi ma PIN mac dinh (" + DEFAULT_PIN + ")!";
        return false;
    }

    if (strNewPin == strOldPin) {
        strOutMessage = "Ma PIN moi phai khac voi ma PIN hien tai!";
        return false;
    }

    if (strNewPin != strConfirmPin) {
        strOutMessage = "Xac nhan ma PIN khong khop!";
        return false;
    }

    if (!card.changePin(strNewPin)) {
        strOutMessage = "Doi ma PIN that bai do sai dinh dang!";
        return false;
    }

    strOutMessage = "Doi ma PIN thanh cong!";
    return true;
}

bool UserController::processChangePinAndPersist(Card& card, const std::string& strOldPin,
                                               const std::string& strNewPin, const std::string& strConfirmPin,
                                               std::string& strOutMessage) {
    if (!processChangePin(card, strOldPin, strNewPin, strConfirmPin, strOutMessage)) {
        return false;
    }

    if (!FileService::updateCardPin(card.getId(), strNewPin)) {
        strOutMessage = "Doi ma PIN trong RAM thanh cong nhung cap nhat tap tin TheTu.txt that bai!";
        return false;
    }

    strOutMessage = "Doi ma PIN thanh cong va da cap nhat vao he thong!";
    return true;
}

bool UserController::displayTransactionHistory(const std::string& strAccountId, bool bPause) {
    LinkedList<Transaction> listTrans;
    bool bLoaded = FileService::loadTransactions(strAccountId, listTrans);
    if (!bLoaded) {
        ConsoleView::printError("Loi I/O he thong: Khong the doc file lich su giao dich!");
        if (bPause) {
            ConsoleView::pauseScreen();
        }
        return false;
    }

    if (listTrans.isEmpty()) {
        ConsoleView::printInfo("Hien tai tai khoan chua co giao dich nao duoc ghi nhan.");
        if (bPause) {
            ConsoleView::pauseScreen();
        }
        return true;
    }

    const int PAGE_SIZE = 5;
    int iTotalItems = listTrans.getSize();
    int iTotalPages = (iTotalItems + PAGE_SIZE - 1) / PAGE_SIZE;
    int iCurrentPage = 1;

    std::string strCurrency = "VND";
    Account tempAcc;
    if (FileService::loadAccount(strAccountId, tempAcc) == ERR_NONE) {
        strCurrency = tempAcc.getCurrency();
    }

    bool bViewing = true;
    while (bViewing) {
        ConsoleView::clearScreen();
        ConsoleView::printHeader("LICH SU GIAO DICH - TAI KHOAN: " + strAccountId);
        std::cout << "  Trang " << iCurrentPage << " / " << iTotalPages 
                  << " (Tong cong: " << iTotalItems << " giao dich)\n";
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << std::left
                  << std::setw(22) << "THOI GIAN"
                  << std::setw(15) << "LOAI GD"
                  << std::setw(16) << "SO TIEN"
                  << "CHI TIET\n";
        std::cout << "----------------------------------------------------------------------\n";

        int iStartIndex = (iCurrentPage - 1) * PAGE_SIZE;
        int iEndIndex = std::min(iStartIndex + PAGE_SIZE, iTotalItems);
        int iIdx = 0;
        auto pCur = listTrans.getHead();
        while (pCur != nullptr) {
            if (iIdx >= iStartIndex && iIdx < iEndIndex) {
                std::cout << std::left
                          << std::setw(22) << pCur->_data.getTimestamp()
                          << std::setw(15) << pCur->_data.getTypeName()
                          << std::right << std::setw(12) << pCur->_data.getAmount() << " " << std::left << std::setw(5) << strCurrency
                          << pCur->_data.getDetail() << "\n";
            }
            pCur = pCur->_pNext;
            iIdx++;
        }
        std::cout << "----------------------------------------------------------------------\n";

        if (!bPause || iTotalPages == 1) {
            if (bPause) {
                ConsoleView::pauseScreen();
            }
            bViewing = false;
        } else {
            std::cout << "  Dieu huong: [N] Trang sau | [P] Trang truoc | [0] Quay lai menu\n";
            std::string strNav = ConsoleView::inputLine("  Nhap lua chon: ");
            size_t s1 = strNav.find_first_not_of(" \t\r\n");
            std::string strClean = (s1 == std::string::npos) ? "" : strNav.substr(s1, strNav.find_last_not_of(" \t\r\n") - s1 + 1);

            if (strClean == "0" || strClean.empty()) {
                bViewing = false;
            } else if (strClean == "N" || strClean == "n") {
                if (iCurrentPage < iTotalPages) {
                    iCurrentPage++;
                } else {
                    ConsoleView::printWarning("Ban dang o trang cuoi cung!");
                    ConsoleView::pauseScreen();
                }
            } else if (strClean == "P" || strClean == "p") {
                if (iCurrentPage > 1) {
                    iCurrentPage--;
                } else {
                    ConsoleView::printWarning("Ban dang o trang dau tien!");
                    ConsoleView::pauseScreen();
                }
            } else {
                ConsoleView::printError("Lua chon khong hop le!");
                ConsoleView::pauseScreen();
            }
        }
    }
    return true;
}

void UserController::displayAccountInfo(const Account& acc) {
    CurrencyConfig cfg = getCurrencyConfig(acc.getCurrency());
    ConsoleView::displayAccountDetails(acc.getId(), acc.getName(), acc.getBalance(), acc.getCurrency());
    long lAvailable = (acc.getBalance() >= cfg.lMinReserve) ? (acc.getBalance() - cfg.lMinReserve) : 0;
    std::cout << "  So du kha dung: " 
              << ConsoleView::formatMoney(lAvailable)
              << " " << acc.getCurrency() << "\n";
    ConsoleView::pauseScreen();
}

void UserController::runUserSession(Card& card, Account& acc, Account* pReceiverMock) {
    while (true) {
        ConsoleView::printUserMenu();
        int iChoice = ConsoleView::inputMenuChoice(0, 5, "Chon chuc nang: ");

        if (iChoice == 0) {
            ConsoleView::printSuccess("Tra the thanh cong. Cam on ban da su dung ATM!");
            ConsoleView::pauseScreen();
            break;
        }

        switch (iChoice) {
            case 1: {
                UserController::displayAccountInfo(acc);
                break;
            }
            case 2: {
                CurrencyConfig cfg = getCurrencyConfig(acc.getCurrency());
                if (cfg.strCode == "VND") {
                    ConsoleView::printHeader("GIAO DICH RUT TIEN (VND)");
                    std::cout << "  So du hien tai: " << ConsoleView::formatMoney(acc.getBalance()) << " VND\n";
                    std::cout << "  Han muc toi thieu: " << ConsoleView::formatMoney(cfg.lMinTransaction) << " VND (la boi so " << ConsoleView::formatMoney(cfg.lMinTransaction) << " VND)\n";
                    std::cout << "  So du toi thieu duy tri: " << ConsoleView::formatMoney(cfg.lMinReserve) << " VND\n\n";

                    std::string strPrompt = "Nhap so tien can rut [" + ConsoleView::formatMoney(cfg.lMinTransaction) + " - " + 
                                            ConsoleView::formatMoney(cfg.lMaxBalance) + " VND] (0 = Huy): ";
                    long lAmount = ConsoleView::inputMoneyRange(strPrompt, cfg.lMinTransaction, cfg.lMaxBalance, "VND");
                    if (lAmount == 0) {
                        ConsoleView::printInfo("Da huy giao dich rut tien.");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    std::string strTime;
                    ErrorCode err = UserController::processWithdrawAndPersist(acc, lAmount, strTime);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien rut toi thieu phai tu " + ConsoleView::formatMoney(cfg.lMinTransaction) + " VND!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien rut phai la boi so cua " + ConsoleView::formatMoney(cfg.lMinTransaction) + " VND!");
                    } else if (err == ERR_INSUFFICIENT_FUNDS) {
                        ConsoleView::printError("So du khong du! Can giu lai it nhat " + ConsoleView::formatMoney(cfg.lMinReserve) + " VND so du toi thieu.");
                    } else if (err == ERR_FILE_NOT_FOUND) {
                        ConsoleView::printError("Loi I/O he thong: Khong the cap nhat so du xuong dia! Giao dich da bi huy.");
                    } else if (err == ERR_NONE) {
                        ConsoleView::printSuccess("Rut tien thanh cong! Vui long nhan tien tai khe.");
                        ConsoleView::printReceipt(acc.getId(), "RUT TIEN MAT", lAmount, acc.getBalance(), strTime, acc.getCurrency());
                    } else {
                        ConsoleView::printError("Rut tien that bai!");
                    }
                } else {
                    // Tai khoan ngoai te (EUR, USD, JPY, GBP)
                    ConsoleView::printHeader("GIAO DICH RUT TIEN (" + cfg.strCode + ")");
                    std::cout << "  So du hien tai: " << ConsoleView::formatMoney(acc.getBalance()) << " " << cfg.strCode << "\n";
                    std::cout << "  Ty gia ATM hien tai: 1 " << cfg.strCode << " = " << ConsoleView::formatMoney(cfg.lExchangeRateToVND) << " VND\n\n";
                    std::cout << "  Chon phuong thuc rut tien:\n";
                    std::cout << "    1. Rut tien mat " << cfg.strCode << " (Dong tien goc tai khoan)\n";
                    std::cout << "    2. Quy doi sang tien mat VND tai ATM (1 " << cfg.strCode << " = " << ConsoleView::formatMoney(cfg.lExchangeRateToVND) << " VND)\n";
                    std::cout << "    0. Quay lai menu\n";
                    int iMethod = ConsoleView::inputMenuChoice(0, 2, "  Chon phuong thuc (0-2): ");

                    if (iMethod == 0) {
                        ConsoleView::printInfo("Da huy giao dich rut tien.");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    if (iMethod == 1) {
                        // Rut dong tien goc
                        std::cout << "\n  Han muc toi thieu: " << ConsoleView::formatMoney(cfg.lMinTransaction) << " " << cfg.strCode 
                                  << " (la boi so " << ConsoleView::formatMoney(cfg.lMinTransaction) << " " << cfg.strCode << ")\n";
                        std::cout << "  So du toi thieu duy tri: " << ConsoleView::formatMoney(cfg.lMinReserve) << " " << cfg.strCode << "\n\n";

                        std::string strPrompt = "Nhap so tien " + cfg.strCode + " can rut (0 = Huy): ";
                        long lAmount = ConsoleView::inputMoneyRange(strPrompt, cfg.lMinTransaction, cfg.lMaxBalance, cfg.strCode);
                        if (lAmount == 0) {
                            ConsoleView::printInfo("Da huy giao dich rut tien.");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        std::string strTime;
                        ErrorCode err = UserController::processWithdrawAndPersist(acc, lAmount, strTime);
                        if (err == ERR_INVALID_AMOUNT) {
                            ConsoleView::printError("So tien rut toi thieu phai tu " + ConsoleView::formatMoney(cfg.lMinTransaction) + " " + cfg.strCode + "!");
                        } else if (err == ERR_NOT_MULTIPLE) {
                            ConsoleView::printError("So tien rut phai la boi so cua " + ConsoleView::formatMoney(cfg.lMinTransaction) + " " + cfg.strCode + "!");
                        } else if (err == ERR_INSUFFICIENT_FUNDS) {
                            ConsoleView::printError("So du khong du! Can giu lai it nhat " + ConsoleView::formatMoney(cfg.lMinReserve) + " " + cfg.strCode + " so du toi thieu.");
                        } else if (err == ERR_NONE) {
                            ConsoleView::printSuccess("Rut tien thanh cong! Vui long nhan tien tai khe.");
                            ConsoleView::printReceipt(acc.getId(), "RUT TIEN MAT", lAmount, acc.getBalance(), strTime, acc.getCurrency());
                        } else {
                            ConsoleView::printError("Rut tien that bai!");
                        }
                    } else if (iMethod == 2) {
                        // Quy doi sang VND
                        std::cout << "\n  [QUY DOI NGOAI TE SANG TIEN MAT VND]\n";
                        std::cout << "  Ty gia ATM ap dung: 1 " << cfg.strCode << " = " << ConsoleView::formatMoney(cfg.lExchangeRateToVND) << " VND\n";
                        std::cout << "  So du toi thieu can duy tri: " << ConsoleView::formatMoney(cfg.lMinReserve) << " " << cfg.strCode << "\n\n";

                        long lVndAmount = ConsoleView::inputMoneyRange("Nhap so tien mat VND muon nhan (boi so 50,000 VND, 0 = Huy): ",
                                                                      MIN_TRANSACTION, 50000000L, "VND");
                        if (lVndAmount == 0) {
                            ConsoleView::printInfo("Da huy giao dich quy doi.");
                            ConsoleView::pauseScreen();
                            break;
                        }
                        if (lVndAmount % MIN_TRANSACTION != 0) {
                            ConsoleView::printError("So tien mat VND rut tai cay phai la boi so cua 50,000 VND!");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        if (cfg.lExchangeRateToVND <= 0) {
                            ConsoleView::printError("Loi he thong: Ty gia quy doi ngoai te chua duoc cau hinh hop le!");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        // Dong bo so du tu dia truoc khi quy doi ngoai te (chong Stale Read)
                        Account currentOnDisk;
                        if (FileService::loadAccount(acc.getId(), currentOnDisk) == ERR_NONE) {
                            acc.setBalance(currentOnDisk.getBalance());
                        }

                        // Tinh so ngoai te bi tru
                        long lDeduct = (lVndAmount + cfg.lExchangeRateToVND - 1) / cfg.lExchangeRateToVND;
                        if (acc.getBalance() < lDeduct || (acc.getBalance() - lDeduct) < cfg.lMinReserve) {
                            ConsoleView::printError("So du " + cfg.strCode + " khong du de quy doi! Can it nhat " +
                                                    ConsoleView::formatMoney(lDeduct + cfg.lMinReserve) + " " + cfg.strCode +
                                                    " de rut " + ConsoleView::formatMoney(lVndAmount) + " VND.");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        long lVndEquivalent = lDeduct * cfg.lExchangeRateToVND;
                        long lRemainder = lVndEquivalent - lVndAmount;

                        std::cout << "\n  Thong tin quy doi:\n";
                        std::cout << "    Tien mat nhan tai ATM : " << ConsoleView::formatMoney(lVndAmount) << " VND\n";
                        std::cout << "    Ty gia quy doi        : 1 " << cfg.strCode << " = " << ConsoleView::formatMoney(cfg.lExchangeRateToVND) << " VND\n";
                        std::cout << "    So tien se bi tru     : " << ConsoleView::formatMoney(lDeduct) << " " << cfg.strCode << "\n";
                        std::cout << "    So du con lai uoc tinh: " << ConsoleView::formatMoney(acc.getBalance() - lDeduct) << " " << cfg.strCode << "\n";
                        if (lRemainder > 0) {
                            std::cout << "    Chenh lech lam tron   : " << ConsoleView::formatMoney(lRemainder) << " VND\n";
                        }
                        std::cout << "\n";

                        if (!ConsoleView::confirmAction("Xac nhan thuc hien quy doi va rut tien?")) {
                            ConsoleView::printInfo("Da huy giao dich quy doi.");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        // Thuc hien tru tien va cap nhat file
                        acc.setBalance(acc.getBalance() - lDeduct);
                        if (!FileService::saveAccount(acc)) {
                            acc.setBalance(acc.getBalance() + lDeduct); // Rollback
                            ConsoleView::printError("Loi I/O he thong: Khong the cap nhat so du xuong dia! Giao dich da bi huy.");
                            ConsoleView::pauseScreen();
                            break;
                        }

                        std::string strTime = getNowTimestamp();
                        std::string strDetail = "Quy doi nhan " + ConsoleView::formatMoney(lVndAmount) + " VND (Ty gia: 1 " + cfg.strCode + " = " + ConsoleView::formatMoney(cfg.lExchangeRateToVND) + " VND)";
                        Transaction trans(acc.getId(), WITHDRAW, lDeduct, strTime, strDetail);
                        FileService::appendTransaction(acc.getId(), trans);

                        ConsoleView::printSuccess("Rut tien quy doi thanh cong! Vui long nhan " + ConsoleView::formatMoney(lVndAmount) + " VND tai khe tien.");
                        std::cout << "\n  ==================================================\n";
                        std::cout << "                 BIEN LAI GIAO DICH QUY DOI         \n";
                        std::cout << "  ==================================================\n";
                        std::cout << "  Ma tai khoan      : " << acc.getId() << "\n";
                        std::cout << "  Giao dich         : RUT TIEN (QUY DOI NGOAI TE)\n";
                        std::cout << "  Tien mat VND nhan : " << ConsoleView::formatMoney(lVndAmount) << " VND\n";
                        std::cout << "  Ty gia ATM        : 1 " << cfg.strCode << " = " << ConsoleView::formatMoney(cfg.lExchangeRateToVND) << " VND\n";
                        std::cout << "  So tien tru vao TK: " << ConsoleView::formatMoney(lDeduct) << " " << cfg.strCode << "\n";
                        std::cout << "  So du con lai     : " << ConsoleView::formatMoney(acc.getBalance()) << " " << cfg.strCode << "\n";
                        if (lRemainder > 0) {
                            std::cout << "  Chenh lech lam tron: " << ConsoleView::formatMoney(lRemainder) << " VND\n";
                        }
                        std::cout << "  Thoi gian         : " << strTime << "\n";
                        std::cout << "  ==================================================\n";
                    }
                }
                ConsoleView::pauseScreen();
                break;
            }
            case 3: {
                ConsoleView::printHeader("GIAO DICH CHUYEN TIEN");
                std::cout << "  So du hien tai: " << ConsoleView::formatMoney(acc.getBalance()) << " " << acc.getCurrency() << "\n\n";

                std::string strReceiverId = ConsoleView::inputLine("Nhap so tai khoan nguoi nhan (14 chu so): ");
                if (!UserController::isValidIdFormat(strReceiverId)) {
                    ConsoleView::printError("So tai khoan nguoi nhan phai bao gom dung 14 chu so!");
                    ConsoleView::pauseScreen();
                    break;
                }

                if (strReceiverId == acc.getId()) {
                    ConsoleView::printError("Khong the tu chuyen tien cho chinh tai khoan cua minh!");
                    ConsoleView::pauseScreen();
                    break;
                }

                // Kiem tra tai khoan nguoi nhan co dang bi khoa hay khong
                LinkedList<std::string> listLocked;
                FileService::loadLockedIds(listLocked);
                if (listLocked.findIf([&strReceiverId](const std::string& strId) { return strId == strReceiverId; }) != nullptr) {
                    ConsoleView::printError("Tai khoan nguoi nhan co ma so " + strReceiverId + " hien dang bi khoa!");
                    ConsoleView::pauseScreen();
                    break;
                }

                CurrencyConfig cfgTransfer = getCurrencyConfig(acc.getCurrency());
                std::string strPromptTransfer = "Nhap so tien muon chuyen [" + ConsoleView::formatMoney(cfgTransfer.lMinTransaction) + " - " + 
                                                ConsoleView::formatMoney(cfgTransfer.lMaxBalance) + " " + cfgTransfer.strCode + "] (0 = Huy): ";
                long lAmount = ConsoleView::inputMoneyRange(strPromptTransfer, cfgTransfer.lMinTransaction, cfgTransfer.lMaxBalance, cfgTransfer.strCode);
                if (lAmount == 0) {
                    ConsoleView::printInfo("Da huy giao dich chuyen tien.");
                    ConsoleView::pauseScreen();
                    break;
                }

                if (pReceiverMock != nullptr && pReceiverMock->getId() == strReceiverId) {
                    ErrorCode err = UserController::processTransfer(acc, *pReceiverMock, lAmount);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien chuyen toi thieu phai tu " + ConsoleView::formatMoney(cfgTransfer.lMinTransaction) + " " + cfgTransfer.strCode + "!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien chuyen phai la boi so cua " + ConsoleView::formatMoney(cfgTransfer.lMinTransaction) + " " + cfgTransfer.strCode + "!");
                    } else if (err == ERR_INSUFFICIENT_FUNDS) {
                        ConsoleView::printError("So du khong du de thuc hien giao dich chuyen tien!");
                    } else if (err == ERR_INVALID_FORMAT) {
                        ConsoleView::printError("Khong the chuyen tien giua hai tai khoan khac loai tien te!");
                    } else {
                        std::string strTime = getNowTimestamp();
                        ConsoleView::printSuccess("Chuyen tien thanh cong den tai khoan " + strReceiverId + " (" + pReceiverMock->getName() + ")!");
                        ConsoleView::printReceipt(acc.getId(), "CHUYEN TIEN", lAmount, acc.getBalance(), strTime, acc.getCurrency());
                    }
                } else {
                    // Chuyen tien that: Nap thong tin tai khoan nguoi nhan tu dia
                    Account receiverAcc;
                    ErrorCode errLoad = FileService::loadAccount(strReceiverId, receiverAcc);
                    if (errLoad != ERR_NONE) {
                        ConsoleView::printError("Tai khoan nguoi nhan co ma so " + strReceiverId + " khong ton tai!");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    // Hien thi xac nhan nguoi nhan
                    std::cout << "\n  --------------------------------------------------\n";
                    std::cout << "  Tai khoan nguoi nhan : " << receiverAcc.getId() << "\n";
                    std::cout << "  Ten chu tai khoan    : " << receiverAcc.getName() << "\n";
                    std::cout << "  So tien chuyen       : " << ConsoleView::formatMoney(lAmount) << " " << acc.getCurrency() << "\n";
                    std::cout << "  --------------------------------------------------\n";
                    bool bConfirm = ConsoleView::confirmAction("Xac nhan thuc hien giao dich chuyen tien tren?");
                    if (!bConfirm) {
                        ConsoleView::printInfo("Da huy giao dich chuyen tien.");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    // Nap lai so du moi nhat cua nguoi nhan ngay sau khi xac nhan (Chong TOCTOU Stale Read)
                    errLoad = FileService::loadAccount(strReceiverId, receiverAcc);
                    if (errLoad != ERR_NONE) {
                        ConsoleView::printError("Tai khoan nguoi nhan co ma so " + strReceiverId + " khong con ton tai tren he thong!");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    // Dong bo so du nguoi gui tu dia chong Double-Spending va stale read
                    Account currentSenderOnDisk;
                    if (FileService::loadAccount(acc.getId(), currentSenderOnDisk) == ERR_NONE) {
                        acc.setBalance(currentSenderOnDisk.getBalance());
                    }
                    if (acc.canWithdraw(lAmount) != ERR_NONE) {
                        ConsoleView::printError("So du tai khoan tren he thong da thay doi! Khong du so du de thuc hien giao dich.");
                        ConsoleView::pauseScreen();
                        break;
                    }

                    ErrorCode err = UserController::processTransfer(acc, receiverAcc, lAmount);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien chuyen toi thieu phai tu " + ConsoleView::formatMoney(cfgTransfer.lMinTransaction) + " " + cfgTransfer.strCode + "!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien chuyen phai la boi so cua " + ConsoleView::formatMoney(cfgTransfer.lMinTransaction) + " " + cfgTransfer.strCode + "!");
                    } else if (err == ERR_INSUFFICIENT_FUNDS) {
                        ConsoleView::printError("So du khong du de thuc hien giao dich chuyen tien!");
                    } else if (err == ERR_INVALID_FORMAT) {
                        ConsoleView::printError("Khong the chuyen tien giua hai tai khoan khac loai tien te!");
                    } else if (err == ERR_SYSTEM_OVERFLOW) {
                        ConsoleView::printError("Tai khoan nguoi nhan bi tran so du! Giao dich da duoc hoan tien.");
                    } else {
                        // Luu ca 2 tai khoan ben vung tren dia voi co che kiem tra loi
                        bool bSaveSender = FileService::saveAccount(acc);
                        bool bSaveReceiver = false;
                        if (bSaveSender) {
                            bSaveReceiver = FileService::saveAccount(receiverAcc);
                        }

                        if (!bSaveSender || !bSaveReceiver) {
                            // Rollback ca tren RAM va tren dia neu co loi I/O
                            acc.deposit(lAmount);
                            receiverAcc.withdraw(lAmount);
                            bool bRollbackSaved = FileService::saveAccount(acc);
                            if (!bRollbackSaved) {
                                FileService::appendAdminLog("CRITICAL_ERROR", "Rollback saveAccount for sender " + acc.getId() + " failed! Data desync on disk!");
                                ConsoleView::printError("Canh bao khan cap: Loi I/O nghiem trong khi hoan tien xuong dia! Vui long lien he Admin.");
                            } else {
                                ConsoleView::printError("Loi I/O he thong khi cap nhat so du! Giao dich da duoc hoan tien an toan.");
                            }
                        } else {
                            std::string strTime = getNowTimestamp();
                            // Ghi log giao dich nguoi gui
                            Transaction senderTx(acc.getId(), TRANSFER, lAmount, strTime,
                                                 "Chuyen tien den " + receiverAcc.getId() + " - " + receiverAcc.getName());
                            FileService::appendTransaction(acc.getId(), senderTx);

                            // Ghi log giao dich nguoi nhan
                            Transaction receiverTx(receiverAcc.getId(), RECEIVE, lAmount, strTime,
                                                   "Nhan tien tu " + acc.getId() + " - " + acc.getName());
                            FileService::appendTransaction(receiverAcc.getId(), receiverTx);

                            ConsoleView::printSuccess("Chuyen tien thanh cong den tai khoan " + strReceiverId + " (" + receiverAcc.getName() + ")!");
                            ConsoleView::printReceipt(acc.getId(), "CHUYEN TIEN", lAmount, acc.getBalance(), strTime, acc.getCurrency());
                        }
                    }
                }
                ConsoleView::pauseScreen();
                break;
            }
            case 4: {
                UserController::displayTransactionHistory(acc.getId());
                break;
            }
            case 5: {
                ConsoleView::printHeader("DOI MA PIN");
                std::string strOldPin = ConsoleView::inputPassword("Nhap ma PIN hien tai (nhap 0 hoac Enter de huy): ");
                if (strOldPin == "0" || strOldPin.empty()) {
                    ConsoleView::printInfo("Da huy thao tac doi ma PIN.");
                    ConsoleView::pauseScreen();
                    break;
                }
                std::string strNewPin = ConsoleView::inputPassword("Nhap ma PIN moi (6 so, nhap 0 hoac Enter de huy): ");
                if (strNewPin == "0" || strNewPin.empty()) {
                    ConsoleView::printInfo("Da huy thao tac doi ma PIN.");
                    ConsoleView::pauseScreen();
                    break;
                }
                std::string strConfirmPin = ConsoleView::inputPassword("Nhap lai ma PIN moi: ");

                std::string strMsg;
                if (UserController::processChangePinAndPersist(card, strOldPin, strNewPin, strConfirmPin, strMsg)) {
                    ConsoleView::printSuccess(strMsg);
                } else {
                    ConsoleView::printError(strMsg);
                }
                ConsoleView::pauseScreen();
                break;
            }
            default:
                ConsoleView::printError("Lua chon khong hop le!");
                ConsoleView::pauseScreen();
                break;
        }
    }
}
