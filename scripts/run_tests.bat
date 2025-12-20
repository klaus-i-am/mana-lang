@echo off
setlocal enabledelayedexpansion

echo.
echo === Mana Language Test Suite ===
echo.

cd /d "%~dp0.."

:: Check if msbuild is available
where msbuild >nul 2>&1
if %errorlevel% neq 0 (
    echo MSBuild not found. Please run from Developer Command Prompt.
    echo Or set up Visual Studio environment first.
    exit /b 1
)

:: Build the test project
echo Building test suite...
msbuild tests\mana_tests\mana_tests.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal /nologo

if %errorlevel% neq 0 (
    echo.
    echo Build failed!
    exit /b 1
)

:: Run the tests
echo.
echo Running compiler unit tests...
tests\mana_tests\x64\Release\mana_tests.exe

set test_result=%errorlevel%

:: Run example compilation tests
echo.
echo === Example Compilation Tests ===
echo.

set passed=0
set failed=0
set total=0

:: Test compilation of examples
for %%f in (examples\*.mana) do (
    set /a total+=1
    echo Testing: %%f

    :: Try to compile the file
    mana %%f -c -o "%TEMP%\mana_test_output.cpp" >nul 2>&1

    if !errorlevel! equ 0 (
        echo   [PASS] %%~nxf compiled successfully
        set /a passed+=1
    ) else (
        echo   [FAIL] %%~nxf compilation failed
        set /a failed+=1
    )
)

echo.
echo === Example Compilation Results ===
echo Total: %total%, Passed: %passed%, Failed: %failed%
echo.

:: Clean up
del "%TEMP%\mana_test_output.cpp" 2>nul

if %test_result% neq 0 (
    echo Unit tests failed!
    exit /b 1
)

if %failed% gtr 0 (
    echo Some example compilations failed!
    exit /b 1
)

echo All tests passed!
exit /b 0
