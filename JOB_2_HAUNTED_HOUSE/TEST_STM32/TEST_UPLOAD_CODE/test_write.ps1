$portName = "COM21"
$baudRate = 115200

try {
    $p = New-Object System.IO.Ports.SerialPort($portName, $baudRate, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
    $p.DtrEnable = $true
    $p.RtsEnable = $true
    $p.Handshake = [System.IO.Ports.Handshake]::None
    $p.WriteTimeout = 2000
    $p.ReadTimeout = 2000
    $p.Open()
    Write-Host ">>> OPEN PORT SUCCESS! <<<"
    
    # Gửi ký tự '1'
    [byte[]]$bytes = @(0x31) # '1'
    $p.Write($bytes, 0, 1)
    Write-Host ">>> WRITE BYTE '1' SUCCESS! <<<"
    
    Start-Sleep -Milliseconds 500
    if ($p.BytesToRead -gt 0) {
        $res = $p.ReadExisting()
        Write-Host ">>> RESPONSE FROM STM32:" $res
    }
    $p.Close()
} catch {
    Write-Host ">>> ERROR: " $_.Exception.ToString()
    if ($p -and $p.IsOpen) { $p.Close() }
}

