@echo off
setlocal
set "ICON_DIR=%LOCALAPPDATA%\Jett"
set "ICON_FILE=%ICON_DIR%\jett-file-icon.ico"

if not exist "%ICON_DIR%" mkdir "%ICON_DIR%"
copy /Y "%~dp0jett-file-icon.ico" "%ICON_FILE%" >nul

reg add "HKCU\Software\Classes\.jett" /ve /d "JettFile" /f >nul
reg add "HKCU\Software\Classes\JettFile" /ve /d "Jett Source File" /f >nul
reg add "HKCU\Software\Classes\JettFile\DefaultIcon" /ve /d "%ICON_FILE%" /f >nul

echo.
echo Jett .jett file icon installed successfully.
echo If Explorer does not update immediately, restart Windows Explorer.
echo.
pause
