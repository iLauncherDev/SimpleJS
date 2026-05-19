@echo off
setlocal

if not defined VCINSTALLDIR (
  echo cannot detect MSVC compiler
  goto quit
)

cl 2>&1 | find "x86" > NUL && set ARCH=i386
cl 2>&1 | find "x64" > NUL && set ARCH=amd64
cl 2>&1 | find "ARM" > NUL && set ARCH=arm
cl 2>&1 | find "ARM64" > NUL && set ARCH=arm64

set BUILD_DIR=output-msvc-%ARCH%

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake .. ^
  -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_C_COMPILER=cl ^
  -DCMAKE_CXX_COMPILER=cl ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL ^
  %*

:quit
endlocal
