@echo off
setlocal

REM Run this from a "Developer Command Prompt for VS" or "Developer PowerShell for VS" so cl.exe is on PATH
REM Ninja still needs the MSVC compiler, same requirement as the old NMake setup.

REM %~dp0 is the drive+path of THIS script, regardless of where it's invoked from
set "workspace=%~dp0"
if "%workspace:~-1%"=="\" set "workspace=%workspace:~0,-1%"
cd /d "%workspace%"

REM vcpkg bootstrap (once per machine, gitignored)
REM Dependencies are declared in vcpkg.json; cmake picks them up via the toolchain file below.
if not exist "vcpkg\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    if exist "vcpkg" (
        echo Removing incomplete vcpkg checkout...
        rmdir /S /Q vcpkg
    )
    git clone https://github.com/microsoft/vcpkg.git vcpkg
    if errorlevel 1 (
        echo Failed to clone vcpkg!
        exit /b 1
    )
    call vcpkg\bootstrap-vcpkg.bat -disableMetrics
    if errorlevel 1 (
        echo Failed to bootstrap vcpkg!
        exit /b 1
    )
)

REM Optional manual clean: .\run.cmd --clean
if "%~1"=="--clean" (
    echo Removing build directory ^(--clean requested^)...
    if exist "build" rmdir /S /Q build
)

REM Configure (safe to re-run every time; CMake only regenerates what's stale)
REM Local dev builds default to Debug for faster iteration; set BUILD_TYPE=Release beforehand to override.
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Debug"
echo Configuring with CMake preset '%BUILD_TYPE%'...
cmake --preset %BUILD_TYPE%
if errorlevel 1 (
    echo CMake configure failed!
    exit /b 1
)

REM Build (incremental: only rebuilds what changed since the last run)
echo Building...
cmake --build build --parallel
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo.

set /P answer="Do you want to run the game? (Y/n) "
if /I "%answer%"=="n" (
    echo Not running the game. You can run it later with '.\build\Othello.exe'.
) else if /I "%answer%"=="y" (
    echo.
    build\Othello.exe
) else if "%answer%"=="" (
    echo.
    build\Othello.exe
)

endlocal