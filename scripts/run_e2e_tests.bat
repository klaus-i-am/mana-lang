@echo off
setlocal enabledelayedexpansion

REM Mana End-to-End Test Runner Script
REM Usage: run_e2e_tests.bat [options]
REM Options:
REM   -v, --verbose    Show detailed output
REM   -k, --keep       Keep compiled artifacts

echo.
echo === Mana End-to-End Tests ===
echo.

REM Parse arguments
set VERBOSE=
set KEEP=
:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="-v" set VERBOSE=-v
if "%~1"=="--verbose" set VERBOSE=-v
if "%~1"=="-k" set KEEP=-k
if "%~1"=="--keep" set KEEP=-k
shift
goto parse_args
:end_parse

REM Check if compiler exists
if not exist "build\Release\mana.exe" (
    echo Error: Compiler not found at build\Release\mana.exe
    echo Please build the compiler first: scripts\build.bat release
    exit /b 1
)

REM Check if E2E test runner exists
if not exist "build\Release\mana_e2e_tests.exe" (
    echo E2E test runner not found, building...
    call scripts\build.bat release
    if errorlevel 1 (
        echo Failed to build E2E test runner
        exit /b 1
    )
)

REM Create temp directory for compiled files
if not exist "build\e2e_temp" mkdir "build\e2e_temp"

REM Run E2E tests
echo Running end-to-end tests...
echo.

build\Release\mana_e2e_tests.exe %VERBOSE% %KEEP% --dir tests\e2e\tests

set RESULT=%ERRORLEVEL%

REM Cleanup temp directory if not keeping artifacts
if not "%KEEP%"=="-k" (
    if exist "build\e2e_temp" rd /s /q "build\e2e_temp" 2>nul
)

exit /b %RESULT%
