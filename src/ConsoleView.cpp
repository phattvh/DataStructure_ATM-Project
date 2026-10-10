#include "ConsoleView.h"
#include "Account.h"
#include <iostream>
#include <iomanip>
#include <cctype>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <signal.h>

static struct termios g_savedTerm;
static bool g_bSavedTermActive = false;

static void linuxSignalHandler(int iSig) {
    if (g_bSavedTermActive) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_savedTerm);
        g_bSavedTermActive = false;
    }
    _exit(128 + iSig);
}

class LinuxTerminalRawGuard {
private:
    struct termios _oldTerm;
    bool _bActive;

public:
    LinuxTerminalRawGuard() : _bActive(false) {
        if (tcgetattr(STDIN_FILENO, &this->_oldTerm) >= 0) {
            g_savedTerm = this->_oldTerm;
            g_bSavedTermActive = true;
            signal(SIGINT, linuxSignalHandler);
            signal(SIGTERM, linuxSignalHandler);

            struct termios newTerm = this->_oldTerm;
            newTerm.c_lflag &= ~(ICANON | ECHO);
            newTerm.c_cc[VMIN] = 1;
            newTerm.c_cc[VTIME] = 0;
            if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) >= 0) {
                this->_bActive = true;
            }
        }
    }

    ~LinuxTerminalRawGuard() {
        if (this->_bActive) {
            tcsetattr(STDIN_FILENO, TCSADRAIN, &this->_oldTerm);
            g_bSavedTermActive = false;
        }
    }

    bool isActive() const {
        return this->_bActive;
    }
};
#endif

// Dinh nghia ma mau ANSI
const std::string ANSI_RESET  = "\033[0m";
const std::string ANSI_BOLD   = "\033[1m";
const std::string ANSI_RED    = "\033[31m";
const std::string ANSI_GREEN  = "\033[32m";
const std::string ANSI_YELLOW = "\033[33m";
const std::string ANSI_BLUE   = "\033[34m";
const std::string ANSI_CYAN   = "\033[36m";

void ConsoleView::printHeader(const std::string& strTitle) {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n======================================================\n";
    std::cout << "  " << strTitle << "\n";
    std::cout << "======================================================\n" << ANSI_RESET;
}

void ConsoleView::printPrompt(const std::string& strPrompt) {
    std::cout << ANSI_CYAN << strPrompt << ANSI_RESET << std::flush;
}

void ConsoleView::printError(const std::string& strMsg) {
    std::cout << ANSI_RED << ANSI_BOLD << "[LOI] " << strMsg << ANSI_RESET << "\n";
}

void ConsoleView::printSuccess(const std::string& strMsg) {
    std::cout << ANSI_GREEN << ANSI_BOLD << "[THANH CONG] " << strMsg << ANSI_RESET << "\n";
}

void ConsoleView::printWarning(const std::string& strMsg) {
    std::cout << ANSI_YELLOW << ANSI_BOLD << "[CANH BAO] " << strMsg << ANSI_RESET << "\n";
}

void ConsoleView::printInfo(const std::string& strMsg) {
    std::cout << ANSI_BLUE << ANSI_BOLD << "[THONG TIN] " << strMsg << ANSI_RESET << "\n";
}

