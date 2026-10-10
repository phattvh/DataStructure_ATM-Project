@echo off
rem ==============================================================================
rem SCRIPT KHOI CHAY & BIEN DICH CHO WINDOWS (KHONG CAN CAI MAKE)
rem Ho tro click dup truc tiep tren Windows hoac chay tu CMD / PowerShell
rem ==============================================================================

title ATM Simulation Project - HCMUE
color 0A

echo ======================================================================
echo    HE THONG MO PHONG CAY ATM NGAN HANG (DATASTRUCTURE_ATM-PROJECT)    
echo ======================================================================

where g++ >nul 2>nul
if %errorlevel% neq 0 (
    color 0C
    echo [LOI] Khong tim thay trinh bien dich g++ tren he thong Windows!
    echo Vui long cai dat MinGW-w64 hoac MSYS2 va them g++ vao bien moi truong PATH.
    echo.
    pause
    exit /b 1
)

if not exist build mkdir build

echo [1/2] Dang bien dich ung dung bang g++ (C++17)...
g++ -std=c++17 -Wall -Wextra -Iinclude src\ConsoleView.cpp src\Card.cpp src\Account.cpp src\UserController.cpp src\Admin.cpp src\Transaction.cpp src\FileService.cpp src\AtmController.cpp src\AdminController.cpp src\main.cpp -o build\atm_app.exe

if %errorlevel% neq 0 (
    color 0C
    echo.
    echo [LOI] Qua trinh bien dich that bai!
    pause
    exit /b 1
)

echo [2/2] Bien dich thanh cong! Dang khoi dong ung dung ATM...
echo ----------------------------------------------------------------------
echo.
build\atm_app.exe

pause
