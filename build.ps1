# Script bien dich nhanh bang PowerShell tren Windows
param (
    [string]$Target = "all"
)

$CXX = "g++"
$CXXFLAGS = @("-std=c++17", "-Wall", "-Wextra", "-Iinclude")
$SRCS = @("src/Admin.cpp", "src/Card.cpp", "src/Account.cpp", "src/Transaction.cpp", "src/FileService.cpp", "src/ConsoleView.cpp", "src/AtmController.cpp")

if ($Target -eq "clean") {
    Write-Host "[CLEAN] Dang xoa cac file thuc thi va file tam..." -ForegroundColor Yellow
    Remove-Item -Path "*.exe", "build/*.o" -Force -ErrorAction SilentlyContinue
    Write-Host "[CLEAN] Hoan tat!" -ForegroundColor Green
    exit 0
}

if ($Target -eq "test") {
    Write-Host "[BUILD] Dang bien dich bo kiem thu test_atm.exe..." -ForegroundColor Cyan
    & $CXX $CXXFLAGS $SRCS test/test_atm.cpp -o test_atm.exe
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[BUILD] Bien dich thanh cong! Dang chay kiem thu..." -ForegroundColor Green
        .\test_atm.exe
    } else {
        Write-Host "[ERROR] Bien dich test that bai!" -ForegroundColor Red
        exit 1
    }
    exit 0
}

Write-Host "[BUILD] Dang bien dich ung dung atm_project.exe..." -ForegroundColor Cyan
& $CXX $CXXFLAGS $SRCS src/main.cpp -o atm_project.exe
if ($LASTEXITCODE -eq 0) {
    Write-Host "[BUILD] Bien dich thanh cong atm_project.exe (0 errors, 0 warnings)!" -ForegroundColor Green
    if ($Target -eq "run") {
        .\atm_project.exe
    }
} else {
    Write-Host "[ERROR] Bien dich that bai!" -ForegroundColor Red
    exit 1
}
