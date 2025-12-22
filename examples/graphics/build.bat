@echo off
REM Mana Graphics Build Script
REM Usage: build.bat <mana_file>

if "%1"=="" (
    echo Usage: build.bat ^<mana_file^>
    echo Example: build.bat window.mana
    exit /b 1
)

set MANA_FILE=%1
set BASE_NAME=%~n1
set MANA_COMPILER=..\..\build\Release\mana_lang.exe
set VCPKG_ROOT=C:\dev\vcpkg

echo [1/4] Compiling Mana to C++...
%MANA_COMPILER% %MANA_FILE% --no-cache -c
if errorlevel 1 (
    echo Error: Mana compilation failed
    exit /b 1
)

echo [2/4] Patching C++ for graphics...
python -c "
import re
with open('%BASE_NAME%.cpp', 'r') as f:
    content = f.read()
pattern = r'extern \"C\" \{[^}]*\}\n*'
content = re.sub(pattern, '', content)
content = content.replace('#include \"mana_runtime.h\"', '#include \"mana_runtime.h\"\n#include \"mana_graphics.h\"')
with open('%BASE_NAME%.cpp', 'w') as f:
    f.write(content)
"

echo [3/4] Configuring CMake...
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake >nul 2>&1
if errorlevel 1 (
    echo Error: CMake configuration failed
    exit /b 1
)

echo [4/4] Building executable...
cmake --build build --config Release
if errorlevel 1 (
    echo Error: Build failed
    exit /b 1
)

echo.
echo Success! Executable: build\Release\%BASE_NAME%.exe
echo Run with: build\Release\%BASE_NAME%.exe
