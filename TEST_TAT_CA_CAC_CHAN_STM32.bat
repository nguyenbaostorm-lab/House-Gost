@echo off
setlocal enabledelayedexpansion
title TEST TOAN BO CAC CHAN GPIO CUA STM32 (BLUE PILL F103C8)
chcp 65001 >nul

set "PROJECT_DIR=D:\dowloands\House-Gost-main\House-Gost-main"
if not exist "!PROJECT_DIR!\platformio.ini" (
    if exist "%~dp0platformio.ini" set "PROJECT_DIR=%~dp0"
    if exist "%~dp0House-Gost-main\platformio.ini" set "PROJECT_DIR=%~dp0House-Gost-main"
)

cd /d "!PROJECT_DIR!"

cls
echo ======================================================================
echo       CHUONG TRINH TEST TOAN BO CAC CHAN GPIO STM32 BLUE PILL
echo ======================================================================
echo.
echo  [DANH SACH CAC CHAN TEST]:
echo   - Cong PA: PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA11, PA12, PA15
echo   - Cong PB: PB0, PB1, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15
echo   - Cong PC: PC13 (LED onboard), PC14, PC15
echo   (Giữ nguyên PA13-SWDIO, PA14-SWCLK de nap ST-Link; PA9/PA10 bao Serial)
echo.
echo  [QUY TRINH VA CACH DO KIEM TRA]:
echo   1. Cam ST-Link nap code nay vao STM32.
echo   2. Sau khi nap xong, TOAN BO CAC CHAN se xuat dien ap:
echo      + 1 GIAY XUAT 3.3V (Muc CAO - HIGH) - LED PC13 SANG
echo      + 1 GIAY XUAT 0.0V (Muc THAP - LOW) - LED PC13 TAT
echo   3. Cach kiem tra:
echo      - Lay DONG HO VOM (de thang do dien ap DC 20V):
echo        * Que den cam co dinh vao chan GND cua STM32.
echo        * Que do cham lan luot vao TUNG CHAN GPIO tren mach.
echo        * Dong ho nhay deu: 3.3V -^> 0V -^> 3.3V -^> 0V la CHAN CON SONG TOT!
echo        * Neu chan nao bi 0V hoac 3.3V dung im khong nhay la CHAN BI HONG!
echo      - Hoac cam 1 bong LED (+ dien tro 1k) tu GND vao chan bat ky de xem nhap nhay.
echo ======================================================================
echo.
pause

echo.
echo Dang tien hanh nap code test toan bo chan qua ST-Link...
echo.

"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_ALL_PINS -t upload

if errorlevel 1 (
    echo.
    echo ======================================================================
    echo   [X] NAP THAT BAI!
    echo   1. Kiem tra lai day cam ST-Link va tiep xuc chan.
    echo   2. Neu chip bi treo nap: GIU nut RESET tren STM32 -^> Chay lai file
    echo      nay -^> Khi hien chu Connecting thi NHA nut RESET.
    echo ======================================================================
) else (
    echo.
    echo ======================================================================
    echo   [V] NAP CODE TEST TOAN BO CHAN THANH CONG!
    echo   Ban co the bat dau dung que do VOM cham tung chan de kiem tra!
    echo ======================================================================
)

echo.
pause

