#Requires -Version 5.1
<#
.SYNOPSIS
    Mana Programming Language Installer for Windows
.DESCRIPTION
    Installs the Mana compiler toolchain including:
    - mana.exe (compiler)
    - mana-lsp.exe (language server)
    - mana-debug.exe (debug adapter)
    - mana_runtime.h (C++ runtime header)
.PARAMETER InstallDir
    Custom installation directory (default: $env:USERPROFILE\.mana)
.PARAMETER NoModifyPath
    Skip modifying the PATH environment variable
.PARAMETER Uninstall
    Uninstall Mana instead of installing
.EXAMPLE
    irm https://mana-lang.dev/install.ps1 | iex
.EXAMPLE
    .\mana-init.ps1 -InstallDir "C:\tools\mana"
#>

param(
    [string]$InstallDir = "$env:USERPROFILE\.mana",
    [switch]$NoModifyPath,
    [switch]$Uninstall,
    [switch]$Yes
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Configuration
# ============================================================================

$MANA_VERSION = "1.0.0"
$MANA_REPO = "https://github.com/mana-lang/mana"
$MANA_RELEASE_URL = "https://github.com/mana-lang/mana/releases/download/v$MANA_VERSION"

# Colors for output
function Write-Color {
    param([string]$Text, [ConsoleColor]$Color = "White")
    $prevColor = $Host.UI.RawUI.ForegroundColor
    $Host.UI.RawUI.ForegroundColor = $Color
    Write-Host $Text
    $Host.UI.RawUI.ForegroundColor = $prevColor
}

function Write-Banner {
    Write-Host ""
    Write-Color "  __  __                   " Cyan
    Write-Color " |  \/  | __ _ _ __   __ _ " Cyan
    Write-Color " | |\/| |/ _`` | '_ \ / _`` |" Cyan
    Write-Color " | |  | | (_| | | | | (_| |" Cyan
    Write-Color " |_|  |_|\__,_|_| |_|\__,_|" Cyan
    Write-Host ""
    Write-Color " The Mana Programming Language Installer" White
    Write-Color " Version $MANA_VERSION" DarkGray
    Write-Host ""
}

function Write-Section {
    param([string]$Title)
    Write-Host ""
    Write-Color "==> $Title" Yellow
}

function Write-Info {
    param([string]$Text)
    Write-Host "    $Text"
}

function Write-Success {
    param([string]$Text)
    Write-Color "    [OK] $Text" Green
}

function Write-Warning {
    param([string]$Text)
    Write-Color "    [!] $Text" Yellow
}

function Write-Error {
    param([string]$Text)
    Write-Color "    [X] $Text" Red
}

# ============================================================================
# Utility Functions
# ============================================================================

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Get-Architecture {
    $arch = $env:PROCESSOR_ARCHITECTURE
    switch ($arch) {
        "AMD64" { return "x64" }
        "ARM64" { return "arm64" }
        "x86"   { return "x86" }
        default { return "x64" }
    }
}

function Test-Command {
    param([string]$Command)
    return $null -ne (Get-Command $Command -ErrorAction SilentlyContinue)
}

function Add-ToUserPath {
    param([string]$PathToAdd)

    $currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    if ($currentPath -notlike "*$PathToAdd*") {
        $newPath = "$currentPath;$PathToAdd"
        [Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
        $env:PATH = "$env:PATH;$PathToAdd"
        return $true
    }
    return $false
}

function Remove-FromUserPath {
    param([string]$PathToRemove)

    $currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    $paths = $currentPath -split ";" | Where-Object { $_ -ne $PathToRemove -and $_ -ne "" }
    $newPath = $paths -join ";"
    [Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
}

# ============================================================================
# Installation Functions
# ============================================================================

function Install-Mana {
    $binDir = Join-Path $InstallDir "bin"
    $includeDir = Join-Path $InstallDir "include"
    $arch = Get-Architecture

    Write-Section "Installation Options"
    Write-Info "Installation directory: $InstallDir"
    Write-Info "Binary directory: $binDir"
    Write-Info "Include directory: $includeDir"
    Write-Info "Architecture: $arch"
    Write-Info "Modify PATH: $(-not $NoModifyPath)"

    if (-not $Yes) {
        Write-Host ""
        $response = Read-Host "    Proceed with installation? [Y/n]"
        if ($response -and $response -notmatch "^[Yy]") {
            Write-Warning "Installation cancelled."
            return
        }
    }

    # Create directories
    Write-Section "Creating directories"

    @($InstallDir, $binDir, $includeDir) | ForEach-Object {
        if (-not (Test-Path $_)) {
            New-Item -ItemType Directory -Path $_ -Force | Out-Null
            Write-Success "Created $_"
        } else {
            Write-Info "$_ already exists"
        }
    }

    # Check if we're doing a local install (from source) or download
    $localBuild = Join-Path $PSScriptRoot "..\build\Release"
    $useLocalBuild = Test-Path (Join-Path $localBuild "mana.exe")

    if ($useLocalBuild) {
        Write-Section "Installing from local build"
        Copy-LocalBuild $binDir $includeDir
    } else {
        Write-Section "Downloading Mana $MANA_VERSION"
        Download-Mana $binDir $includeDir $arch
    }

    # Modify PATH
    if (-not $NoModifyPath) {
        Write-Section "Configuring environment"
        if (Add-ToUserPath $binDir) {
            Write-Success "Added $binDir to PATH"
        } else {
            Write-Info "$binDir is already in PATH"
        }

        # Set MANA_HOME environment variable
        [Environment]::SetEnvironmentVariable("MANA_HOME", $InstallDir, "User")
        $env:MANA_HOME = $InstallDir
        Write-Success "Set MANA_HOME=$InstallDir"
    }

    # Create uninstall script
    Write-Section "Creating uninstaller"
    Create-Uninstaller
    Write-Success "Created uninstall.ps1"

    # Check for C++ compiler
    Write-Section "Checking prerequisites"
    Check-Prerequisites

    # Installation complete
    Write-Section "Installation complete!"
    Write-Host ""
    Write-Color "    Mana has been installed to: $InstallDir" Green
    Write-Host ""
    Write-Info "To get started, open a new terminal and run:"
    Write-Host ""
    Write-Color "        mana --version" Cyan
    Write-Host ""
    Write-Info "To compile a Mana program:"
    Write-Host ""
    Write-Color "        mana hello.mana -o hello.cpp" Cyan
    Write-Host ""
    Write-Info "Documentation: https://mana-lang.dev/docs"
    Write-Info "Report issues: $MANA_REPO/issues"
    Write-Host ""

    if (-not $NoModifyPath) {
        Write-Warning "NOTE: Restart your terminal for PATH changes to take effect."
    }
}

function Copy-LocalBuild {
    param(
        [string]$BinDir,
        [string]$IncludeDir
    )

    $sourceDir = Join-Path $PSScriptRoot ".."
    $buildDir = Join-Path $sourceDir "build\Release"

    # Copy executables
    $binaries = @("mana.exe", "mana-lsp.exe", "mana-debug.exe")
    foreach ($bin in $binaries) {
        $src = Join-Path $buildDir $bin
        if (Test-Path $src) {
            Copy-Item $src -Destination $BinDir -Force
            Write-Success "Installed $bin"
        } else {
            # Try Debug build
            $debugSrc = Join-Path (Join-Path $sourceDir "build\Debug") $bin
            if (Test-Path $debugSrc) {
                Copy-Item $debugSrc -Destination $BinDir -Force
                Write-Success "Installed $bin (debug build)"
            } else {
                Write-Warning "$bin not found, skipping"
            }
        }
    }

    # Copy runtime header
    $runtimeSrc = Join-Path $sourceDir "backend-cpp\mana_runtime.h"
    if (Test-Path $runtimeSrc) {
        Copy-Item $runtimeSrc -Destination $IncludeDir -Force
        Write-Success "Installed mana_runtime.h"
    }

    # Copy examples directory
    $examplesSrc = Join-Path $sourceDir "examples"
    $examplesDst = Join-Path $InstallDir "examples"
    if (Test-Path $examplesSrc) {
        if (-not (Test-Path $examplesDst)) {
            New-Item -ItemType Directory -Path $examplesDst -Force | Out-Null
        }
        Copy-Item "$examplesSrc\*.mana" -Destination $examplesDst -Force -ErrorAction SilentlyContinue
        Write-Success "Installed examples"
    }
}

function Download-Mana {
    param(
        [string]$BinDir,
        [string]$IncludeDir,
        [string]$Arch
    )

    $zipName = "mana-$MANA_VERSION-windows-$Arch.zip"
    $downloadUrl = "$MANA_RELEASE_URL/$zipName"
    $tempZip = Join-Path $env:TEMP $zipName
    $tempExtract = Join-Path $env:TEMP "mana-extract"

    Write-Info "Downloading from $downloadUrl"

    try {
        # Download with progress
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($downloadUrl, $tempZip)
        Write-Success "Downloaded $zipName"

        # Extract
        if (Test-Path $tempExtract) {
            Remove-Item $tempExtract -Recurse -Force
        }
        Expand-Archive -Path $tempZip -DestinationPath $tempExtract -Force
        Write-Success "Extracted archive"

        # Copy files
        $extractedBin = Join-Path $tempExtract "bin"
        if (Test-Path $extractedBin) {
            Copy-Item "$extractedBin\*" -Destination $BinDir -Force
        } else {
            Copy-Item "$tempExtract\*.exe" -Destination $BinDir -Force
        }

        $extractedInclude = Join-Path $tempExtract "include"
        if (Test-Path $extractedInclude) {
            Copy-Item "$extractedInclude\*" -Destination $IncludeDir -Force
        }

        Write-Success "Installed Mana binaries"

    } catch {
        Write-Error "Failed to download Mana: $_"
        Write-Host ""
        Write-Info "You can manually download from: $MANA_REPO/releases"
        Write-Info "Or build from source with: scripts\build.bat release"
        throw
    } finally {
        # Cleanup
        if (Test-Path $tempZip) { Remove-Item $tempZip -Force }
        if (Test-Path $tempExtract) { Remove-Item $tempExtract -Recurse -Force }
    }
}

function Check-Prerequisites {
    # Check for C++ compiler
    $hasMSVC = Test-Path "C:\Program Files\Microsoft Visual Studio"
    $hasClang = Test-Command "clang++"
    $hasGCC = Test-Command "g++"

    if ($hasMSVC -or $hasClang -or $hasGCC) {
        Write-Success "C++ compiler found"
        if ($hasMSVC) { Write-Info "  - Visual Studio detected" }
        if ($hasClang) { Write-Info "  - clang++ available" }
        if ($hasGCC) { Write-Info "  - g++ available" }
    } else {
        Write-Warning "No C++ compiler detected"
        Write-Info "  Mana compiles to C++, so you'll need a C++ compiler."
        Write-Info "  Recommended: Visual Studio 2022 with C++ workload"
        Write-Info "  Download: https://visualstudio.microsoft.com/"
    }

    # Check for CMake
    if (Test-Command "cmake") {
        Write-Success "CMake found"
    } else {
        Write-Info "CMake not found (optional, for building projects)"
    }

    # Check for VS Code
    if (Test-Command "code") {
        Write-Success "VS Code found"
        Write-Info "  Install Mana extension: code --install-extension mana-lang.mana"
    }
}

function Create-Uninstaller {
    $uninstallScript = @"
# Mana Uninstaller
# Generated by mana-init.ps1

`$InstallDir = "$InstallDir"
`$BinDir = "$InstallDir\bin"

Write-Host "Uninstalling Mana..."

# Remove from PATH
`$currentPath = [Environment]::GetEnvironmentVariable("PATH", "User")
`$paths = `$currentPath -split ";" | Where-Object { `$_ -ne `$BinDir -and `$_ -ne "" }
[Environment]::SetEnvironmentVariable("PATH", (`$paths -join ";"), "User")
Write-Host "Removed from PATH"

# Remove MANA_HOME
[Environment]::SetEnvironmentVariable("MANA_HOME", `$null, "User")
Write-Host "Removed MANA_HOME"

# Remove installation directory
if (Test-Path `$InstallDir) {
    Remove-Item `$InstallDir -Recurse -Force
    Write-Host "Removed `$InstallDir"
}

Write-Host ""
Write-Host "Mana has been uninstalled." -ForegroundColor Green
Write-Host "Thank you for using Mana!"
"@

    $uninstallPath = Join-Path $InstallDir "uninstall.ps1"
    Set-Content -Path $uninstallPath -Value $uninstallScript -Encoding UTF8
}

# ============================================================================
# Uninstall Function
# ============================================================================

function Uninstall-Mana {
    Write-Section "Uninstalling Mana"

    $binDir = Join-Path $InstallDir "bin"

    if (-not (Test-Path $InstallDir)) {
        Write-Warning "Mana is not installed at $InstallDir"
        return
    }

    if (-not $Yes) {
        Write-Info "This will remove Mana from: $InstallDir"
        $response = Read-Host "    Are you sure? [y/N]"
        if ($response -notmatch "^[Yy]") {
            Write-Warning "Uninstall cancelled."
            return
        }
    }

    # Remove from PATH
    Remove-FromUserPath $binDir
    Write-Success "Removed from PATH"

    # Remove MANA_HOME
    [Environment]::SetEnvironmentVariable("MANA_HOME", $null, "User")
    Write-Success "Removed MANA_HOME"

    # Remove installation directory
    Remove-Item $InstallDir -Recurse -Force
    Write-Success "Removed $InstallDir"

    Write-Host ""
    Write-Color "Mana has been uninstalled." Green
    Write-Host "Thank you for using Mana!"
}

# ============================================================================
# Main
# ============================================================================

Write-Banner

if ($Uninstall) {
    Uninstall-Mana
} else {
    Install-Mana
}
