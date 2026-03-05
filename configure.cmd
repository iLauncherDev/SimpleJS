@echo off
setlocal

if not exist output-msvc mkdir output-msvc
cd output-msvc

cmake .. ^
  -G "Ninja" ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_C_COMPILER=cl ^
  -DCMAKE_CXX_COMPILER=cl ^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL

endlocal
