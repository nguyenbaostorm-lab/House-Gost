@echo off
chcp 65001 >nul
echo Đang gửi lệnh bật đèn PC13 tới STM32 (COM21)...
powershell -NoProfile -Command "$p=New-Object System.IO.Ports.SerialPort 'COM21',9600;$p.DtrEnable=$true;$p.RtsEnable=$true;try{$p.Open();$p.Write('1');Start-Sleep -m 200;$p.Close();Write-Host '>>> ĐÈN LED PC13 ĐÃ SÁNG! <<<' -ForegroundColor Green}catch{Write-Host 'Lỗi: ' $_.Exception.Message -ForegroundColor Red}"
pause
