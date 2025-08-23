@echo off
setlocal

if "%VCPKG_ROOT%"=="" set VCPKG_ROOT=C:\vcpkg
set TRIPLET=x64-mingw-static

rem очистить старую сборку
if exist build rmdir /s /q build

cmake -S . -B build -G "MinGW Makefiles" ^
  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
  -DVCPKG_TARGET_TRIPLET=%TRIPLET% ^
  -DVCPKG_LIBRARY_LINKAGE=static ^
  -DCMAKE_BUILD_TYPE=Release

cmake --build build -- -j 4

copy config.ini build\config.ini
echo.
echo Build finished. Check build\mqtt_logger.exe and DLLs:
dir /b build\*.dll
pause
endlocal