std::string ConsoleView::inputPassword(const std::string& strPrompt, std::istream* pInStream) {
    std::cout << strPrompt << std::flush;

    if (pInStream != nullptr) {
        std::string strPass = "";
        if (std::getline(*pInStream, strPass)) {
            return strPass;
        }
        return "";
    }

#ifndef _WIN32
    if (!isatty(STDIN_FILENO)) {
        std::string strPass = "";
        if (std::getline(std::cin, strPass)) {
            size_t s = strPass.find_first_not_of(" \t\r\n");
            if (s != std::string::npos) {
                return strPass.substr(s, strPass.find_last_not_of(" \t\r\n") - s + 1);
            }
            return "";
        }
        return "";
    }
#endif

    std::string strPassword = "";

#ifdef _WIN32
    while (true) {
        int iRaw = _getch();
        if (iRaw == 0 || iRaw == 224) {
            _getch();
            continue;
        }
        char cKey = static_cast<char>(iRaw);

        if (cKey == '\r' || cKey == '\n') {
            std::cout << "\n";
            break;
        }

        if (cKey == '\b') {
            if (!strPassword.empty()) {
                strPassword.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        if (cKey < 32 || cKey > 126) {
            continue;
        }

        // Ky tu hop le: gioi han toi da 32 ky tu phong chong tan cong Denial of Service (DoS)
        if (strPassword.length() < 32) {
            strPassword.push_back(cKey);
            std::cout << '*' << std::flush;
        }
    }
#else
    LinuxTerminalRawGuard rawGuard;
    while (true) {
        char cKey = 0;
        if (read(STDIN_FILENO, &cKey, 1) <= 0) {
            break;
        }

        // Xu ly chuoi ANSI escape sequences 
        if (cKey == 27) {
            struct termios drainTerm;
            if (tcgetattr(STDIN_FILENO, &drainTerm) >= 0) {
                drainTerm.c_cc[VMIN] = 0;
                drainTerm.c_cc[VTIME] = 1; // 100ms
                tcsetattr(STDIN_FILENO, TCSANOW, &drainTerm);

                char cSeq[8];
                while (read(STDIN_FILENO, cSeq, sizeof(cSeq)) > 0) {

                }

                drainTerm.c_cc[VMIN] = 1;
                drainTerm.c_cc[VTIME] = 0;
                tcsetattr(STDIN_FILENO, TCSANOW, &drainTerm);
            }
            continue;
        }

        // Xu ly phim Enter de hoan thanh
        if (cKey == '\r' || cKey == '\n') {
            std::cout << "\n";
            break;
        }

        // Xu ly phim Backspace
        if (cKey == '\b' || cKey == 127) {
            if (!strPassword.empty()) {
                strPassword.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        // Bo qua cac phim dieu khien khac
        if (cKey < 32 || cKey > 126) {
            continue;
        }

        // gioi han toi da 32 ky tu 
        if (strPassword.length() < 32) {
            strPassword.push_back(cKey);
            std::cout << '*' << std::flush;
        }
    }
#endif

    return strPassword;
}

std::string ConsoleView::inputPin(const std::string& strPrompt) {
    return ConsoleView::inputPassword(strPrompt);
}

std::string ConsoleView::formatMoney(long lAmount) {
    std::string strNum = std::to_string(lAmount);
    std::string strFormatted = "";
    int iCount = 0;
    int iStart = (lAmount < 0) ? 1 : 0;
    for (int i = static_cast<int>(strNum.length()) - 1; i >= iStart; --i) {
        strFormatted = strNum[i] + strFormatted;
        iCount++;
        if (iCount % 3 == 0 && i > iStart) {
            strFormatted = "," + strFormatted;
        }
    }
    if (lAmount < 0) {
        strFormatted = "-" + strFormatted;
    }
    return strFormatted;
}

long ConsoleView::inputMoney(const std::string& strPrompt, std::istream& inStream) {
    while (true) {
        std::cout << strPrompt << std::flush;
        std::string strLine;
        if (!std::getline(inStream, strLine)) {
            ConsoleView::printWarning("Luong nhap lieu da ket thuc (EOF).");
            return 0;
        }

        // Trim khoang trang
        size_t iStart = strLine.find_first_not_of(" \t\r\n");
        if (iStart == std::string::npos) {
            ConsoleView::printError("So tien khong duoc de trong. Vui long nhap lai!");
            continue;
        }
        size_t iEnd = strLine.find_last_not_of(" \t\r\n");
        std::string strTrimmed = strLine.substr(iStart, iEnd - iStart + 1);

        bool bAllDigits = true;
        for (char c : strTrimmed) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                bAllDigits = false;
                break;
            }
        }

        if (!bAllDigits) {
            ConsoleView::printError("Dinh dang khong hop le (chi chap nhan so nguyen duong). Vui long nhap lai!");
            continue;
        }

        try {
            long lAmount = std::stol(strTrimmed);
            return lAmount;
        } catch (const std::out_of_range&) {
            ConsoleView::printError("So tien vuot qua gioi han he thong co the xu ly (Toi da: " +
                                    formatMoney(MAX_BALANCE_DEFAULT) + " VND). Vui long nhap lai!");
        } catch (...) {
            ConsoleView::printError("Loi chuyen doi du lieu so. Vui long nhap lai!");
        }
    }
}

long ConsoleView::inputMoneyRange(const std::string& strPrompt,
                                  long lMin,
                                  long lMax,
                                  const std::string& strCurrency,
                                  std::istream& inStream) {
    while (true) {
        std::cout << strPrompt << std::flush;
        std::string strLine;
        if (!std::getline(inStream, strLine)) {
            ConsoleView::printWarning("Luong nhap lieu da ket thuc (EOF).");
            return 0;
        }

        size_t iStart = strLine.find_first_not_of(" \t\r\n");
        if (iStart == std::string::npos) {
            ConsoleView::printError("So tien khong duoc de trong. Vui long nhap lai!");
            continue;
        }
        size_t iEnd = strLine.find_last_not_of(" \t\r\n");
        std::string strTrimmed = strLine.substr(iStart, iEnd - iStart + 1);

        bool bAllDigits = true;
        for (char c : strTrimmed) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                bAllDigits = false;
                break;
            }
        }

        if (!bAllDigits) {
            ConsoleView::printError("Dinh dang khong hop le (chi chap nhan so nguyen duong). Vui long nhap lai!");
            continue;
        }

        try {
            long lAmount = std::stol(strTrimmed);
            if (lAmount == 0) {
                return 0; // Cho phep nhap 0 de thoat/huy
            }
            if (lAmount < lMin || lAmount > lMax) {
                ConsoleView::printError("So tien vuot ngoai khoang cho phep! Gioi han: Tu " +
                                        formatMoney(lMin) + " den " + formatMoney(lMax) + " " + strCurrency + ". Vui long nhap lai!");
                continue;
            }
            return lAmount;
        } catch (const std::out_of_range&) {
            ConsoleView::printError("So tien qua lon vuot qua gioi han he thong! Khoang cho phep: Tu " +
                                    formatMoney(lMin) + " den " + formatMoney(lMax) + " " + strCurrency + ". Vui long nhap lai!");
        } catch (...) {
            ConsoleView::printError("Loi chuyen doi du lieu so. Vui long nhap lai!");
        }
    }
}

