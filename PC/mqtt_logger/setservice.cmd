@echo off
REM run as Administrator
set NSSM="D:\Tools\nssm\nssm.exe"
set EXE="D:\Projects\LED\Alex\WiFi_AHT21\PC\mqtt_logger\build\mqtt_logger.exe"
set SRVNAME=mqtt_logger
set APPDIR="D:\Projects\LED\Alex\WiFi_AHT21\PC\mqtt_logger\build"
set OUTLOG="D:\Projects\LED\Alex\WiFi_AHT21\PC\mqtt_logger\logs\stdout.log"
set ERRLOG="D:\Projects\LED\Alex\WiFi_AHT21\PC\mqtt_logger\logs\stderr.log"

REM create logs dir
if not exist "%~dp0logs" mkdir "%~dp0logs"

"%NSSM%" install %SRVNAME% %EXE%
"%NSSM%" set %SRVNAME% AppDirectory %APPDIR%
"%NSSM%" set %SRVNAME% AppStdout %OUTLOG%
"%NSSM%" set %SRVNAME% AppStderr %ERRLOG%
"%NSSM%" set %SRVNAME% AppRotateFiles 1
"%NSSM%" set %SRVNAME% Start SERVICE_AUTO_START

echo Service %SRVNAME% installed. Start it with:
echo     "%NSSM%" start %SRVNAME%
pause