@echo off
REM Mana Language Build Script for Windows
REM Usage: build.bat [debug|release|clean|test|examples]

setlocal enabledelayedexpansion

set BUILD_TYPE=Release
set BUILD_DIR=build
set ACTION=build

if "%1"=="debug" (
    set BUILD_TYPE=Debug
    set ACTION=build
) else if "%1"=="release" (
    set BUILD_TYPE=Release
    set ACTION=build
) else if "%1"=="clean" (
    set ACTION=clean
) else if "%1"=="test" (
    set ACTION=test
) else if "%1"=="examples" (
    set ACTION=examples
) else if "%1"=="all" (
    set ACTION=all
) else if "%1"=="help" (
    goto :help
) else if not "%1"=="" (
    echo Unknown option: %1
    goto :help
)

if "%ACTION%"=="clean" (
    echo Cleaning build directories...
    if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
    if exist examples\build rmdir /s /q examples\build
    echo Done.
    goto :end
)

if "%ACTION%"=="build" (
    call :build_compiler
    goto :end
)

if "%ACTION%"=="test" (
    call :build_compiler
    call :run_tests
    goto :end
)

if "%ACTION%"=="examples" (
    call :build_compiler
    call :build_examples
    goto :end
)

if "%ACTION%"=="all" (
    call :build_compiler
    call :build_examples
    call :run_tests
    goto :end
)

:build_compiler
echo.
echo === Building Mana Compiler (%BUILD_TYPE%) ===
echo.
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
cmake --build . --config %BUILD_TYPE%
popd
echo.
echo Compiler built: %BUILD_DIR%\%BUILD_TYPE%\mana_lang.exe
exit /b 0

:build_examples
echo.
echo === Building Examples ===
echo.
if not exist examples\build mkdir examples\build
pushd examples\build
cmake ..
cmake --build . --config %BUILD_TYPE%
popd
echo.
echo Examples built in: examples\build\%BUILD_TYPE%
exit /b 0

:run_tests
echo.
echo === Running Tests ===
echo.
set COMPILER=%BUILD_DIR%\%BUILD_TYPE%\mana_lang.exe
if not exist "%COMPILER%" (
    echo Error: Compiler not found. Build first.
    exit /b 1
)

REM Run built-in test runner
"%COMPILER%" examples\testing_example.mana --test -c
if exist examples\build\%BUILD_TYPE%\testing_example.exe (
    examples\build\%BUILD_TYPE%\testing_example.exe
)
exit /b 0

:help
echo.
echo Mana Language Build Script
echo.
echo Usage: build.bat [command]
echo.
echo Commands:
echo   debug     Build compiler in debug mode
echo   release   Build compiler in release mode (default)
echo   clean     Remove all build artifacts
echo   test      Build and run tests
echo   examples  Build all example programs
echo   all       Build compiler, examples, and run tests
echo   help      Show this help message
echo.
exit /b 0

:end
endlocal
