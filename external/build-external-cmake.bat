@echo off
REM require i686-w64-mingw32 cmake ninja git
chcp 65001

set ORIG_DIR=%CD%
set WORK_DIR=%~dp0
set BUILD_GENERATOR=Ninja
set BUILD_CMD=ninja -j 0
set BUILD_TYPE=Release

cd /d %WORK_DIR%
cmake -Bcmake-build -H. -G "%BUILD_GENERATOR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% 
if %errorlevel% neq 0 goto _EXIT

cd cmake-build
%BUILD_CMD% zlib liblzma
if %errorlevel% neq 0 goto _EXIT

%BUILD_CMD% libarchive
if %errorlevel% neq 0 goto _EXIT
strip -s %WORK_DIR%install\bin\libarchive.dll

:_EXIT
cd /d %ORIG_DIR%
exit /b %errorlevel%