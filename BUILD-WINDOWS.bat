@echo off
setlocal
cd /d "%~dp0"
echo ==============================================
echo TitanTune Metal - Windows x64 VST3 build
echo ==============================================
where cmake >nul 2>nul || (echo ERROR: Install CMake and Visual Studio 2022 C++ tools.& pause & exit /b 1)
where git >nul 2>nul || (echo ERROR: Install Git for Windows.& pause & exit /b 1)
if not exist "Source\signalsmith-linear\stft.h" (
  echo Downloading Signalsmith Linear dependency...
  git clone --depth 1 https://github.com/Signalsmith-Audio/linear.git linear-dependency || exit /b 1
  if not exist "Source\signalsmith-linear" mkdir "Source\signalsmith-linear"
  xcopy /E /I /Y "linear-dependency\include\signalsmith-linear\*" "Source\signalsmith-linear" >nul
  rmdir /S /Q linear-dependency
)
if not exist "Source\signalsmith-stretch.h" (
  echo Downloading Signalsmith Stretch dependency...
  powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -Uri 'https://raw.githubusercontent.com/Signalsmith-Audio/signalsmith-stretch/main/signalsmith-stretch.h' -OutFile 'Source/signalsmith-stretch.h'" || exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 || exit /b 1
cmake --build build --config Release --target TitanTuneMetal_VST3 || exit /b 1
echo.
echo Build finished. Check build\TitanTuneMetal_artefacts\Release\VST3pause
