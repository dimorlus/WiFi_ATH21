@echo off
REM run as Administrator
set NSSM="c:\mqtt_logger\nssm.exe"
set EXE="c:\mqtt_logger\mqtt_logger.exe"
set SRVNAME=mqtt_logger
set APPDIR="c:\mqtt_logger"
set OUTLOG="c:\mqtt_logger\logs\stdout.log"
set ERRLOG="c:\mqtt_logger\logs\stderr.log"

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
c:\mqtt_logger\nssm.exe start mqtt_logger
pause