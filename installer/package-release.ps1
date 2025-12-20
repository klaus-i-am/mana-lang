#Requires -Version 5.1
<#
.SYNOPSIS
    Package Mana releases for distribution
.DESCRIPTION
    Creates distributable archives for Windows releases.
    Run this after building the compiler in Release mode.
.PARAMETER Version
    Version string (e.g., "1.0.0")
.PARAMETER BuildDir
    Path to the build directory containing Release binaries
.PARAMETER OutputDir
    Where to place the output archives
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$Version,

    [string]$BuildDir = "..\build\Release",
    [string]$OutputDir = ".\dist"
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

Write-Host ""
Write-Host "  Mana Release Packager" -ForegroundColor Cyan
Write-Host "  Version: $Version" -ForegroundColor Gray
Write-Host ""

# Resolve paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Resolve-Path (Join-Path $ScriptDir "..")
$BuildDir = Resolve-Path (Join-Path $ScriptDir $BuildDir)
$OutputDir = Join-Path $ScriptDir $OutputDir

# Check build exists
if (-not (Test-Path (Join-Path $BuildDir "mana.exe"))) {
    Write-Host "Error: mana.exe not found in $BuildDir" -ForegroundColor Red
    Write-Host "Run 'scripts\build.bat release' first." -ForegroundColor Yellow
    exit 1
}

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# Determine architecture
$arch = if ($env:PROCESSOR_ARCHITECTURE -eq "ARM64") { "arm64" } else { "x64" }

Write-Step "Creating Windows $arch package"

$packageName = "mana-$Version-windows-$arch"
$tempDir = Join-Path $env:TEMP $packageName
$zipPath = Join-Path $OutputDir "$packageName.zip"

# Clean temp directory
if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
}

# Create package structure
$binDir = New-Item -ItemType Directory -Path (Join-Path $tempDir "bin") -Force
$includeDir = New-Item -ItemType Directory -Path (Join-Path $tempDir "include") -Force
$examplesDir = New-Item -ItemType Directory -Path (Join-Path $tempDir "examples") -Force

# Copy binaries
Write-Host "  Copying binaries..."
Copy-Item (Join-Path $BuildDir "mana.exe") -Destination $binDir
if (Test-Path (Join-Path $BuildDir "mana-lsp.exe")) {
    Copy-Item (Join-Path $BuildDir "mana-lsp.exe") -Destination $binDir
}
if (Test-Path (Join-Path $BuildDir "mana-debug.exe")) {
    Copy-Item (Join-Path $BuildDir "mana-debug.exe") -Destination $binDir
}

# Copy runtime header
Write-Host "  Copying runtime header..."
Copy-Item (Join-Path $RepoRoot "backend-cpp\mana_runtime.h") -Destination $includeDir

# Copy examples
Write-Host "  Copying examples..."
Get-ChildItem (Join-Path $RepoRoot "examples\*.mana") | ForEach-Object {
    Copy-Item $_.FullName -Destination $examplesDir
}

# Copy README
Write-Host "  Copying documentation..."
Copy-Item (Join-Path $ScriptDir "README.md") -Destination $tempDir

# Create zip
Write-Host "  Creating archive..."
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}
Compress-Archive -Path "$tempDir\*" -DestinationPath $zipPath -CompressionLevel Optimal

# Cleanup
Remove-Item $tempDir -Recurse -Force

# Calculate checksum
$hash = (Get-FileHash $zipPath -Algorithm SHA256).Hash.ToLower()
$checksumFile = Join-Path $OutputDir "$packageName.sha256"
"$hash  $packageName.zip" | Set-Content $checksumFile -NoNewline

Write-Host ""
Write-Host "  Package created:" -ForegroundColor Green
Write-Host "    $zipPath"
Write-Host "    SHA256: $hash"

Write-Step "Release packaging complete!"

Write-Host ""
Write-Host "Files created in $OutputDir:"
Get-ChildItem $OutputDir | ForEach-Object { Write-Host "  - $($_.Name)" }
Write-Host ""
Write-Host "Upload these files to GitHub Releases." -ForegroundColor Yellow
