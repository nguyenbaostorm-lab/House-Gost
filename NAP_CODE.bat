@echo off
cd /d "%~dp0"
setlocal enabledelayedexpansion
title HE THONG NAP CODE STM32 DA PHIEN BAN (MULTI-VERSION)
chcp 65001 >nul

:MAIN_MENU
cls
echo ========================================================
echo   HE THONG NAP CODE STM32 (PLATFORMIO MULTI-VERSION)
echo ========================================================
echo.

pnputil /enum-devices /connected | findstr /i "STLink" >nul
if errorlevel 1 (
    echo   TRANG THAI ST-LINK: [ CHUA KET NOI - Vui long cam vao USB! ]
) else (
    echo   TRANG THAI ST-LINK: [ DA KET NOI - SAN SANG ]
)

echo.
echo --------------------------------------------------------
echo   CHON LOAI MACH DE NAP CODE:
echo   [1] NAP MACH RELAY (STM32_RELAY)
echo   [2] NAP MACH SOUND (STM32_SOUND)
echo   [3] NAP MACH JUMPSCARE (STM32_JUMPSCARE)
echo.
echo   TIEN ICH PHIEN BAN VA NGOAI VI:
echo   [4] LUU CODE VAO VER MOI NHAT HIEN CO (Ghi de code vao Ver moi nhat)
echo   [5] TAO VERSION MOI (Luu thanh mot phien ban ten moi)
echo   [6] KIEM TRA BIEN DICH (BUILD TEST CA 3 MACH)
echo   [7] QUET LAI CONG ST-LINK
echo   [8] NAP TEST CAM BIEN (TEST_CAM_BIEN: PB0 + PC13)
echo   [9] NAP TEST NUT A6 - RELAY A4 (PA6 den PA4 trong 4s)
echo   [T0] TEST STM32 SONG CHET (3s nhay cham, sau 3s nhay nhanh, keo GND tat PC13)
echo   [T1] TEST TOAN BO CAC CHAN GPIO (Xuat 3.3V/0V tren tat ca chan PA, PB, PC)
echo.
echo   TEST GIAO TIEP UART (SOUND VA JUMPSCARE):
echo   [TS] NAP TEST UART BO SOUND     (PB10 sang B6, PB11 tu B5)
echo   [TJ] NAP TEST UART BO JUMPSCARE (PB5 sang B11, PB6 tu B10)
echo   [0] THOAT
echo ========================================================
set "main_choice="
set /p main_choice="Nhap lua chon: "

if "!main_choice!"=="1" goto CHON_VER_RELAY
if "!main_choice!"=="2" goto CHON_VER_SOUND
if "!main_choice!"=="3" goto CHON_VER_JUMPSCARE
if "!main_choice!"=="4" goto LUU_VAO_VER_MOI_NHAT
if "!main_choice!"=="5" goto TAO_VER_MOI
if "!main_choice!"=="6" goto TEST_BUILD
if "!main_choice!"=="7" goto MAIN_MENU
if "!main_choice!"=="8" goto NAP_TEST_CAM_BIEN
if "!main_choice!"=="9" goto NAP_TEST_NUT_A6_RELAY_A4
if /i "!main_choice!"=="T0" goto NAP_TEST_CHIP_ALIVE
if /i "!main_choice!"=="TEST" goto NAP_TEST_CHIP_ALIVE
if /i "!main_choice!"=="T" goto NAP_TEST_CHIP_ALIVE
if /i "!main_choice!"=="T1" goto NAP_TEST_ALL_PINS
if /i "!main_choice!"=="TS" goto NAP_TEST_UART_SOUND
if /i "!main_choice!"=="TJ" goto NAP_TEST_UART_JUMPSCARE
if "!main_choice!"=="0" exit /b 0
goto MAIN_MENU

:: ==========================================================
:: CHON VERSION CHO TUNG MACH
:: ==========================================================
:CHON_VER_RELAY
set "MACH_NAME=STM32_RELAY"
set "ENV_NAME=NAP_MACH_RELAY"
goto MENU_VERSION

:CHON_VER_SOUND
set "MACH_NAME=STM32_SOUND"
set "ENV_NAME=NAP_MACH_AM_THANH"
goto MENU_VERSION

:CHON_VER_JUMPSCARE
set "MACH_NAME=STM32_JUMPSCARE"
set "ENV_NAME=NAP_MACH_JUMPSCARE"
goto MENU_VERSION

:MENU_VERSION
cls
echo ========================================================
echo   CHON PHIEN BAN CODE DE NAP: [%MACH_NAME%]
echo ========================================================
echo.
echo   [0] BAN CODE HIEN TAI (Dang lam viec truc tiep trong thu muc)
echo.
echo   --- DANH SACH PHIEN BAN TRONG THU MUC VERSIONS ---

