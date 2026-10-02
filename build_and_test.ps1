Write-Host "=================================================" -ForegroundColor Cyan
Write-Host " BIEN DICH VA CHAY KIEM THU MODULE MEMBER A (TUAN)" -ForegroundColor Cyan
Write-Host "=================================================" -ForegroundColor Cyan

g++ -std=c++17 -Wall -Wextra -Iinclude src/Card.cpp src/Account.cpp src/ConsoleView.cpp src/UserController.cpp test/test_member_a.cpp -o test_member_a.exe

if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] Bien dich thanh cong!" -ForegroundColor Green
    .\test_member_a.exe
    Remove-Item test_member_a.exe -ErrorAction SilentlyContinue
} else {
    Write-Host "[ERROR] Bien dich that bai!" -ForegroundColor Red
}
