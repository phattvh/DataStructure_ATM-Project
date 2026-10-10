/******************************************************************************
 * @file: main.cpp
 * @description: Diem vao chinh (Entry Point) cua ung dung mo phong ATM
 *               Khoi tao du lieu mau va khoi dong Bo dieu phoi AtmController.
 *               (Tuan thu C++ Coding Standard V2 - Nhiem vu Phase 2 Member C)
 ******************************************************************************/

#include "AtmController.h"
#include "FileService.h"

int main() {
    FileService::initSampleData();

    AtmController atmApp;

    atmApp.run();

    return 0;
}
