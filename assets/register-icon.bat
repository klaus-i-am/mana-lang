@echo off
:: Check for admin privileges and self-elevate if needed
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo Registering Mana file icon...
reg add "HKEY_CLASSES_ROOT\.mana" /ve /d "ManaSourceFile" /f
reg add "HKEY_CLASSES_ROOT\ManaSourceFile" /ve /d "Mana Source File" /f
reg add "HKEY_CLASSES_ROOT\ManaSourceFile\DefaultIcon" /ve /d "C:\dev\mana-lang\assets\mana.ico" /f
reg add "HKEY_CLASSES_ROOT\.rune" /ve /d "ManaSourceFile" /f
echo Done! Refreshing icons...
ie4uinit.exe -show
echo.
echo Restart Explorer or your PC if icons don't appear.
pause
