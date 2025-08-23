::esptool.exe -p COM3 read_mac
::esptool.exe -p COM3 read_mac > mac.txt
esptool.exe -p COM3 chip_id > mac.txt
esptool.exe -p COM3 chip_id
pause
