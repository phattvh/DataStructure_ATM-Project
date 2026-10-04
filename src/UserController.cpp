#include "UserController.h"
#include "ConsoleView.h"
#include <iostream>
#include <iomanip>
#include <cctype>

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
        std::string strNewPin = ConsoleView::inputPassword("Nhap ma PIN moi gom 6 chu so: ");
        if (!UserController::isValidPinFormat(strNewPin)) {
            ConsoleView::printError("Ma PIN moi phai chua dung 6 chu so!");
            continue;
        }

        if (strNewPin == DEFAULT_PIN) {
            ConsoleView::printError("Ma PIN moi khong duoc trung voi ma PIN mac dinh (" + DEFAULT_PIN + ")!");
            continue;
        }

        std::string strConfirmPin = ConsoleView::inputPassword("Xac nhan lai ma PIN moi: ");
        if (strNewPin != strConfirmPin) {
            ConsoleView::printError("Hai lan nhap ma PIN khong khop nhau! Vui long thu lai.");
            continue;
        }

        if (!card.changePin(strNewPin)) {
            ConsoleView::printError("Cap nhat ma PIN that bai. Vui long thu lai!");
            continue;
        }

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
                    ConsoleView::printSuccess("Rut tien thanh cong! Vui long nhan tien tai khe.");
                    ConsoleView::printReceipt(acc.getId(), "RUT TIEN MAT", lAmount, acc.getBalance(), "Realtime");
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
                    } else {
                        ConsoleView::printSuccess("Chuyen tien thanh cong den tai khoan " + strReceiverId + " (" + pReceiverMock->getName() + ")!");
                        ConsoleView::printReceipt(acc.getId(), "CHUYEN TIEN", lAmount, acc.getBalance(), "Realtime");
                    }
                } else {
                    // Kiem tra rang buoc tai chinh khi chua ket noi Storage (Member B)
                    ErrorCode err = acc.canWithdraw(lAmount);
                    if (err == ERR_INVALID_AMOUNT) {
                        ConsoleView::printError("So tien chuyen toi thieu phai tu 50,000 VND!");
                    } else if (err == ERR_NOT_MULTIPLE) {
                        ConsoleView::printError("So tien chuyen phai la boi so cua 50,000 VND!");
                    } else if (err == ERR_INSUFFICIENT_FUNDS) {
                        ConsoleView::printError("So du khong du de thuc hien giao dich chuyen tien!");
                    } else {
                        acc.withdraw(lAmount);
                        ConsoleView::printSuccess("Chuyen tien thanh cong den tai khoan " + strReceiverId + "!");
                        ConsoleView::printReceipt(acc.getId(), "CHUYEN TIEN", lAmount, acc.getBalance(), "Realtime");
                    }
                }
                ConsoleView::pauseScreen();
                break;
            }
            case 4: {
                ConsoleView::printHeader("LICH SU GIAO DICH");
                ConsoleView::printInfo("Lich su giao dich se duoc ket noi voi Storage Module cua Member B.");
                ConsoleView::pauseScreen();
                break;
            }
            case 5: {
                ConsoleView::printHeader("DOI MA PIN");
                std::string strOldPin = ConsoleView::inputPassword("Nhap ma PIN hien tai: ");
                std::string strNewPin = ConsoleView::inputPassword("Nhap ma PIN moi (6 so): ");
                std::string strConfirmPin = ConsoleView::inputPassword("Nhap lai ma PIN moi: ");

                std::string strMsg;
                if (UserController::processChangePin(card, strOldPin, strNewPin, strConfirmPin, strMsg)) {
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
