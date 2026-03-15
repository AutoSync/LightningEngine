@echo off
REM Build script for LightningEngine using MSVC
REM Visual Studio path: D:\Arquivos\HDDR\Microsoft\VisualStudio\VS26\

setlocal enabledelayedexpansion

REM Set Visual Studio path
set VS_PATH=D:\Arquivos\HDDR\Microsoft\VisualStudio\VS26

REM Check if Visual Studio exists
if not exist "%VS_PATH%\Common7\Tools\VsDevCmd.bat" (
    echo ERROR: Visual Studio not found at %VS_PATH%
    echo Please verify the Visual Studio installation path.
    pause
    exit /b 1
)

REM Configuration (Debug or Release)
set CONFIG=Release
if not "%1"=="" set CONFIG=%1

REM Platform (x64 or Win32)
set PLATFORM=x64
if not "%2"=="" set PLATFORM=%2

echo ========================================
echo Building LightningEngine with MSVC
echo Visual Studio: %VS_PATH%
echo Configuration: %CONFIG%
echo Platform: %PLATFORM%
echo ========================================

REM Setup MSVC environment
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=%PLATFORM%

REM Build the solution
MSBuild LightningEngine.sln /p:Configuration=%CONFIG% /p:Platform=%PLATFORM% /m

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo Executable: %PLATFORM%\%CONFIG%\LightningEngine.exe
    echo ========================================
    
    REM Copy DLLs if needed
    if exist "third_party\SDL3\lib\x64\*.dll" (
        copy "third_party\SDL3\lib\x64\*.dll" "%PLATFORM%\%CONFIG%\" >nul 2>&1
        echo SDL3 DLLs copied to output directory.
    )
    
    REM Optionally run the executable
    echo.
    echo To run the application:
    echo   %PLATFORM%\%CONFIG%\LightningEngine.exe
) else (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    exit /b %ERRORLEVEL%
)

pause
