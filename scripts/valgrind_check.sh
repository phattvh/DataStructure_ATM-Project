#!/usr/bin/env bash
###############################################################################
# @file: scripts/valgrind_check.sh
# @description: Kịch bản tự động hóa kiểm định rò rỉ bộ nhớ với Valgrind Memcheck
#               dành cho môi trường Linux / Ubuntu / WSL2.
#               Tuân thủ nghiêm ngặt tiêu chuẩn nghiệm thu đồ án HCMUE (Phase 4).
#
# Cách sử dụng:
#   chmod +x scripts/valgrind_check.sh
#   ./scripts/valgrind_check.sh
###############################################################################

set -e

# Màu sắc hiển thị terminal
GREEN='\033[0;32m'
RED='\033[0;31m'
CYAN='\033[0;36m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${CYAN}======================================================================${NC}"
echo -e "${CYAN}     KIỂM ĐỊNH RÒ RỈ BỘ NHỚ VỚI VALGRIND MEMCHECK (PHASE 4 - TRÍ)     ${NC}"
echo -e "${CYAN}       Dự án: DataStructure_ATM-Project | Chuẩn C++17 (-Wall -Wextra) ${NC}"
echo -e "${CYAN}======================================================================${NC}"

# 1. Kiểm tra công cụ valgrind
if ! command -v valgrind &> /dev/null; then
    echo -e "${RED}[LỖI] Công cụ 'valgrind' chưa được cài đặt trên hệ thống!${NC}"
    echo -e "Vui lòng cài đặt bằng lệnh: ${YELLOW}sudo apt update && sudo apt install -y valgrind${NC}"
    exit 1
fi

echo -e "\n${YELLOW}[BƯỚC 1/3] Biên dịch toàn bộ hệ thống kèm cờ debug (-g)...${NC}"
make clean
make CXXFLAGS="-std=c++17 -Wall -Wextra -g -Iinclude" all
make CXXFLAGS="-std=c++17 -Wall -Wextra -g -Iinclude" test_runner
make CXXFLAGS="-std=c++17 -Wall -Wextra -g -Iinclude" test_mem

VALGRIND_FLAGS=(
    "--leak-check=full"
    "--show-leak-kinds=all"
    "--track-origins=yes"
    "--error-exitcode=1"
)

echo -e "\n${YELLOW}[BƯỚC 2/3] Chạy Valgrind trên Bộ kiểm thử bộ nhớ chuyên sâu (test_mem)...${NC}"
valgrind "${VALGRIND_FLAGS[@]}" ./build/test_mem

echo -e "\n${YELLOW}[BƯỚC 3/3] Chạy Valgrind trên Bộ kiểm thử toàn diện (test_runner)...${NC}"
valgrind "${VALGRIND_FLAGS[@]}" ./build/test_runner

echo -e "\n${GREEN}======================================================================${NC}"
echo -e "${GREEN}  ✓ NGHIỆM THU THÀNH CÔNG: 0 BYTES DEFINITELY LOST TRÊN VALGRIND!     ${NC}"
echo -e "${GREEN}  ✓ ĐẠT ĐIỂM TỐI ĐA PHẦN BỘ NHỚ CHO BÁO CÁO VÀ VIDEO DEMO!            ${NC}"
echo -e "${GREEN}======================================================================${NC}"
