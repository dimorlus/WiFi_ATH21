md bin
del bin\*.bin
rem @make BOOT=none APP=0 rebuild
rem copy firmware\*.bin bin\*.bin
@mingw32-make.exe BOOT=new APP=1 rebuild
copy firmware\upgrade\user?*.bin bin\user?.bin
@mingw32-make.exe BOOT=new APP=2 rebuild
copy firmware\upgrade\user?*.bin bin\user?.bin

::del sftp.log
::"C:\Program Files (x86)\WinSCP\WinSCP.exe" /script=sftp.txt /log=sftp.log
::grep -i "Transfer done:" sftp.log
::copy bin\user?.bin \\192.168.0.2\WebUpdate\PLUG_EM\user?.bin

::git add .
::git commit -m "%date% %time%"
::git push

"c:\program files\winrar\rar" u ..\..\..\backup\wifi_aht21 -r -ver -x.git\*.* -x.tmp.drivedownload\*.* -x@..\..\.gitignore -x..\..\Datasheets\*.* -ed ..\..\
