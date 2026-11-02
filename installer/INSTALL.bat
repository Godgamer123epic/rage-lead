@echo off
setlocal
title RAGE LEAD installer
echo.
echo  ==========================================
echo    RAGE LEAD  -  installer
echo  ==========================================
echo.

set "SRC=%~dp0RAGE LEAD.vst3"
set "DEST=%CommonProgramFiles%\VST3"

if not exist "%SRC%" (
    echo  ERROR: I can't find the "RAGE LEAD.vst3" folder next to this file.
    echo  Please UNZIP the whole download first, then run INSTALL.bat again.
    echo.
    pause
    exit /b 1
)

REM ---- ask for administrator permission if we do not have it yet ----
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo  Asking Windows for administrator permission...
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

echo  Installing to: %DEST%
if not exist "%DEST%" mkdir "%DEST%"
if exist "%DEST%\RAGE LEAD.vst3" rmdir /s /q "%DEST%\RAGE LEAD.vst3"

xcopy "%SRC%" "%DEST%\RAGE LEAD.vst3\" /E /I /Y /Q >nul
if %errorlevel% neq 0 (
    echo.
    echo  ERROR: copying failed. Close FL Studio and try again.
    pause
    exit /b 1
)

REM ---- remove the "downloaded from the internet" block so Windows/FL accept it ----
powershell -NoProfile -Command "Get-ChildItem -LiteralPath '%DEST%\RAGE LEAD.vst3' -Recurse -File | Unblock-File" >nul 2>&1

echo.
echo  DONE!  RAGE LEAD is installed.
echo.
echo  Next: open FL Studio ^> Options ^> Manage plugins ^> "Find installed plugins".
echo.
pause
