@echo off
reg delete "HKCU\Software\Classes\.jett" /f >nul 2>&1
reg delete "HKCU\Software\Classes\JettFile" /f >nul 2>&1
rmdir /S /Q "%LOCALAPPDATA%\Jett" 2>nul
echo Jett .jett file icon removed.
pause
