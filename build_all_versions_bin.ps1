$ErrorActionPreference = "Stop"

$workspaceRoot = "D:\dowloands\House-Gost-main\House-Gost-main"
$jobRoot       = "$workspaceRoot\JOB_2_HAUNTED_HOUSE"
$versionsRoot  = "$jobRoot\VERSIONS"
$binCodeRoot   = "$workspaceRoot\BIN_CODE"
$buildDir      = "C:\Users\MR_STORM\.pio_build\job_team"
$pioExe        = "C:\Users\MR_STORM\.platformio\penv\Scripts\pio.exe"

$versionMap = [ordered]@{
    "V1_CHUAN"                           = "V1_CHUAN"
    "V2_RESET_PB0"                       = "V2_RESET"
    "V3_1_FIX_VIN_SCANIGHT"              = "V3_1"
    "V3_2"                               = "V3_2"
    "V4_1"                               = "V4_1"
    "V5_1"                               = "V5_1"
    "V5_2"                               = "V5_2"
    "V5_3"                               = "V5_3"
    "V6_1"                               = "V6_1"
    "V6_2"                               = "V6_2"
    "V6_4"                               = "V6_4"
    "V6_5_FIX_THOI_GIAN_BO_GO_P14"       = "V6_5"
    "V6_6_TICH_HOP_UART_SOUND_JUMPSCARE" = "V6_6"
    "V7_0_FIX_TREO_LENH_NHAY_RELAY"      = "V7_0"
    "V8_0_KICH_ON_BO_GO_P14_45S"         = "V8_0"
    "V9_0_NUT_NHAN_A5_KICH_AM_P14_45S"   = "V9_0"
    "V10_0_DU_PHONG_PA4_PA5_BO_GO_45S"   = "V10_0"
    "V11_0_NUT_B1_KICH_AM_A6_SOUND"      = "V11_0"
}

# Backup working directories
$backupDir = "$jobRoot\_TEMP_BACKUP_WORK"
if (Test-Path $backupDir) { Remove-Item $backupDir -Recurse -Force }
New-Item -ItemType Directory -Path "$backupDir\STM32_RELAY" -Force | Out-Null
New-Item -ItemType Directory -Path "$backupDir\STM32_SOUND" -Force | Out-Null
New-Item -ItemType Directory -Path "$backupDir\STM32_JUMPSCARE" -Force | Out-Null

Copy-Item "$jobRoot\STM32_RELAY\*" "$backupDir\STM32_RELAY\" -Recurse -Force
Copy-Item "$jobRoot\STM32_SOUND\*" "$backupDir\STM32_SOUND\" -Recurse -Force
Copy-Item "$jobRoot\STM32_JUMPSCARE\*" "$backupDir\STM32_JUMPSCARE\" -Recurse -Force

Write-Host ">>> Da tao backup working directory thanh cong!" -ForegroundColor Green

Set-Location $workspaceRoot

$totalVersions = $versionMap.Keys.Count
$idx = 0

