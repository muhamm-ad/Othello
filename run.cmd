@echo off
setlocal EnableDelayedExpansion

REM Safe to run from a normal Command Prompt or PowerShell. The script loads the
REM MSVC environment itself so CMake does not pick MinGW (which cannot use the
REM x64-windows SFML package vcpkg installs).

REM %~dp0 is the drive+path of THIS script, regardless of where it's invoked from
set "workspace=%~dp0"
if "%workspace:~-1%"=="\" set "workspace=%workspace:~0,-1%"
cd /d "%workspace%"

REM Ninja + vcpkg's x64-windows triplet need MSVC, not MinGW's g++.
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x64" (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" (
        echo Could not find Visual Studio. Install "Desktop development with C++"
        echo and rerun this script, or open a Developer Command Prompt for VS.
        exit /b 1
    )
    set "VCVARS="
    for /f "usebackq delims=" %%i in (`"!VSWHERE!" -latest -products * -find VC\Auxiliary\Build\vcvarsall.bat`) do set "VCVARS=%%i"
    if not defined VCVARS (
        echo Found Visual Studio but not the C++ toolset ^(vcvarsall.bat^).
        echo Install the "Desktop development with C++" workload and rerun.
        exit /b 1
    )
    echo Loading MSVC x64 environment...
    call "!VCVARS!" x64
    if errorlevel 1 (
        echo Failed to load the Visual Studio developer environment.
        exit /b 1
    )
)
set "CC=cl"
set "CXX=cl"
where cl >nul 2>&1
if errorlevel 1 (
    echo cl.exe is not on PATH. Cannot configure with MSVC.
    exit /b 1
)

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

REM A previous configure from a MinGW PATH must not be reused: that cache
REM points at g++ and find_package(SFML 3) then rejects the MSVC package.
if exist "build\CMakeCache.txt" (
    findstr /I /C:"MinGW" /C:"g++" /C:"c++.exe" "build\CMakeCache.txt" >nul 2>&1
    if not errorlevel 1 (
        echo Clearing CMake cache generated with MinGW...
        del /q "build\CMakeCache.txt"
        if exist "build\CMakeFiles" rmdir /S /Q "build\CMakeFiles"
    )
)

REM Configure (safe to re-run every time; CMake only regenerates what's stale)
REM Local dev builds default to Debug for faster iteration; set BUILD_TYPE=Release beforehand to override.
if "%BUILD_TYPE%"=="" set "BUILD_TYPE=Debug"
echo Configuring with CMake preset '%BUILD_TYPE%'...
cmake --preset %BUILD_TYPE% -DCMAKE_CXX_COMPILER=cl
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