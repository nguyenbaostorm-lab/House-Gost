@echo off
chcp 65001 >nul
echo Đang gửi lệnh tắt đèn PC13 tới STM32 (COM21)...
powershell -NoProfile -Command "$p=New-Object System.IO.Ports.SerialPort 'COM21',9600;$p.DtrEnable=$true;$p.RtsEnable=$true;try{$p.Open();$p.Write('0');Start-Sleep -m 200;$p.Close();Write-Host '>>> ĐÈN LED PC13 ĐÃ TẮT! <<<' -ForegroundColor Yellow}catch{Write-Host 'Lỗi: ' $_.Exception.Message -ForegroundColor Red}"
pause
