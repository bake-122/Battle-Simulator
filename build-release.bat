@echo off
setlocal
cd /d "%~dp0"

echo === Configure Release ===
cmake -S . -B build/cmake-release -G Ninja -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto error

echo === Build Release ===
cmake --build build/cmake-release --parallel
if errorlevel 1 goto error

echo.
echo === Build succeeded ===
echo Executable: build\cmake-release\ExedraSim.exe
exit /b 0

:error
echo.
echo === Build failed ===
exit /b 1