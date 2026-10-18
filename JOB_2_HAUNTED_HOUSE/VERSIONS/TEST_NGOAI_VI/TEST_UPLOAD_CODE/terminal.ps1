Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

$portName = "COM21"
$baudRate = 115200

# Tao Form giao dien
$form = New-Object System.Windows.Forms.Form
$form.Text = "STM32 SERIAL MONITOR - $portName ($baudRate)"
$form.Size = New-Object System.Drawing.Size(660, 530)
$form.StartPosition = "CenterScreen"
$form.BackColor = [System.Drawing.Color]::FromArgb(30, 30, 30)
$form.ForeColor = [System.Drawing.Color]::White
$form.Font = New-Object System.Drawing.Font("Segoe UI", 10)

# Khung hien thi Log
$txtLog = New-Object System.Windows.Forms.TextBox
$txtLog.Multiline = $true
$txtLog.ScrollBars = "Vertical"
$txtLog.ReadOnly = $true
$txtLog.BackColor = [System.Drawing.Color]::FromArgb(20, 20, 20)
$txtLog.ForeColor = [System.Drawing.Color]::LimeGreen
$txtLog.Font = New-Object System.Drawing.Font("Consolas", 10)
$txtLog.Location = New-Object System.Drawing.Point(15, 15)
$txtLog.Size = New-Object System.Drawing.Size(615, 330)
$form.Controls.Add($txtLog)

# Label huong dan
$lblInput = New-Object System.Windows.Forms.Label
$lblInput.Text = "O nhap ky tu can gui:"
$lblInput.Location = New-Object System.Drawing.Point(15, 362)
$lblInput.Size = New-Object System.Drawing.Size(160, 25)
$form.Controls.Add($lblInput)

# O nhap ky tu (Input Box)
$txtInput = New-Object System.Windows.Forms.TextBox
$txtInput.Location = New-Object System.Drawing.Point(175, 358)
$txtInput.Size = New-Object System.Drawing.Size(325, 28)
$txtInput.Font = New-Object System.Drawing.Font("Segoe UI", 11)
$form.Controls.Add($txtInput)

# Nut Gui (Send)
$btnSend = New-Object System.Windows.Forms.Button
$btnSend.Text = "GUI (Send)"
$btnSend.Location = New-Object System.Drawing.Point(510, 356)
$btnSend.Size = New-Object System.Drawing.Size(120, 32)
$btnSend.BackColor = [System.Drawing.Color]::FromArgb(0, 122, 204)
$btnSend.ForeColor = [System.Drawing.Color]::White
$btnSend.FlatStyle = "Flat"
$form.Controls.Add($btnSend)

# Nut Bat den nhanh ('1')
$btnOn = New-Object System.Windows.Forms.Button
$btnOn.Text = "BAT DEN PC13 (Gui '1')"
$btnOn.Location = New-Object System.Drawing.Point(15, 410)
$btnOn.Size = New-Object System.Drawing.Size(300, 48)
$btnOn.BackColor = [System.Drawing.Color]::FromArgb(40, 167, 69)
$btnOn.ForeColor = [System.Drawing.Color]::White
$btnOn.Font = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$btnOn.FlatStyle = "Flat"
$form.Controls.Add($btnOn)

# Nut Tat den nhanh ('0')
$btnOff = New-Object System.Windows.Forms.Button
$btnOff.Text = "TAT DEN PC13 (Gui '0')"
$btnOff.Location = New-Object System.Drawing.Point(330, 410)
$btnOff.Size = New-Object System.Drawing.Size(300, 48)
$btnOff.BackColor = [System.Drawing.Color]::FromArgb(220, 53, 69)
$btnOff.ForeColor = [System.Drawing.Color]::White
$btnOff.Font = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$btnOff.FlatStyle = "Flat"
$form.Controls.Add($btnOff)

# Khoi tao Cong COM
$port = $null
try {
    $port = New-Object System.IO.Ports.SerialPort($portName, $baudRate, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
    $port.DtrEnable = $true
    $port.RtsEnable = $true
    $port.Handshake = [System.IO.Ports.Handshake]::None
    $port.ReadTimeout = 1000
    $port.WriteTimeout = 2000
    $port.Open()
    $txtLog.AppendText("[HE THONG] Ket noi thanh cong $portName ($baudRate)`r`n`r`n")
} catch {
    $txtLog.AppendText("[LOI KET NOI]: " + $_.Exception.Message + "`r`n")
}

# Timer doc du lieu tu cong Serial
$timer = New-Object System.Windows.Forms.Timer
$timer.Interval = 100
$timer.Add_Tick({
    if ($port -ne $null -and $port.IsOpen) {
        try {
            if ($port.BytesToRead -gt 0) {
                $data = $port.ReadExisting()
                $txtLog.AppendText($data)
                $txtLog.SelectionStart = $txtLog.TextLength
                $txtLog.ScrollToCaret()
            }
        } catch {}
    }
})
$timer.Start()

# Ham gui du lieu
function SendData($str) {
    if ($port -ne $null -and $port.IsOpen) {
        try {
            [byte[]]$bytes = [System.Text.Encoding]::ASCII.GetBytes($str)
            $port.Write($bytes, 0, $bytes.Length)
            $txtLog.AppendText("`r`n[BAN DA GUI]: " + $str + "`r`n")
            $txtLog.SelectionStart = $txtLog.TextLength
            $txtLog.ScrollToCaret()
        } catch {
            $txtLog.AppendText("`r`n[LOI GUI CHI TIET]: " + $_.Exception.Message + "`r`n")
            $txtLog.SelectionStart = $txtLog.TextLength
            $txtLog.ScrollToCaret()
        }
    } else {
        [System.Windows.Forms.MessageBox]::Show("Cong $portName chua duoc mo! Hay kiem tra ket noi.", "Thong bao")
    }
}

# Su kien bam nut Send
$btnSend.Add_Click({
    if ($txtInput.Text.Length -gt 0) {
        SendData($txtInput.Text)
        $txtInput.Clear()
        $txtInput.Focus()
    }
})

# Nhan phim Enter trong o nhap cung gui
$txtInput.Add_KeyDown({
    param($s, $e)
    if ($e.KeyCode -eq [System.Windows.Forms.Keys]::Enter) {
        $e.SuppressKeyPress = $true
        $btnSend.PerformClick()
    }
})

# Su kien 2 nut Bat / Tat
$btnOn.Add_Click({ SendData("1") })
$btnOff.Add_Click({ SendData("0") })

# Dong cong khi thoat form
$form.Add_FormClosing({
    $timer.Stop()
    if ($port -ne $null -and $port.IsOpen) {
        $port.Close()
    }
})

[void]$form.ShowDialog()
