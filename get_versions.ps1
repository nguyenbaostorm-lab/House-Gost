param(
    [string]$machName,
    [switch]$Latest
)
$versionsDir = Join-Path $PSScriptRoot "JOB_2_HAUNTED_HOUSE\VERSIONS"
$sorted = Get-ChildItem -Directory $versionsDir | Where-Object {
    if ([string]::IsNullOrWhiteSpace($machName)) { $true }
    else { Test-Path (Join-Path $_.FullName $machName) }
} | Sort-Object {
    if ($_.Name -match '^V(\d+)(?:_(\d+))?') {
        [int]$matches[1] * 1000 + $(if ($matches[2]) { [int]$matches[2] } else { 0 })
    } else { 0 }
}

if ($Latest) {
    if ($sorted.Count -gt 0) { $sorted[-1].Name }
} else {
    $sorted | ForEach-Object { $_.Name }
}
