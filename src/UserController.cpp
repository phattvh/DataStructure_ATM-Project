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

        // Luu ngay ma PIN moi xuong dia (TheTu.txt) chong mat mat trang thai
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
        // Co che Rollback bao dam tinh nguyen tu ACID: hoan tien lai nguoi gui
        senderAcc.deposit(lAmount);
        return ERR_SYSTEM_OVERFLOW;
    }

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

void UserController::displayAccountInfo(const Account& acc) {
    ConsoleView::displayAccountDetails(acc.getId(), acc.getName(), acc.getBalance(), acc.getCurrency());
    std::cout << "  So du kha dung: " 
              << (acc.getBalance() >= MIN_BALANCE_RESERVE ? acc.getBalance() - MIN_BALANCE_RESERVE : 0) 
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
                ConsoleView::printHeader("GIAO DICH RUT TIEN");
                std::cout << "  So du hien tai: " << acc.getBalance() << " " << acc.getCurrency() << "\n";
                std::cout << "  Han muc toi thieu: " << MIN_TRANSACTION << " VND (la boi so 50,000 VND)\n";
                std::cout << "  So du toi thieu duy tri: " << MIN_BALANCE_RESERVE << " VND\n\n";

                long lAmount = ConsoleView::inputMoney("Nhap so tien can rut (0 = Huy): ");
                if (lAmount == 0) {
                    ConsoleView::printInfo("Da huy giao dich rut tien.");
                    ConsoleView::pauseScreen();
                    break;
                }

                ErrorCode err = UserController::processWithdraw(acc, lAmount);
                if (err == ERR_INVALID_AMOUNT) {
                    ConsoleView::printError("So tien rut toi thieu phai tu 50,000 VND!");
                } else if (err == ERR_NOT_MULTIPLE) {
                    ConsoleView::printError("So tien rut phai la boi so cua 50,000 VND!");
                } else if (err == ERR_INSUFFICIENT_FUNDS) {
                    ConsoleView::printError("So du khong du! Can giu lai it nhat 50,000 VND so du toi thieu.");
                } else {
                    // Luu so du moi vao file [ID].txt tren dia
                    bool bSaveOk = FileService::saveAccount(acc);
                    if (!bSaveOk) {
                        acc.deposit(lAmount); // Rollback trong RAM
                        ConsoleView::printError("Loi I/O he thong: Khong the cap nhat so du xuong dia! Giao dich da bi huy.");
                    } else {
                        std::string strTime = getNowTimestamp();
                        // Ghi log giao dich vao file LichSu[ID].txt
                        Transaction tx(acc.getId(), WITHDRAW, lAmount, strTime, "Rut tien mat tai ATM");
                        FileService::appendTransaction(acc.getId(), tx);

                        ConsoleView::printSuccess("Rut tien thanh cong! Vui long nhan tien tai khe.");
                        ConsoleView::printReceipt(acc.getId(), "RUT TIEN MAT", lAmount, acc.getBalance(), strTime, acc.getCurrency());
                    }
                }
                ConsoleView::pauseScreen();
                break;
            }
            case 3: {
                ConsoleView::printHeader("GIAO DICH CHUYEN TIEN");
                std::cout << "  So du hien tai: " << acc.getBalance() << " " << acc.getCurrency() << "\n\n";

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

                long lAmount = ConsoleView::inputMoney("Nhap so tien muon chuyen (0 = Huy): ");
                if (lAmount == 0) {
                    ConsoleView::printInfo("Da huy giao dich chuyen tien.");
                    ConsoleView::pauseScreen();
                    break;
                }

                if (pReceiverMock != nullptr && pReceiverMock->getId() == strReceiverId) {
                    ErrorCode err = UserController::processTransfer(acc, *pReceiverMock, lAmount);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien chuyen toi thieu phai tu 50,000 VND!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien chuyen phai la boi so cua 50,000 VND!");
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
                    std::cout << "  So tien chuyen       : " << lAmount << " " << acc.getCurrency() << "\n";
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

                    // Dong bo so du nguoi gui tu dia neu co giao dich nhan tien ngoai luong dien ra song song
                    Account currentSenderOnDisk;
                    if (FileService::loadAccount(acc.getId(), currentSenderOnDisk) == ERR_NONE) {
                        if (currentSenderOnDisk.getBalance() > acc.getBalance()) {
                            acc.setBalance(currentSenderOnDisk.getBalance());
                        }
                    }

                    ErrorCode err = UserController::processTransfer(acc, receiverAcc, lAmount);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien chuyen toi thieu phai tu 50,000 VND!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien chuyen phai la boi so cua 50,000 VND!");
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
                            FileService::saveAccount(acc);
                            ConsoleView::printError("Loi I/O he thong khi cap nhat so du! Giao dich da duoc hoan tien an toan.");
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
                ConsoleView::printHeader("LICH SU GIAO DICH");
                LinkedList<Transaction> listTrans;
                bool bLoaded = FileService::loadTransactions(acc.getId(), listTrans);
                if (!bLoaded || listTrans.isEmpty()) {
                    ConsoleView::printInfo("Hien tai tai khoan chua co giao dich nao duoc ghi nhan.");
                    ConsoleView::pauseScreen();
                } else {
                    const int PAGE_SIZE = 5;
                    int iTotalItems = listTrans.getSize();
                    int iTotalPages = (iTotalItems + PAGE_SIZE - 1) / PAGE_SIZE;
                    int iCurrentPage = 1;

                    bool bViewing = true;
                    while (bViewing) {
                        ConsoleView::clearScreen();
                        ConsoleView::printHeader("LICH SU GIAO DICH - TAI KHOAN: " + acc.getId());
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
                                          << std::right << std::setw(12) << pCur->_data.getAmount() << " " << std::left << std::setw(5) << acc.getCurrency()
                                          << pCur->_data.getDetail() << "\n";
                            }
                            pCur = pCur->_pNext;
                            iIdx++;
                        }
                        std::cout << "----------------------------------------------------------------------\n";

                        if (iTotalPages == 1) {
                            ConsoleView::pauseScreen();
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
                }
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
                if (UserController::processChangePin(card, strOldPin, strNewPin, strConfirmPin, strMsg)) {
                    // Luu ngay ma PIN moi xuong dia (TheTu.txt) chong mat mat trang thai
                    FileService::updateCardPin(card.getId(), strNewPin);
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
