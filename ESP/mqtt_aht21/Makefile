# Main settings includes
include	../settings.mk

#1MB (SONOFF, AWP07L)
#SPI_MODE = DOUT
#SPI_SIZE_MAP = 2
#APP = 0, 1/2
#BOOTLDR = boot_v1.3.bin

#HW-625 board
#SPI_MODE = DOUT
#SPI_SIZE_MAP = 4
#APP = 0, 1/2
#BOOTLDR = boot_v1.3.bin

#ESP-12F/S
#SPI_MODE = QIO
#SPI_SIZE_MAP = 4
#APP = 0, 1/2
#BOOTLDR = boot_v1.2.bin

# Individual project settings (Optional)
# BOOT = none
# BOOT = old - boot_v1.1
# BOOT = new - boot_v1.2+
BOOT = new
# APP = 0 - eagle.flash.bin + eagle.irom0text.bin
# APP = 1 - user1.bin
# APP = 2 - user2.bin
#APP = 1
#SPI_SPEED	= 40
#SPI_MODE	= DIO

SPI_SIZE_MAP = 2
SPI_MODE = DOUT
BOOTLDR = boot_v1.7.bin
#BOOTLDR = boot_v1.4.bin
#SPI_SIZE_MAP = 4
#SPI_MODE = QIO
#BOOTLDR = boot_v1.2.bin
#ESPPORT		= COM3
#ESPBAUD		= 256000
VERBOSE = 1
# Basic project settings

#XTENSA_TOOLS_ROOT = d:/PlatformIO/.platformio/packages/toolchain-xtensa/bin
XTENSA_TOOLS_ROOT = c:/Espressif/xtensa-lx106-elf/bin

SDK_VER = V2x
#SDK_VER = V3x
#SDK_BASE	= c:/Espressif/ESP8266_NONOS_SDK-2.1.0
SDK_BASE	= c:/Espressif/ESP8266_NONOS_SDK-V2.2.0
#SDK_BASE	= d:/ESP8266_NONOS_SDK/ESP8266_NONOS_SDK-2.2.1
#SDK_BASE	= d:/ESP8266_NONOS_SDK/ESP8266_NONOS_SDK-3.0.2

MODULES	= driver user mqtt modules
#LIBS	= c gcc hal phy pp net80211 lwip wpa crypto main ssl
LIBS	= c gcc phy pp net80211 lwip wpa crypto main upgrade ssl espnow
# Root includes
include	../common_nonos.mk
