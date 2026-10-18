@echo off
title NAP CODE TEST NUT A6 -> RELAY A4 (PA6 - PA4 4S)
cls
echo ========================================================
echo   DANG NAP CODE TEST NUT A6 - RELAY A4 QUA ST-LINK...
echo   Target: PA6 (Nut nhan GND) - PA4 (Relay 4s) - PC13 (LED)
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_NUT_A6_RELAY_A4 -t upload
echo.
echo ========================================================
if errorlevel 1 (
    echo   NAP THAT BAI! Vui long kiem tra lai ST-Link va day cam.
) else (
    echo   NAP THANH CONG!
)
echo ========================================================
pause
