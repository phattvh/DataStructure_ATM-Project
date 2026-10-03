#include "ConsoleView.h"
#include "Account.h"
#include <iostream>
#include <iomanip>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

/**********************************************************
 * @Description Doc 1 ky tu tu terminal Linux khong echo
 * @return Ky tu nguoi dung vua go
 **********************************************************/
static char getchLinux() {
    char cKey = 0;
    struct termios oldTerm;
    if (tcgetattr(STDIN_FILENO, &oldTerm) < 0) {
        return 0;
    }
    struct termios newTerm = oldTerm;
    newTerm.c_lflag &= ~(ICANON | ECHO);
    newTerm.c_cc[VMIN] = 1;
    newTerm.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newTerm) < 0) {
        return 0;
    }
    if (read(STDIN_FILENO, &cKey, 1) < 0) {
        cKey = 0;
    }
    tcsetattr(STDIN_FILENO, TCSADRAIN, &oldTerm);
    return cKey;
}
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

std::string ConsoleView::inputPassword(const std::string& strPrompt) {
    std::cout << strPrompt << std::flush;
    std::string strPassword = "";

    while (true) {
        char cKey = 0;
#ifdef _WIN32
        cKey = static_cast<char>(_getch());
#else
        cKey = getchLinux();
#endif
        // Xu ly phim Enter de hoan thanh
        if (cKey == '\r' || cKey == '\n') {
            std::cout << "\n";
            break;
        }

        // Xu ly phim Backspace de xoa lui
        if (cKey == '\b' || cKey == 127) {
            if (!strPassword.empty()) {
                strPassword.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }

        // Bo qua cac phim dac biet / escape
        if (cKey < 32 || cKey > 126) {
            continue;
        }

        // Ky tu hop le: luu vao chuoi va in dau *
        strPassword.push_back(cKey);
        std::cout << '*' << std::flush;
    }

    return strPassword;
}

long ConsoleView::inputMoney(const std::string& strPrompt) {
    long lAmount = 0;
    while (true) {
        std::cout << strPrompt << std::flush;
        if (std::cin >> lAmount) {
            if (lAmount >= 0) {
                // Xoa phan du con lai tren dong (bao gom ky tu \n)
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return lAmount;
            }
            ConsoleView::printError("So tien khong duoc am. Vui long nhap lai!");
        } else {
            // Bay loi cin.fail() khi nguoi dung nhap chu hoac ky tu dac biet
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ConsoleView::printError("Dinh dang khong hop le (phai la so). Vui long nhap lai!");
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

void ConsoleView::displayAccountInfo(const Account& account) {
    std::cout << ANSI_CYAN << ANSI_BOLD << "\n===== THONG TIN TAI KHOAN =====\n" << ANSI_RESET;
    std::cout << "  Ma so ID : " << account.getId() << "\n";
    std::cout << "  Chu the  : " << account.getName() << "\n";
    std::cout << "  So du    : " << ANSI_GREEN << ANSI_BOLD << account.getBalance() << " " << account.getCurrency() << ANSI_RESET << "\n";
    std::cout << ANSI_CYAN << "===============================\n" << ANSI_RESET;
}

void ConsoleView::pauseScreen() {
    std::cout << "\nNhan [Enter] de tiep tuc...";
    std::string strDummy;
    std::getline(std::cin, strDummy);
}
