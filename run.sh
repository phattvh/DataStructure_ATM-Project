#!/usr/bin/env bash
# ==============================================================================
# SCRIPT KHOI CHAY & BIEN DICH TU DONG CHO HE THONG ATM (LINUX / MACOS / WSL)
# Tu dong kiem tra moi truong: Ho tro ca khi CO hoac KHONG CO cong cu 'make'
# ==============================================================================

set -e

# Chuyen den thu muc chua script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "======================================================================"
echo "    HE THONG MO PHONG CAY ATM NGAN HANG (DATASTRUCTURE_ATM-PROJECT)    "
echo "======================================================================"

# Kiem tra trinh bien dich g++
if ! command -v g++ &> /dev/null; then
    echo "[LOI] Khong tim thay trinh bien dich g++ tren he thong!"
    echo "Vui long cai dat g++ (C++17 tro len) de tiep tuc: sudo apt install g++"
    exit 1
fi

MODE="${1:-run}"

if [ "$MODE" = "test" ]; then
    echo "[INFO] Dang tien hanh bien dich va chay toan bo Test Suites..."
    if command -v make &> /dev/null; then
        make test
    else
        mkdir -p build
        echo "[INFO] Bien dich va chay test_runner truc tiep qua g++..."
        g++ -std=c++17 -Wall -Wextra -Iinclude src/ConsoleView.cpp src/Card.cpp src/Account.cpp src/UserController.cpp src/Admin.cpp src/Transaction.cpp src/FileService.cpp src/AtmController.cpp src/AdminController.cpp test/test_phase_3.cpp -o build/test_phase_3
        ./build/test_phase_3
        g++ -std=c++17 -Wall -Wextra -Iinclude src/ConsoleView.cpp src/Card.cpp src/Account.cpp src/UserController.cpp src/Admin.cpp src/Transaction.cpp src/FileService.cpp src/AtmController.cpp src/AdminController.cpp test/test_memory_leak.cpp -o build/test_memory_leak
        ./build/test_memory_leak
    fi
    exit 0
fi

# Che do chay ung dung chinh
echo "[1/2] Dang kiem tra va bien dich ung dung..."

if command -v make &> /dev/null; then
    echo "[INFO] Phat hien cong cu 'make' -> Su dung Makefile de build..."
    make app
    BIN_PATH="./build/atm_app"
else
    echo "[THONG BAO] He thong khong co san 'make' -> Tu dong bien dich truc tiep bang g++..."
    mkdir -p build
    g++ -std=c++17 -Wall -Wextra -Iinclude src/ConsoleView.cpp src/Card.cpp src/Account.cpp src/UserController.cpp src/Admin.cpp src/Transaction.cpp src/FileService.cpp src/AtmController.cpp src/AdminController.cpp src/main.cpp -o build/atm_app
    BIN_PATH="./build/atm_app"
fi

echo "[2/2] Bien dich hoan tat! Dang khoi chay ung dung ATM..."
echo "----------------------------------------------------------------------"
"$BIN_PATH"
