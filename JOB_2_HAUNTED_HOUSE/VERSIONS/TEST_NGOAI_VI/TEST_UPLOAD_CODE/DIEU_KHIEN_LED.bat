@echo off
chcp 65001 >nul
title BẢNG ĐIỀU KHIỂN TEST ĐÈN LED PC13 - STM32

:MENU
cls
echo ========================================================
echo        BẢNG ĐIỀU KHIỂN TEST ĐÈN LED PC13 (STM32)
echo ========================================================
echo.
echo   [1] BẬT SÁNG ĐÈN LED PC13
echo   [0] TẮT ĐÈN LED PC13
echo   [Q] Thoát
echo.
echo ========================================================
set /p choice="Nhập lựa chọn của bạn (1 hoặc 0): "

if "%choice%"=="1" goto BAT
if "%choice%"=="0" goto TAT
if /i "%choice%"=="q" exit
goto MENU

:BAT
echo.
echo Đang gửi lệnh '1' tới STM32 (COM21)...
powershell -NoProfile -Command "$p=New-Object System.IO.Ports.SerialPort 'COM21',9600;$p.DtrEnable=$true;$p.RtsEnable=$true;try{$p.Open();$p.Write('1');Start-Sleep -m 200;$p.Close();Write-Host '>>> ĐÃ BẬT SÁNG ĐÈN LED PC13 THÀNH CÔNG! <<<' -ForegroundColor Green}catch{Write-Host 'Lỗi kết nối COM21: ' $_.Exception.Message -ForegroundColor Red}"
echo.
pause
goto MENU

:TAT
echo.
echo Đang gửi lệnh '0' tới STM32 (COM21)...
powershell -NoProfile -Command "$p=New-Object System.IO.Ports.SerialPort 'COM21',9600;$p.DtrEnable=$true;$p.RtsEnable=$true;try{$p.Open();$p.Write('0');Start-Sleep -m 200;$p.Close();Write-Host '>>> ĐÃ TẮT ĐÈN LED PC13 THÀNH CÔNG! <<<' -ForegroundColor Yellow}catch{Write-Host 'Lỗi kết nối COM21: ' $_.Exception.Message -ForegroundColor Red}"
echo.
pause
goto MENU
