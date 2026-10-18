@echo off
title NAP CODE TEST CAM BIEN E3F-DS30C4 (PB0 & PC13)
cls
echo ========================================================
echo   DANG NAP CODE TEST CAM BIEN QUA ST-LINK...
echo   Target: PB0 (Tin hieu cam bien) - PC13 (Den tin hieu)
echo ========================================================
echo.
"C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe" run -e TEST_CAM_BIEN -t upload
echo.
echo ========================================================
if errorlevel 1 (
    echo   NAP THAT BAI! Vui long kiem tra lai ST-Link va day cam.
) else (
    echo   NAP THANH CONG!
)
echo ========================================================
pause
