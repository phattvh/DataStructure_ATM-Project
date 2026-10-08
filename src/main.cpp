/******************************************************************************
 * @file: main.cpp
 * @description: Diem vao chinh (Entry Point) cua ung dung mo phong ATM
 *               Khoi tao du lieu mau va khoi dong Bo dieu phoi AtmController.
 *               (Tuan thu C++ Coding Standard V2 - Nhiem vu Phase 2 Member C)
 ******************************************************************************/

#include "AtmController.h"
#include "FileService.h"

int main() {
    // 1. Khoi tao thu muc va du lieu mau ban dau neu chua ton tai (Auto-Recovery)
    FileService::initSampleData();

    // 2. Khoi tao bo dieu phoi trung tam
    AtmController atmApp;

    // 3. Chay vong lap ung dung
    atmApp.run();

    return 0;
}