set ver_count=0
for /f "delims=" %%D in ('powershell -ExecutionPolicy Bypass -File "%~dp0get_versions.ps1" "!MACH_NAME!"') do (
    set /a ver_count+=1
    set "VER_NAME_!ver_count!=%%D"
    set "VER_PATH_!ver_count!=JOB_2_HAUNTED_HOUSE\VERSIONS\%%D\!MACH_NAME!"
    echo   [!ver_count!] %%D
)

echo.
echo   [B] Quay lai Menu chinh
echo ========================================================
set /p ver_choice="Nhap so Version muon nap (0-!ver_count! hoac B): "

if /i "!ver_choice!"=="B" goto MAIN_MENU
if "!ver_choice!"=="0" (
    set "IS_CURRENT=1"
    goto CHON_CHE_DO_GIO
)

rem Kiem tra so hop le tu 1 den ver_count
for /l %%i in (1,1,!ver_count!) do (
    if "!ver_choice!"=="%%i" (
        set "IS_CURRENT=0"
        set "SELECTED_VER=!VER_NAME_%%i!"
        set "SELECTED_SRC=!VER_PATH_%%i!"
        goto CHON_CHE_DO_GIO
    )
)

echo Lua chon khong hop le! Vui long nhap lai.
pause
goto MENU_VERSION

:: ==========================================================
:: CHON CAU HINH THOI GIAN (GIO MOI / GIO CU)
:: ==========================================================
:CHON_CHE_DO_GIO
rem Neu la mach JUMPSCARE thi khong co chon gio, nap truc tiep:
if "!MACH_NAME!"=="STM32_JUMPSCARE" (
    set "TEN_GIO=MAC DINH"
    if "!IS_CURRENT!"=="1" goto FLASH_CURRENT
    goto FLASH_VERSION
)

cls
echo ========================================================
echo   CHON CAU HINH THOI GIAN DE NAP: [!MACH_NAME!]
if "!IS_CURRENT!"=="1" (
    echo   PHIEN BAN: [BAN CODE HIEN TAI]
) else (
    echo   PHIEN BAN: [!SELECTED_VER!]
)
echo ========================================================
echo.
if "!MACH_NAME!"=="STM32_RELAY" goto SHOW_MENU_RELAY
if "!MACH_NAME!"=="STM32_SOUND" goto SHOW_MENU_SOUND
goto SHOW_MENU_RELAY

:SHOW_MENU_RELAY
echo   [1] BAN GIO MOI - DUNG GIO THUC TE
echo       - Thoi gian quan sat chuyen pha: 3 PHUT 0 GIAY [180 giay]
echo.
echo   [2] BAN GIO CU - SAI GIO TEST NHANH
echo       - Thoi gian quan sat chuyen pha: 10 GIAY [10 giay]
goto CHON_INPUT_GIO

:SHOW_MENU_SOUND
echo   [1] BAN GIO MOI - DUNG GIO THUC TE
echo       - Thoi gian chay thoai A1: 3 PHUT 43 GIAY [223 giay]
echo.
echo   [2] BAN GIO CU - SAI GIO TEST NHANH
echo       - Thoi gian chay thoai A1: 15 GIAY [15 giay]
goto CHON_INPUT_GIO

:CHON_INPUT_GIO
echo.
echo   [B] Quay lai chon phien ban
echo ========================================================
set "gio_choice="
set /p gio_choice="Nhap lua chon [1: Gio moi, 2: Gio cu, B de quay lai]: "

if /i "!gio_choice!"=="B" goto MENU_VERSION

if "!gio_choice!"=="1" goto SET_GIO_MOI
if "!gio_choice!"=="3" goto SET_GIO_MOI
if "!gio_choice!"=="2" goto SET_GIO_CU
if "!gio_choice!"=="4" goto SET_GIO_CU

echo Lua chon khong hop le! Vui long nhap 1 hoac 2.
pause
goto CHON_CHE_DO_GIO

:SET_GIO_MOI
if "!MACH_NAME!"=="STM32_RELAY" (
    set "ENV_NAME=NAP_MACH_RELAY"
    set "TEN_GIO=BAN GIO MOI [DUNG GIO: 3 PHUT 0S]"
) else (
    set "ENV_NAME=NAP_MACH_AM_THANH"
    set "TEN_GIO=BAN GIO MOI [DUNG GIO: 3 PHUT 43S]"
)
if "!IS_CURRENT!"=="1" goto FLASH_CURRENT
goto FLASH_VERSION