foreach ($verFolder in $versionMap.Keys) {
    $idx++
    $tag = $versionMap[$verFolder]
    $verPath = "$versionsRoot\$verFolder"
    $binDir  = "$verPath\BIN"
    $binCodeDir = "$binCodeRoot\$verFolder"

    New-Item -ItemType Directory -Path $binDir -Force | Out-Null
    New-Item -ItemType Directory -Path $binCodeDir -Force | Out-Null

    Write-Host "`n========================================================" -ForegroundColor Cyan
    Write-Host "[$idx/$totalVersions] DANG BIEN DICH VERSION: $verFolder (TAG: $tag)" -ForegroundColor Yellow
    Write-Host "========================================================" -ForegroundColor Cyan

    $hasRelay = Test-Path "$verPath\STM32_RELAY"
    $hasSound = Test-Path "$verPath\STM32_SOUND"
    $hasJumpscare = Test-Path "$verPath\STM32_JUMPSCARE"

    # 1. BIEN DICH RELAY (2 LOAI THOI GIAN)
    if ($hasRelay) {
        Write-Host ">>> Copy STM32_RELAY cho $verFolder..." -ForegroundColor DarkGray
        Copy-Item "$verPath\STM32_RELAY\*" "$jobRoot\STM32_RELAY\" -Recurse -Force

        Write-Host ">>> Compiling RELAY (Gio Moi & Gio Cu)..." -ForegroundColor White
        & $pioExe run -e NAP_MACH_RELAY -e NAP_MACH_RELAY_GIO_CU
        if ($LASTEXITCODE -ne 0) { throw "Build loi RELAY tai $verFolder" }

        Copy-Item "$buildDir\NAP_MACH_RELAY\firmware.bin"        "$binDir\STM32_RELAY_${tag}_GIO_MOI_3P.bin" -Force
        Copy-Item "$buildDir\NAP_MACH_RELAY\firmware.bin"        "$binDir\STM32_RELAY_${tag}.bin" -Force
        Copy-Item "$buildDir\NAP_MACH_RELAY_GIO_CU\firmware.bin" "$binDir\STM32_RELAY_${tag}_GIO_CU_10S.bin" -Force
        Write-Host ">>> Da tao file bin RELAY ($tag) thanh cong!" -ForegroundColor Green
    }

    # 2. BIEN DICH SOUND (2 LOAI THOI GIAN)
    if ($hasSound) {
        Write-Host ">>> Copy STM32_SOUND cho $verFolder..." -ForegroundColor DarkGray
        Copy-Item "$verPath\STM32_SOUND\*" "$jobRoot\STM32_SOUND\" -Recurse -Force

        Write-Host ">>> Compiling SOUND (Gio Moi & Gio Cu)..." -ForegroundColor White
        & $pioExe run -e NAP_MACH_AM_THANH -e NAP_MACH_AM_THANH_GIO_CU
        if ($LASTEXITCODE -ne 0) { throw "Build loi SOUND tai $verFolder" }

        Copy-Item "$buildDir\NAP_MACH_AM_THANH\firmware.bin"        "$binDir\STM32_SOUND_${tag}_GIO_MOI_3P43S.bin" -Force
        Copy-Item "$buildDir\NAP_MACH_AM_THANH\firmware.bin"        "$binDir\STM32_SOUND_${tag}.bin" -Force
        Copy-Item "$buildDir\NAP_MACH_AM_THANH_GIO_CU\firmware.bin" "$binDir\STM32_SOUND_${tag}_GIO_CU_15S.bin" -Force
        Write-Host ">>> Da tao file bin SOUND ($tag) thanh cong!" -ForegroundColor Green
    }

    # 3. BIEN DICH JUMPSCARE (NEU CO)
    if ($hasJumpscare) {
        Write-Host ">>> Copy STM32_JUMPSCARE cho $verFolder..." -ForegroundColor DarkGray
        Copy-Item "$verPath\STM32_JUMPSCARE\*" "$jobRoot\STM32_JUMPSCARE\" -Recurse -Force

        Write-Host ">>> Compiling JUMPSCARE..." -ForegroundColor White
        & $pioExe run -e NAP_MACH_JUMPSCARE
        if ($LASTEXITCODE -ne 0) { throw "Build loi JUMPSCARE tai $verFolder" }

        Copy-Item "$buildDir\NAP_MACH_JUMPSCARE\firmware.bin" "$binDir\STM32_JUMPSCARE_${tag}.bin" -Force
        Write-Host ">>> Da tao file bin JUMPSCARE ($tag) thanh cong!" -ForegroundColor Green
    }

    # Dong bo sang BIN_CODE
    Copy-Item "$binDir\*.bin" "$binCodeDir\" -Force
    Write-Host ">>> Da dong bo file bin sang $binCodeDir" -ForegroundColor Cyan
}

# Restore working directories to backup
Write-Host "`n>>> Dang phuc hoi lai working directories tu backup..." -ForegroundColor Yellow
Copy-Item "$backupDir\STM32_RELAY\*" "$jobRoot\STM32_RELAY\" -Recurse -Force
Copy-Item "$backupDir\STM32_SOUND\*" "$jobRoot\STM32_SOUND\" -Recurse -Force
Copy-Item "$backupDir\STM32_JUMPSCARE\*" "$jobRoot\STM32_JUMPSCARE\" -Recurse -Force
Remove-Item $backupDir -Recurse -Force

Write-Host "`n========================================================" -ForegroundColor Green
Write-Host "TAT CA 18 PHIEN BAN DA DUOC BIEN DICH VA TAO BIN HOAN TAT!" -ForegroundColor Green
Write-Host "========================================================" -ForegroundColor Green

