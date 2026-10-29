[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
Set-Location -Path $PSScriptRoot

$pioPath = "C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe"
$versionsRoot = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\VERSIONS"

function Get-STLinkStatus {
    $dev = Get-PnpDevice -PresentOnly -ErrorAction SilentlyContinue | Where-Object { $_.FriendlyName -match "STLink|ST-Link" }
    if ($dev) {
        return @{ Connected = $true; Name = $dev.FriendlyName[0]; Status = $dev.Status[0] }
    }
    return @{ Connected = $false }
}

function Show-MainMenu {
    Clear-Host
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "   HE THONG NAP CODE STM32 (PLATFORMIO MULTI-VERSION)   " -ForegroundColor Yellow
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    
    $st = Get-STLinkStatus
    if ($st.Connected) {
        Write-Host "  >> TRẠNG THÁI ST-LINK: [ ĐÃ KẾT NỐI ] ($($st.Name))" -ForegroundColor Green
    } else {
        Write-Host "  >> TRẠNG THÁI ST-LINK: [ CHƯA CẮM ] (Vui lòng cắm ST-Link vào USB!)" -ForegroundColor Red
    }
    Write-Host ""
    Write-Host "--------------------------------------------------------" -ForegroundColor DarkGray
    Write-Host "  CHỌN LOẠI MẠCH ĐỂ NẠP CODE:" -ForegroundColor DarkCyan
    Write-Host "  [1] NẠP MẠCH RELAY (STM32_RELAY)" -ForegroundColor Green
    Write-Host "  [2] NẠP MẠCH SOUND (STM32_SOUND)" -ForegroundColor Green
    Write-Host "  [3] NẠP MẠCH JUMPSCARE (STM32_JUMPSCARE)" -ForegroundColor Green
    Write-Host ""
    Write-Host "  TIỆN ÍCH PHIÊN BẢN & NGOẠI VI:" -ForegroundColor DarkCyan
    Write-Host "  [4] LƯU CODE VÀO VER MỚI NHẤT HIỆN CÓ (Ghi đè vào bản mới nhất)" -ForegroundColor Yellow
    Write-Host "  [5] TẠO VERSION MỚI (Lưu thành một phiên bản tên mới)" -ForegroundColor Yellow
    Write-Host "  [6] KIỂM TRA BIÊN DỊCH (BUILD TEST CẢ 3 MẠCH)" -ForegroundColor White
    Write-Host "  [7] QUÉT LẠI CỔNG ST-LINK" -ForegroundColor Cyan
    Write-Host "  [8] NẠP TEST CẢM BIẾN (TEST_CAM_BIEN: PB0 + PC13)" -ForegroundColor Magenta
    Write-Host "  [9] NẠP TEST NÚT A6 - RELAY A4 (PA6 -> PA4 4S)" -ForegroundColor Magenta
    Write-Host "  [0] Thoát" -ForegroundColor Red
    Write-Host "========================================================" -ForegroundColor Cyan
}

function Select-And-Flash-Version {
    param (
        [string]$machName,      # "STM32_RELAY" hoặc "STM32_SOUND"
        [string]$envName,       # "NAP_MACH_RELAY" hoặc "NAP_MACH_AM_THANH"
        [string]$machLabel      # Nhãn hiển thị thân thiện
    )

    Clear-Host
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "   CHỌN PHIÊN BẢN CODE CHO: $machLabel" -ForegroundColor Yellow
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  [0] BẢN CODE HIỆN TẠI (Đang làm việc trực tiếp trong thư mục)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  --- DANH SÁCH CÁC PHIÊN BẢN TRONG THƯ MỤC VERSIONS ---" -ForegroundColor DarkGray

    $verDirs = Get-ChildItem -Path $versionsRoot -Directory | Where-Object {
        Test-Path (Join-Path $_.FullName $machName)
    } | Sort-Object {
        if ($_.Name -match '^V(\d+)(?:_(\d+))?') {
            [int]$matches[1] * 1000 + $(if ($matches[2]) { [int]$matches[2] } else { 0 })
        } else { 0 }
    }

    for ($i = 0; $i -lt $verDirs.Count; $i++) {
        Write-Host "  [$($i+1)] $($verDirs[$i].Name)" -ForegroundColor White
    }

    Write-Host ""
    Write-Host "  [B] Quay lại Menu chính" -ForegroundColor DarkGray
    Write-Host "========================================================" -ForegroundColor Cyan

    $choice = Read-Host "Nhập số Version muốn nạp (0-$($verDirs.Count) hoặc B)"
    if ($choice -eq "B" -or $choice -eq "b") { return }

    if ($choice -eq "0") {
        Clear-Host
        Write-Host "========================================================" -ForegroundColor Cyan
        Write-Host "   ĐANG NẠP CODE HIỆN TẠI QUA ST-LINK (ENV: $envName)..." -ForegroundColor Green
        Write-Host "========================================================" -ForegroundColor Cyan
        Write-Host ""
        & $pioPath run -e $envName -t upload
        Write-Host ""
        Write-Host "Nạp hoàn tất cho $machLabel!" -ForegroundColor Green
        Read-Host "Nhấn Enter để quay lại menu..."
        return
    }

    $num = 0
    if ([int]::TryParse($choice, [ref]$num) -and $num -ge 1 -and $num -le $verDirs.Count) {
        $chosenDir = $verDirs[$num - 1]
        $sourcePath = Join-Path $chosenDir.FullName $machName
        $targetPath = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\$machName"

        Clear-Host
        Write-Host "========================================================" -ForegroundColor Cyan
        Write-Host "   ĐANG LẤY CODE VERSION: [$($chosenDir.Name)]" -ForegroundColor Yellow
        Write-Host "   ĐÍCH ĐẾN: [JOB_2_HAUNTED_HOUSE\$machName]" -ForegroundColor DarkGray
        Write-Host "========================================================" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Đang sao chép mã nguồn từ thư mục VERSIONS ra thư mục làm việc..." -ForegroundColor Cyan
        Copy-Item -Path "$sourcePath\*" -Destination $targetPath -Recurse -Force

        Write-Host "Bắt đầu nạp code qua ST-Link (ENV: $envName)..." -ForegroundColor Green
        & $pioPath run -e $envName -t upload
        Write-Host ""
        Write-Host "Đã nạp xong cho $machLabel - Version [$($chosenDir.Name)]!" -ForegroundColor Green
        Read-Host "Nhấn Enter để quay lại..."
    }
}

function Save-To-Latest-Version {
    Clear-Host
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "   LƯU CODE ĐANG SỬA TAY VÀO VERSION MỚI NHẤT HIỆN CÓ   " -ForegroundColor Yellow
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""

    $latestDir = Get-ChildItem -Path $versionsRoot -Directory | Where-Object { $_.Name -like "V*" } | Sort-Object Name | Select-Object -Last 1
    if (-not $latestDir) {
        Write-Host "Không tìm thấy thư mục Version nào!" -ForegroundColor Red
        Read-Host "Nhấn Enter để quay lại..."
        return
    }

    Write-Host "  >> Phiên bản mới nhất tìm thấy: [$($latestDir.Name)]" -ForegroundColor Green
    Write-Host ""
    Write-Host "Đang sao chép toàn bộ code hiện tại vào [$($latestDir.FullName)]..." -ForegroundColor Cyan

    $relaySrc = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_RELAY"
    $soundSrc = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_SOUND"
    $jumpSrc  = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE"
    $relayDest = Join-Path $latestDir.FullName "STM32_RELAY"
    $soundDest = Join-Path $latestDir.FullName "STM32_SOUND"
    $jumpDest  = Join-Path $latestDir.FullName "STM32_JUMPSCARE"

    New-Item -ItemType Directory -Path $relayDest -Force | Out-Null
    New-Item -ItemType Directory -Path $soundDest -Force | Out-Null
    New-Item -ItemType Directory -Path $jumpDest -Force | Out-Null

    Copy-Item -Path "$relaySrc\*" -Destination $relayDest -Recurse -Force
    Copy-Item -Path "$soundSrc\*" -Destination $soundDest -Recurse -Force
    if (Test-Path $jumpSrc) {
        Copy-Item -Path "$jumpSrc\*" -Destination $jumpDest -Recurse -Force
    }

    Write-Host ""
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "=> ĐÃ LƯU THÀNH CÔNG CODE VÀO BẢN MỚI NHẤT: [$($latestDir.Name)]!" -ForegroundColor Green
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    Read-Host "Nhấn Enter để quay lại..."
}

function Create-New-Version {
    Clear-Host
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "   TẠO PHIÊN BẢN MỚI VÀ LƯU VÀO THƯ MỤC VERSIONS" -ForegroundColor Yellow
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Ví dụ tên phiên bản: V3_2, V4_KICH_BAN_MOI, V5_TEST..." -ForegroundColor DarkGray
    $verName = Read-Host "Nhập tên Version mới muốn tạo"
    if ([string]::IsNullOrWhiteSpace($verName)) {
        Write-Host "Tên không được để trống!" -ForegroundColor Red
        Read-Host "Nhấn Enter để tiếp tục..."
        return
    }

    $destVer = Join-Path $versionsRoot $verName
    if (Test-Path $destVer) {
        $ov = Read-Host "Phiên bản [$verName] đã tồn tại! Bạn có muốn ghi đè? (Y/N)"
        if ($ov -ne "Y" -and $ov -ne "y") { return }
    } else {
        New-Item -ItemType Directory -Path $destVer -Force | Out-Null
    }

    $relaySrc = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_RELAY"
    $soundSrc = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_SOUND"
    $jumpSrc  = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\STM32_JUMPSCARE"
    $relayDest = Join-Path $destVer "STM32_RELAY"
    $soundDest = Join-Path $destVer "STM32_SOUND"
    $jumpDest  = Join-Path $destVer "STM32_JUMPSCARE"

    New-Item -ItemType Directory -Path $relayDest -Force | Out-Null
    New-Item -ItemType Directory -Path $soundDest -Force | Out-Null
    New-Item -ItemType Directory -Path $jumpDest -Force | Out-Null

    Copy-Item -Path "$relaySrc\*" -Destination $relayDest -Recurse -Force
    Copy-Item -Path "$soundSrc\*" -Destination $soundDest -Recurse -Force
    if (Test-Path $jumpSrc) {
        Copy-Item -Path "$jumpSrc\*" -Destination $jumpDest -Recurse -Force
    }

    Write-Host ""
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "=> ĐÃ LƯU THÀNH CÔNG PHIÊN BẢN MỚI: [$verName]!" -ForegroundColor Green
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    Read-Host "Nhấn Enter để quay lại..."
}

# Vòng lặp Menu chính
while ($true) {
    Show-MainMenu
    $choice = Read-Host "Nhập lựa chọn (1-9 hoặc 0)"
    switch ($choice) {
        "1" { Select-And-Flash-Version "STM32_RELAY" "NAP_MACH_RELAY" "MẠCH RELAY" }
        "2" { Select-And-Flash-Version "STM32_SOUND" "NAP_MACH_AM_THANH" "MẠCH SOUND" }
        "3" { Select-And-Flash-Version "STM32_JUMPSCARE" "NAP_MACH_JUMPSCARE" "MẠCH JUMPSCARE" }
        "4" { Save-To-Latest-Version }
        "5" { Create-New-Version }
        "6" {
            Clear-Host
            Write-Host "Đang kiểm tra biên dịch cả 3 mạch (RELAY + SOUND + JUMPSCARE)..." -ForegroundColor Yellow
            & $pioPath run -e NAP_MACH_RELAY -e NAP_MACH_AM_THANH -e NAP_MACH_JUMPSCARE
            Read-Host "Nhấn Enter để tiếp tục..."
        }
        "7" { continue }
        "8" {
            Clear-Host
            Write-Host "Đang nạp code TEST CẢM BIẾN..." -ForegroundColor Magenta
            & $pioPath run -e TEST_CAM_BIEN -t upload
            Read-Host "Nhấn Enter để tiếp tục..."
        }
        "9" {
            Clear-Host
            Write-Host "Đang nạp code TEST NÚT A6 -> RELAY A4..." -ForegroundColor Magenta
            & $pioPath run -e TEST_NUT_A6_RELAY_A4 -t upload
            Read-Host "Nhấn Enter để tiếp tục..."
        }
        "0" { exit }
        default { }
    }
}
