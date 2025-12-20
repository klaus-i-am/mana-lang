#Requires -Version 5.1
<#
.SYNOPSIS
    Mana Programming Language Uninstaller for Windows
.DESCRIPTION
    Removes the Mana compiler toolchain from your system.
.PARAMETER InstallDir
    Installation directory to uninstall from (default: $env:USERPROFILE\.mana)
.PARAMETER Yes
    Skip confirmation prompt
#>

param(
    [string]$InstallDir = "$env:USERPROFILE\.mana",
    [switch]$Yes
)

$ErrorActionPreference = "Stop"

function Write-Color {
    param([string]$Text, [ConsoleColor]$Color = "White")
    $prevColor = $Host.UI.RawUI.ForegroundColor
    $Host.UI.RawUI.ForegroundColor = $Color
    Write-Host $Text
    $Host.UI.RawUI.ForegroundColor = $prevColor
}

Write-Host ""
Write-Color "  Mana Uninstaller" Cyan
Write-Host ""

$binDir = Join-Path $InstallDir "bin"

if (-not (Test-Path $InstallDir)) {
    Write-Color "Mana is not installed at $InstallDir" Yellow
    Write-Host "Nothing to uninstall."
    exit 0
}

Write-Host "This will remove Mana from: $InstallDir"
Write-Host ""

if (-not $Yes) {
    $response = Read-Host "Are you sure you want to uninstall? [y/N]"
    if ($response -notmatch "^[Yy]") {
        Write-Color "Uninstall cancelled." Yellow
        exit 0
    }
}

Write-Host ""

# Remove from PATH
$currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")
$paths = $currentPath -split ";" | Where-Object { $_ -ne $binDir -and $_ -ne "" }
[Environment]::SetEnvironmentVariable("PATH", ($paths -join ";"), "User")
Write-Color "[OK] Removed from PATH" Green

# Remove MANA_HOME
[Environment]::SetEnvironmentVariable("MANA_HOME", $null, "User")
Write-Color "[OK] Removed MANA_HOME environment variable" Green

# Remove installation directory
Remove-Item $InstallDir -Recurse -Force
Write-Color "[OK] Removed $InstallDir" Green

Write-Host ""
Write-Color "Mana has been successfully uninstalled." Green
Write-Host ""
Write-Host "Thank you for using Mana!"
Write-Host "If you have feedback, please visit: https://github.com/mana-lang/mana/issues"
Write-Host ""
