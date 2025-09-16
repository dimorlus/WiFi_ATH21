esptool.exe -p COM3 erase_flash
esptool.exe -p COM3 write_flash -ff 40m -fm qio -fs 4MB 0x3FC000 esp_init_data_default_v05.bin
esptool.exe -p COM3 -b 256000 write_flash -ff 40m -fm qio -fs 4MB 0x00000 boot_v1.7.bin
esptool.exe -p COM3 -b 256000 write_flash -ff 40m -fm qio -fs 4MB 0x77000 esp_ca_cert.bin 0x78000 esp_cert_private_key.bin
dir "..\..\ESP\mqtt_aht21\firmware\upgrade\user2.1024.new.2.bin"
esptool.exe -p COM3 -b 256000 write_flash -ff 40m -fm qio -fs 4MB 0x81000 "..\..\ESP\mqtt_aht21\firmware\upgrade\user2.1024.new.2.bin"
esptool.exe -p COM3 read_mac
pause