:SET_GIO_CU
if "!MACH_NAME!"=="STM32_RELAY" (
    set "ENV_NAME=NAP_MACH_RELAY_GIO_CU"
    set "TEN_GIO=BAN GIO CU [SAI GIO TEST: 10 GIAY]"
) else (
    set "ENV_NAME=NAP_MACH_AM_THANH_GIO_CU"
    set "TEN_GIO=BAN GIO CU [SAI GIO TEST: 15 GIAY]"
)
if "!IS_CURRENT!"=="1" goto FLASH_CURRENT
goto FLASH_VERSION

:FLASH_VERSION
cls
echo ========================================================
echo   DANG LAY CODE VERSION: [!SELECTED_VER!]
echo   CHE DO THOI GIAN:      [!TEN_GIO!]
echo   DICH DEN:              [JOB_2_HAUNTED_HOUSE\!MACH_NAME!]
echo ========================================================
echo.
echo Dang sao chep ma nguon tu thu muc VERSIONS ra thu muc lam viec...
xcopy "!SELECTED_SRC!\*" "JOB_2_HAUNTED_HOUSE\!MACH_NAME!\" /s /e /y /q >nul
if errorlevel 1 (
    echo LOI: Khong the sao chep ma nguon!
    pause
    goto MENU_VERSION
)
echo Da lay code Version [!SELECTED_VER!] ra thanh cong!
echo.
echo ========================================================
echo   DANG TIEN HANH NAP CODE QUA ST-LINK (ENV: !ENV_NAME!)...
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e !ENV_NAME! -t upload
echo.
echo Da thuc hien nap code xong cho [!MACH_NAME!] - Version [!SELECTED_VER!] (!TEN_GIO!)!
pause
goto MAIN_MENU

:FLASH_CURRENT
cls
echo ========================================================
echo   DANG TIEN HANH NAP CODE HIEN TAI (ENV: !ENV_NAME!)...
echo   CHE DO THOI GIAN:      [!TEN_GIO!]
echo   TARGET:                [JOB_2_HAUNTED_HOUSE\!MACH_NAME!]
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e !ENV_NAME! -t upload
echo.
echo Da thuc hien nap code xong cho [!MACH_NAME!] (!TEN_GIO!)!
pause
goto MAIN_MENU

:: ==========================================================
:: LUU CODE HIEN TAI VAO VERSION MOI NHAT HIEN CO
:: ==========================================================
:LUU_VAO_VER_MOI_NHAT
cls
echo ========================================================
echo   LUU CODE DANG SUA TAY VAO VERSION MOI NHAT HIEN CO
echo ========================================================
echo.

set "LATEST_VER=V11_0_NUT_B1_KICH_AM_A6_SOUND"
for /f "delims=" %%D in ('powershell -ExecutionPolicy Bypass -File "%~dp0get_versions.ps1" -Latest') do set "LATEST_VER=%%D"

echo   >> Phien ban moi nhat tim thay: [!LATEST_VER!]
echo.
echo Dang sao chep toan bo code hien tai vao [JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!]...
if not exist "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_RELAY" mkdir "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_RELAY"
if not exist "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_SOUND" mkdir "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_SOUND"
if not exist "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_JUMPSCARE" mkdir "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_JUMPSCARE"

xcopy "JOB_2_HAUNTED_HOUSE\STM32_RELAY\*" "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_RELAY\" /s /e /y /q >nul
xcopy "JOB_2_HAUNTED_HOUSE\STM32_SOUND\*" "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_SOUND\" /s /e /y /q >nul
if exist "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE" xcopy "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE\*" "JOB_2_HAUNTED_HOUSE\VERSIONS\!LATEST_VER!\STM32_JUMPSCARE\" /s /e /y /q >nul

echo.
echo ========================================================
echo   [THANH CONG] DA LUU XONG TOAN BO CODE VAO: [!LATEST_VER!]!
echo ========================================================
echo.
pause
goto MAIN_MENU

:: ==========================================================
:: TAO VERSION MOI VOI TEN RIENG
:: ==========================================================
:TAO_VER_MOI
cls
echo ========================================================
echo   TAO PHIEN BAN CODE MOI VA LUU VAO THU MUC VERSIONS
echo ========================================================
echo.
echo Vi du ten phien ban: V3_2, V4_KICH_BAN_MOI, V5_TEST...
set /p new_ver_name="Nhap ten Version moi muon tao: "
if "%new_ver_name%"=="" (
    echo Ten khong duoc de trong!
    pause
    goto MAIN_MENU
)