int ConsoleView::inputMenuChoice(int iMin, int iMax, const std::string& strPrompt, std::istream& inStream) {
    while (true) {
        std::cout << strPrompt << std::flush;
        std::string strLine;
        if (!std::getline(inStream, strLine)) {
            return iMin;
        }

        size_t iStart = strLine.find_first_not_of(" \t\r\n");
        if (iStart == std::string::npos) {
            ConsoleView::printError("Lua chon khong duoc de trong!");
            continue;
        }
        size_t iEnd = strLine.find_last_not_of(" \t\r\n");
        std::string strTrimmed = strLine.substr(iStart, iEnd - iStart + 1);

        bool bStrictDigits = true;
        size_t iCheckStart = 0;
        if (strTrimmed[0] == '-' || strTrimmed[0] == '+') {
            if (strTrimmed.length() == 1) {
                bStrictDigits = false;
            }
            iCheckStart = 1;
        }
        for (size_t i = iCheckStart; i < strTrimmed.length(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(strTrimmed[i]))) {
                bStrictDigits = false;
                break;
            }
        }

        if (!bStrictDigits) {
            ConsoleView::printError("Vui long nhap so hop le!");
            continue;
        }

        try {
            size_t idx = 0;
            int iChoice = std::stoi(strTrimmed, &idx);
            if (idx == strTrimmed.length()) {
                if (iChoice >= iMin && iChoice <= iMax) {
                    return iChoice;
                }
                ConsoleView::printError("Lua chon ngoai pham vi hop le! Vui long chon lai.");
            } else {
                ConsoleView::printError("Vui long nhap so hop le!");
            }
        } catch (...) {
            ConsoleView::printError("Vui long nhap so hop le!");
        }
    }
}

std::string ConsoleView::inputLine(const std::string& strPrompt) {
    std::cout << strPrompt << std::flush;
    std::string strResult;
    std::getline(std::cin, strResult);
    return strResult;
}

bool ConsoleView::confirmAction(const std::string& strPrompt) {
    std::cout << ANSI_YELLOW << strPrompt << " (y/n): " << ANSI_RESET << std::flush;
    std::string strInput;
    std::getline(std::cin, strInput);
    return (!strInput.empty() && (strInput[0] == 'y' || strInput[0] == 'Y'));
}

void ConsoleView::printMainMenu() {
    ConsoleView::printHeader("HE THONG ATM NGAN HANG (BANKING SIMULATION)");
    std::cout << "  1. Dang nhap Quan tri vien (Admin)\n";
    std::cout << "  2. Dang nhap Khach hang (User)\n";
    std::cout << "  0. Thoat chuong trinh\n";
    std::cout << "------------------------------------------------------\n";
}

