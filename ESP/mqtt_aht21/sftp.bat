del sftp.log
"C:\Program Files (x86)\WinSCP\WinSCP.exe" /script=sftp.txt /log=sftp.log
grep -i "Transfer done:" sftp.log
