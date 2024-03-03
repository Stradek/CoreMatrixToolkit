@echo off
setlocal

rem Copyright (c) 2024 Piotr Stradowski. All rights reserved.
rem Software distributed under the permissive MIT License.

set myPath=%~dp0

set search_dir="%myPath%/build/CMake-VS2022-x64/"

:: Find the first .sln file
for /r "%search_dir%" %%i in (*.sln) do (
    set "sln_file=%%i"
    goto :open_solution
)

pause

exit /b

:open_solution
if defined sln_file (
    echo Found solution: "%sln_file%"
    start /b %sln_file%
) else (
    echo No .sln file found in %search_dir% and its subdirectories.
)