void ConsoleView::printAdminMenu() {
    ConsoleView::printHeader("PHAN HE QUAN TRI VIEN (ADMIN MODULE)");
    std::cout << "  1. Xem danh sach the tu\n";
    std::cout << "  2. Them tai khoan the moi\n";
    std::cout << "  3. Xoa tai khoan the\n";
    std::cout << "  4. Mo khoa the bi khoa\n";
    std::cout << "  0. Dang xuat (Quay lai menu chinh)\n";
    std::cout << "------------------------------------------------------\n";
}

void ConsoleView::printUserMenu() {
    ConsoleView::printHeader("PHAN HE KHACH HANG (USER MODULE)");
    std::cout << "  1. Xem thong tin tai khoan\n";
    std::cout << "  2. Rut tien\n";
    std::cout << "  3. Chuyen tien\n";
    std::cout << "  4. Xem lich su giao dich\n";
    std::cout << "  5. Doi ma PIN\n";
    std::cout << "  0. Tra the - Dang xuat\n";
    std::cout << "------------------------------------------------------\n";
}

void ConsoleView::clearScreen() {
    std::cout << "\033[2J\033[1;1H" << std::flush;
}

void ConsoleView::pauseScreen() {
    std::cout << "\nNhan [Enter] de tiep tuc..." << std::flush;
    std::string strDummy;
    std::getline(std::cin, strDummy);
}

void ConsoleView::printCardTableHeader() {
    std::cout << ANSI_CYAN << "\n+----+----------------+--------+----------------+\n";
    std::cout << "| STT| MA SO THE (ID) | MA PIN | TRANG THAI     |\n";
    std::cout << "+----+----------------+--------+----------------+\n" << ANSI_RESET;
}

void ConsoleView::printCardRow(const std::string& strId,
                             const std::string& strPin,
                             bool bIsLocked) {
    std::cout << "|    | " << std::left << std::setw(15) << strId
              << "| " << std::setw(7) << strPin << "| ";
    if (bIsLocked) {
        std::cout << ANSI_RED << std::setw(15) << "Bi Khoa" << ANSI_RESET;
    } else {
        std::cout << ANSI_GREEN << std::setw(15) << "Hoat Dong" << ANSI_RESET;
    }
    std::cout << "|\n";
}

void ConsoleView::printCardTableFooter() {
    std::cout << ANSI_CYAN << "+----+----------------+--------+----------------+\n" << ANSI_RESET;
}

void ConsoleView::printReceipt(const std::string& strId,
                             const std::string& strAction,
                             long lAmount,
                             long lRemainingBalance,
                             const std::string& strTimestamp,
                             const std::string& strCurrency) {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n============ BIEN LAI GIAO DICH ============\n" << ANSI_RESET;
    std::cout << "  Ma tai khoan : " << strId << "\n";
    std::cout << "  Loai GD      : " << strAction << "\n";
    std::cout << "  So tien GD   : " << ANSI_YELLOW << ANSI_BOLD << lAmount << " " << strCurrency << ANSI_RESET << "\n";
    std::cout << "  So du con lai: " << ANSI_GREEN << ANSI_BOLD << lRemainingBalance << " " << strCurrency << ANSI_RESET << "\n";
    std::cout << "  Thoi gian    : " << strTimestamp << "\n";
    std::cout << ANSI_CYAN << ANSI_BOLD << "============================================\n" << ANSI_RESET;
}

void ConsoleView::displayAccountDetails(const std::string& strId,
                                       const std::string& strName,
                                       long lBalance,
                                       const std::string& strCurrency) {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n===== THONG TIN TAI KHOAN =====\n" << ANSI_RESET;
    std::cout << "  Ma so ID : " << strId << "\n";
    std::cout << "  Chu the  : " << strName << "\n";
    std::cout << "  So du    : " << ANSI_GREEN << ANSI_BOLD << lBalance << " " << strCurrency << ANSI_RESET << "\n";
    std::cout << ANSI_CYAN << "===============================\n" << ANSI_RESET;
}

void ConsoleView::displayAccountInfo(const Account& account) {
    displayAccountDetails(account.getId(), account.getName(), account.getBalance(), account.getCurrency());
}
