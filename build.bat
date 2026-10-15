@echo off
setlocal
cd /d "%~dp0"

echo === Configure Debug ===
cmake -S . -B build/cmake-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 goto error

echo === Build Debug ===
cmake --build build/cmake-debug --parallel
if errorlevel 1 goto error

echo.
echo === Build succeeded ===
echo Executable: build\cmake-debug\ExedraSim.exe
exit /b 0

:error
echo.
echo === Build failed ===
exit /b 1