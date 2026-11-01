@echo off
setlocal enabledelayedexpansion
title KIEM TRA VI DIEU KHIEN STM32 (TEST SONG / CHET)
chcp 65001 >nul

set "PROJECT_DIR=D:\dowloands\House-Gost-main\House-Gost-main"
if not exist "!PROJECT_DIR!\platformio.ini" (
    if exist "%~dp0platformio.ini" set "PROJECT_DIR=%~dp0"
    if exist "%~dp0House-Gost-main\platformio.ini" set "PROJECT_DIR=%~dp0House-Gost-main"
)

cd /d "!PROJECT_DIR!"

cls
echo ======================================================================
echo          CHUONG TRINH TEST KIEM TRA STM32 (BLUE PILL F103C8)
echo ======================================================================
echo.
echo  [CHUAN BI TRUOC KHI TEST]:
echo   1. Cam ST-Link vao cong USB may tinh.
echo   2. Ket noi 4 day giua ST-Link va STM32:
echo      - SWDIO ---^> SWDIO
echo      - SWCLK ---^> SWCLK
echo      - GND   ---^> GND
echo      - 3.3V  ---^> 3.3V (hoac 5V neu cap chan 5V STM32)
echo   3. Jumper BOOT0 va BOOT1 tren STM32 dat o vi tri so 0 (sat bo mach).
echo.
echo  [QUY TRINH TEST STM32 SONG CHET VA GPIO]:
echo   1. Khoi dong 3s:   LED PC13 chop CHAT (500ms) - Xac nhan khoi dong OK
echo   2. Sau 3s:         LED PC13 chop NHANH (100ms) - STM32 song, san sang test
echo   3. Test GPIO:      Cam 1 day vao GND, cham vao bat ky chan GPIO (PA/PB/PC)
echo                      -^> Neu chan tot: LED PC13 se TAT trong luc cham/keo!
echo                      -^> Nha khoi GND: LED PC13 tiep tuc chop nhanh.
echo   - Cong Serial 115200 (PA9) xuat log chi tiet chan vua keo GND.
echo ======================================================================
echo.
pause

echo.
echo Dang tien hanh nap code kiem tra qua ST-Link...
echo.

"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_STM32_CHIP_ALIVE -t upload

if errorlevel 1 (
    echo.
    echo ======================================================================
    echo   [X] NAP THAT BAI!
    echo   Cac nguyen nhan va huong xu ly:
    echo   1. Long day hoac cam nham chan SWDIO / SWCLK / GND / 3.3V.
    echo   2. Jumper BOOT0 dang o muc 1 - Hay chuyen ve muc 0.
    echo   3. Chip bi treo nap: GIU NUT RESET tren bo STM32 -^> Chay lai file
    echo      nay -^> Khi man hinh hien chu "ST-Link / Connecting" thi NHA NUT RESET.
    echo   4. STM32 bi chet / chap nguon (chip nong ran).
    echo ======================================================================
) else (
    echo.
    echo ======================================================================
    echo   [V] NAP CODE THANH CONG!
    echo   Vi dieu khien STM32 CON HOAT DONG TOT (CHIP ALIVE)!
    echo   - 3 giay dau: LED PC13 chop cham.
    echo   - Sau 3 giay: LED PC13 chop nhanh lien tuc.
    echo   - Test chan:  Lay day cham chan bat ky vao GND -> LED PC13 se TAT!
    echo ======================================================================
)

echo.
pause

