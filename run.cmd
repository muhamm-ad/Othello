REM TO BE VERIFIED !!!
@echo off
setlocal

REM Get the current directory
set workspace=%cd%

cd %workspace%

REM If the build directory exists, delete it
if exist "build" (
    echo Deleting old build directory...
    rmdir /S /Q build
)

REM Create the build directory
echo Creating new build directory...
mkdir build
cd build

REM Run CMake
echo Running CMake...
cmake -G "NMake Makefiles" ..
if errorlevel 1 (
    echo CMake failed!
    exit /b
)

REM Run NMake
echo Running NMake...
nmake
if errorlevel 1 (
    echo NMake failed!
    exit /b
)

echo Build completed successfully!
endlocal

REM Ask the user if they want to run the game
set /P answer="Do you want to run the game? (y/n) "
if /I "%answer%" EQU "y" (
    REM Ask the user for their piece type
    set /P piece_type="Choose a piece type (X or O): "
    echo.
    
    REM Run the game with the chosen piece type
    Othello.exe %piece_type%
) else (
    echo Not running the game. You can run it later with '.\build\Othello.exe <X|O>'.
)
