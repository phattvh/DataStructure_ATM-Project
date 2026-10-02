#include "ConsoleView.h"
#include <iostream>
#include <iomanip>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
static char _getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        return 0;
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        return 0;
    if (read(0, &buf, 1) < 0)
        return 0;
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        return 0;
    return buf;
}
#endif

// ANSI Escape Codes
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
    std::cout << ANSI_BLUE << "[THONG TIN] " << strMsg << ANSI_RESET << "\n";
}

std::string ConsoleView::inputPassword(const std::string& strPrompt) {
    std::cout << strPrompt;
    std::string strPass = "";
    while (true) {
        char ch = _getch();
        if (ch == '\r' || ch == '\n') {
            std::cout << "\n";
            break;
        } else if (ch == '\b' || ch == 127) { // Backspace
            if (!strPass.empty()) {
                strPass.pop_back();
                std::cout << "\b \b";
            }
        } else if (ch == 3) { // Ctrl+C
            std::cout << "\n";
            return "";
        } else if (ch >= 32 && ch <= 126) {
            strPass.push_back(ch);
            std::cout << "*";
        }
    }
    return strPass;
}

long ConsoleView::inputMoney(const std::string& strPrompt) {
    long lAmount = 0;
    while (true) {
        std::cout << strPrompt;
        if (std::cin >> lAmount) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (lAmount >= 0) {
                return lAmount;
            }
            ConsoleView::printError("So tien khong the am! Vui long nhap lai.");
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ConsoleView::printError("Nhap sai dinh dang so! Vui long nhap lai.");
        }
    }
}

int ConsoleView::inputMenuChoice(int iMin, int iMax, const std::string& strPrompt) {
    int iChoice = 0;
    while (true) {
        std::cout << strPrompt;
        if (std::cin >> iChoice) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (iChoice >= iMin && iChoice <= iMax) {
                return iChoice;
            }
            ConsoleView::printError("Lua chon ngoai pham vi! Vui long chon lai.");
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ConsoleView::printError("Vui long nhap so hop le!");
        }
    }
}

std::string ConsoleView::inputLine(const std::string& strPrompt) {
    std::cout << strPrompt;
    std::string strResult;
    std::getline(std::cin, strResult);
    return strResult;
}

void ConsoleView::printMainMenu() {
    ConsoleView::printHeader("HE THONG ATM NGAN HANG (BANKING SIMULATION)");
    std::cout << "1. Dang nhap Quan tri vien (Admin)\n";
    std::cout << "2. Dang nhap Khach hang (User)\n";
    std::cout << "0. Thoat chuong trinh\n";
    std::cout << "------------------------------------------------------\n";
}

void ConsoleView::printAdminMenu() {
    ConsoleView::printHeader("PHAN HE QUAN TRI VIEN (ADMIN MODULE)");
    std::cout << "1. Xem danh sach the tu\n";
    std::cout << "2. Them tai khoan the moi\n";
    std::cout << "3. Xoa tai khoan the\n";
    std::cout << "4. Mo khoa the bi khoa\n";
    std::cout << "0. Dang xuat (Quay lai menu chinh)\n";
    std::cout << "------------------------------------------------------\n";
}


void ConsoleView::pauseScreen() {
    std::cout << "\nNhan phim bat ky de tiep tuc...";
    _getch();
    std::cout << "\n";
}

void ConsoleView::clearScreen() {
#ifdef _WIN32
    // ANSI clear screen
    std::cout << "\033[2J\033[1;1H";
#else
    std::cout << "\033[2J\033[1;1H";
#endif
}

void ConsoleView::printUserMenu() {
    ConsoleView::printHeader("PHAN HE KHACH HANG (USER MODULE)");
    std::cout << "1. Xem thong tin tai khoan\n";
    std::cout << "2. Rut tien\n";
    std::cout << "3. Chuyen tien\n";
    std::cout << "4. Xem lich su giao dich\n";
    std::cout << "5. Doi ma PIN\n";
    std::cout << "0. Tra the - Dang xuat\n";
    std::cout << "------------------------------------------------------\n";
}

bool ConsoleView::confirmAction(const std::string& strPrompt) {
    std::cout << ANSI_YELLOW << strPrompt << " (y/n): " << ANSI_RESET;
    std::string strInput;
    std::getline(std::cin, strInput);
    return (strInput == "y" || strInput == "Y");
}

std::string ConsoleView::inputPin(const std::string& strPrompt) {
    return ConsoleView::inputPassword(strPrompt);
}
