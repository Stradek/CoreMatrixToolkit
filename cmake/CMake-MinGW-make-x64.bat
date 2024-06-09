@echo off
setlocal

rem Copyright (c) 2024 Piotr Stradowski. All rights reserved.
rem Software distributed under the permissive MIT License.

set myPath=%~dp0
set sourceDir=%1
set buildDir=%2
set presetsFile=%3

if [%sourceDir%] == [] set sourceDir="%myPath%\.."
if [%buildDir%] == [] set buildDir="%sourceDir%\build\mingw64-make"

echo ---- Preparing build directory...
mkdir %buildDir% >nul 2>&1

where /q cmake.exe
if %ERRORLEVEL% NEQ 0 (
    goto Error_CMakeNotInstalled
)

echo ---- Generating CMake project for Mingw32 Make...

cmake.exe --preset "x64-mingw64-make-debug" -S %sourceDir% -B %buildDir%

if %ERRORLEVEL% NEQ 0 (
    goto Error_CMakeBuildingProjectFailed
)

goto:EndCall

:Error_CMakeNotInstalled
echo "CMake is not installed or not in environmental path."
goto:EndCall

:Error_CMakeBuildingProjectFailed
echo "CMake failed to build the project for MinGW Makefiles."
goto:EndCall

:EndCall
pause
