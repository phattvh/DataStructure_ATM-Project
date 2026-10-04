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

/**********************************************************
 * @Description Lop RAII tu dong bat va tat terminal raw mode
 * dam bao khong lam hong trang thai terminal khi ket thuc
 **********************************************************/
class LinuxTerminalRawGuard {
private:
    struct termios _oldTerm;
    bool _bActive;

public:
    LinuxTerminalRawGuard() : _bActive(false) {
        if (tcgetattr(STDIN_FILENO, &this->_oldTerm) >= 0) {
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
        }
    }

    bool isActive() const {
        return this->_bActive;
    }
};
#endif

// Dinh nghia cac ma mau ANSI escape
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

std::string ConsoleView::inputPassword(const std::string& strPrompt, std::istream* pInStream) {
    std::cout << strPrompt << std::flush;

    if (pInStream != nullptr) {
        std::string strPass = "";
        if (std::getline(*pInStream, strPass)) {
            return strPass;
        }
        return "";
    }

    std::string strPassword = "";

#ifdef _WIN32
    while (true) {
        int iRaw = _getch();
        if (iRaw == 0 || iRaw == 224) {
            // Phim chuc nang / mui ten tren Windows: doc bo ma quet
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

        strPassword.push_back(cKey);
        std::cout << '*' << std::flush;
    }
#else
    LinuxTerminalRawGuard rawGuard;
    while (true) {
        char cKey = 0;
        if (read(STDIN_FILENO, &cKey, 1) <= 0) {
            break;
        }

        // Xu ly chuoi thoat ANSI Escape Sequences (phim mui ten, Delete, v.v.)
        if (cKey == 27) {
            struct termios drainTerm;
            if (tcgetattr(STDIN_FILENO, &drainTerm) >= 0) {
                drainTerm.c_cc[VMIN] = 0;
                drainTerm.c_cc[VTIME] = 1; // 100ms
                tcsetattr(STDIN_FILENO, TCSANOW, &drainTerm);

                char cSeq[8];
                while (read(STDIN_FILENO, cSeq, sizeof(cSeq)) > 0) {
                    // Bo qua toan bo byte chuoi escape con lai
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

        // Ky tu hop le: luu vao chuoi va in dau *
        strPassword.push_back(cKey);
        std::cout << '*' << std::flush;
    }
#endif

    return strPassword;
}

long ConsoleView::inputMoney(const std::string& strPrompt, std::istream& inStream) {
    while (true) {
        std::cout << strPrompt << std::flush;
        std::string strLine;
        if (!std::getline(inStream, strLine)) {
            // Gap tin hieu EOF (Ctrl+D hoac luong dong)
            ConsoleView::printWarning("Luong nhap lieu da ket thuc (EOF).");
            return 0;
        }

        // Trim khoang trang dau va cuoi
        size_t iStart = strLine.find_first_not_of(" \t\r\n");
        if (iStart == std::string::npos) {
            ConsoleView::printError("So tien khong duoc de trong. Vui long nhap lai!");
            continue;
        }
        size_t iEnd = strLine.find_last_not_of(" \t\r\n");
        std::string strTrimmed = strLine.substr(iStart, iEnd - iStart + 1);

        // Kiem tra toan bo ky tu phai la chu so (chan so thuc, ky tu dac biet, so am)
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
            ConsoleView::printError("So tien vuot qua gioi han he thong co the xu ly. Vui long nhap lai!");
        } catch (...) {
            ConsoleView::printError("Loi chuyen doi du lieu so. Vui long nhap lai!");
        }
    }
}

void ConsoleView::printAdminMenu() {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n* * * * * * * * * * MENU ADMIN * * * * * * * * * *\n" << ANSI_RESET;
    std::cout << "  1. Xem danh sach tai khoan\n";
    std::cout << "  2. Them tai khoan\n";
    std::cout << "  3. Xoa tai khoan\n";
    std::cout << "  4. Mo khoa tai khoan\n";
    std::cout << "  5. Thoat\n";
    std::cout << ANSI_CYAN << ANSI_BOLD << "* * * * * * * * * * * * * * * * * * * * * * * * * *\n" << ANSI_RESET;
}

void ConsoleView::printUserMenu() {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n* * * * * * * * * * MENU KHACH HANG * * * * * * * *\n" << ANSI_RESET;
    std::cout << "  1. Xem thong tin tai khoan\n";
    std::cout << "  2. Rut tien\n";
    std::cout << "  3. Chuyen tien\n";
    std::cout << "  4. Doi ma PIN\n";
    std::cout << "  5. Dang xuat\n";
    std::cout << ANSI_CYAN << ANSI_BOLD << "* * * * * * * * * * * * * * * * * * * * * * * * * *\n" << ANSI_RESET;
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
                             const std::string& strTimestamp) {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n============ BIEN LAI GIAO DICH ============\n" << ANSI_RESET;
    std::cout << "  Ma tai khoan : " << strId << "\n";
    std::cout << "  Loai GD      : " << strAction << "\n";
    std::cout << "  So tien GD   : " << ANSI_YELLOW << ANSI_BOLD << lAmount << " VND" << ANSI_RESET << "\n";
    std::cout << "  So du con lai: " << ANSI_GREEN << ANSI_BOLD << lRemainingBalance << " VND" << ANSI_RESET << "\n";
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

void ConsoleView::pauseScreen() {
    std::cout << "\nNhan [Enter] de tiep tuc...";
    std::string strDummy;
    std::getline(std::cin, strDummy);
}
