#ifndef CONSOLEVIEW_H_INCLUDED_
#define CONSOLEVIEW_H_INCLUDED_

#include <string>
#include <iostream>
#include <iomanip>
#include "Common.h"
#include "Card.h"

/**
 * @Description: Lop tien ich xu ly giao dien dong lenh Console, mau sac ANSI, an pass va bat loi
 * Phan cong: Member A (Tuan) - Presentation Layer & User Experience
 */
class ConsoleView {
public:
    static void printHeader(const std::string& strTitle);
    static void printError(const std::string& strMsg);
    static void printSuccess(const std::string& strMsg);
    static void printWarning(const std::string& strMsg);
    static void printInfo(const std::string& strMsg);

    static std::string inputPassword(const std::string& strPrompt);
    static long inputMoney(const std::string& strPrompt);
    static int inputMenuChoice(int iMin, int iMax, const std::string& strPrompt);
    static std::string inputLine(const std::string& strPrompt);

    static void printMainMenu();
    static void printAdminMenu();
    static void printUserMenu();
    static void pauseScreen();
    static void clearScreen();

    static bool confirmAction(const std::string& strPrompt);
    static std::string inputPin(const std::string& strPrompt);

    /**
     * @Description: Template hien thi danh sach the tu, tuong thich voi moi kieu LinkedList cua Member B (Tri)
     */
    template <typename ListCardType, typename ListLockedType>
    static void displayCardList(const ListCardType& listCards, const ListLockedType& listLockedIds) {
        printHeader("DANH SACH THE TU HE THONG");
        std::cout << std::left 
                  << std::setw(6)  << "STT" 
                  << std::setw(20) << "MA THE (ID)" 
                  << std::setw(12) << "MA PIN" 
                  << std::setw(18) << "TRANG THAI" 
                  << "\n";
        std::cout << "------------------------------------------------------\n";

        int iIndex = 1;
        auto pCur = listCards.getHead();
        while (pCur != nullptr) {
            std::string strId = pCur->_data.getId();
            bool bIsLocked = pCur->_data.isLocked();

            auto pLockCur = listLockedIds.getHead();
            while (pLockCur != nullptr) {
                if (pLockCur->_data == strId) {
                    bIsLocked = true;
                    break;
                }
                pLockCur = pLockCur->_pNext;
            }

            std::string strStatus = bIsLocked ? "\033[31mBi khoa\033[0m" 
                                              : "\033[32mHoat dong\033[0m";

            std::cout << std::left 
                      << std::setw(6)  << iIndex++ 
                      << std::setw(20) << strId 
                      << std::setw(12) << "******" 
                      << std::setw(18) << strStatus 
                      << "\n";

            pCur = pCur->_pNext;
        }
        std::cout << "------------------------------------------------------\n";
    }
};

#endif // CONSOLEVIEW_H_INCLUDED_
