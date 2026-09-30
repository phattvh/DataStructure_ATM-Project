#ifndef CONSOLEVIEW_H_INCLUDED_
#define CONSOLEVIEW_H_INCLUDED_

#include <string>
#include "Common.h"
#include "LinkedList.h"
#include "Card.h"

/**
 * @Description: Lop tien ich xu ly giao dien dong lenh Console, mau sac ANSI, an pass va bat loi
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
    static void displayCardList(const LinkedList<Card>& listCards, const LinkedList<std::string>& listLockedIds);
    static void pauseScreen();
    static void clearScreen();
};

#endif // CONSOLEVIEW_H_INCLUDED_