set "TARGET_DIR=JOB_2_HAUNTED_HOUSE\VERSIONS\%new_ver_name%"
if exist "%TARGET_DIR%" (
    echo CANH BAO: Phien ban [%new_ver_name%] da ton tai!
    set /p ghi_de="Ban co muon ghi de khong? (Y/N): "
    if /i not "!ghi_de!"=="Y" goto MAIN_MENU
)

echo.
echo Dang luu code hien tai vao: [%TARGET_DIR%]...
if not exist "%TARGET_DIR%\STM32_RELAY" mkdir "%TARGET_DIR%\STM32_RELAY"
if not exist "%TARGET_DIR%\STM32_SOUND" mkdir "%TARGET_DIR%\STM32_SOUND"
if not exist "%TARGET_DIR%\STM32_JUMPSCARE" mkdir "%TARGET_DIR%\STM32_JUMPSCARE"

xcopy "JOB_2_HAUNTED_HOUSE\STM32_RELAY\*" "%TARGET_DIR%\STM32_RELAY\" /s /e /y /q >nul
xcopy "JOB_2_HAUNTED_HOUSE\STM32_SOUND\*" "%TARGET_DIR%\STM32_SOUND\" /s /e /y /q >nul
if exist "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE" xcopy "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE\*" "%TARGET_DIR%\STM32_JUMPSCARE\" /s /e /y /q >nul

echo.
echo ========================================================
echo   [THANH CONG] DA TAO VA LUU XONG PHIEN BAN: [%new_ver_name%]!
echo ========================================================
echo Tu nay ban co the chon Version nay trong menu nap code!
echo.
pause
goto MAIN_MENU

:: ==========================================================
:: CAC CHUC NANG KIEM TRA ^& NGOAI VI
:: ==========================================================
:TEST_BUILD
cls
echo ========================================================
echo   Dang kiem tra bien dich ca 3 mach (RELAY + SOUND + JUMPSCARE)...
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e NAP_MACH_RELAY -e NAP_MACH_RELAY_GIO_CU -e NAP_MACH_AM_THANH -e NAP_MACH_AM_THANH_GIO_CU -e NAP_MACH_JUMPSCARE
echo.
echo Kiem tra hoan tat!
pause
goto MAIN_MENU

:NAP_TEST_CAM_BIEN
cls
echo ========================================================
echo   Dang nap code TEST CAM BIEN qua ST-Link...
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_CAM_BIEN -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU

:NAP_TEST_NUT_A6_RELAY_A4
cls
echo ========================================================
echo   Dang nap code TEST NUT A6 -^> RELAY A4 (4S) qua ST-Link...
echo   Target: PA6 (Nut nhan) - PA4 (Relay 4s) - PC13 (LED)
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_NUT_A6_RELAY_A4 -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU

:NAP_TEST_UART_SOUND
cls
echo ========================================================
echo   Dang nap code TEST UART BO SOUND qua ST-Link...
echo   Target: PB10 (TX sang B6 Jump) - PB11 (RX tu B5 Jump)
echo   LED PC13 nhay khi truyen / nhan thanh cong!
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_UART_SOUND -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU

:NAP_TEST_UART_JUMPSCARE
cls
echo ========================================================
echo   Dang nap code TEST UART BO JUMPSCARE qua ST-Link...
echo   Target: PB5 (TX sang B11 Sound) - PB6 (RX tu B10 Sound)
echo   Relay PA3 va LED PC13 nhay khi nhan Ping!
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_UART_JUMPSCARE -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU

:NAP_TEST_CHIP_ALIVE
cls
echo ========================================================
echo   Dang nap code TEST STM32 SONG CHET qua ST-Link...
echo ========================================================
echo   QUY TRINH TEST:
echo   1. Khoi dong 3 giay: LED PC13 nhay CHAT (500ms) - Xac nhan khoi dong OK
echo   2. Sau 3 giay:       LED PC13 nhay NHANH (100ms) - STM32 song, san sang test
echo   3. Test GPIO:        Cam day GND cham vao bat ky chan GPIO (PA/PB/PC)
echo                        -^> Neu chan tot: LED PC13 se TAT trong luc cham!
echo                        -^> Nha ra khoi GND: LED PC13 tiep tuc nhay nhanh.
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_STM32_CHIP_ALIVE -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU

:NAP_TEST_ALL_PINS
cls
echo ========================================================
echo   Dang nap code TEST TOAN BO CAC CHAN GPIO qua ST-Link...
echo   Target: PA0..PA8, PA11, PA12, PA15, PB0..PB15, PC13..PC15
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_ALL_PINS -t upload
echo.
echo Da thuc hien xong!
pause
goto MAIN_MENU
