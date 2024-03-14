@echo off
setlocal

rem Copyright (c) 2024 Piotr Stradowski. All rights reserved.
rem Software distributed under the permissive MIT License.

set myPath=%~dp0

set sourceDir="%myPath%"

:: Find the first .sln file
for %%i in (*.sln) do (
   set sln_file=%%~fi
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
