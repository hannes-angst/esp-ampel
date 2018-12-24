#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "spi_flash.h"
#include "osapi.h"
#include "user_interface.h"
#include "app_config.h"
#include "user_config.h"

SYSCFG sysCfg;
SYSCFG myCfg;

bool ICACHE_FLASH_ATTR CFG_Reset() {
	SpiFlashOpResult res = spi_flash_erase_sector(CFG_LOCATION);
	if (res != SPI_FLASH_RESULT_OK) {
		INFO("Failed erase config.\r\n");
		return false;
	}
	return true;
}

bool ICACHE_FLASH_ATTR CFG_Save() {
	if (os_memcmp(&myCfg, &sysCfg, sizeof(sysCfg))) {
		INFO("CONFIG: Save configuration to flash ...\r\n");
		myCfg = sysCfg;
		SpiFlashOpResult res;

		res = spi_flash_erase_sector(CFG_LOCATION);
		if (res != SPI_FLASH_RESULT_OK) {
			INFO("Failed erase config.\r\n");
			return false;
		}

		res = spi_flash_write((CFG_LOCATION) * SPI_FLASH_SEC_SIZE, (uint32 *) &myCfg, sizeof(SYSCFG));
		if (res != SPI_FLASH_RESULT_OK) {
			INFO("Failed write config.\r\n");
			return false;
		}

		res = spi_flash_erase_sector(0x7E);
		if (res != SPI_FLASH_RESULT_OK) {
			INFO("Failed erase wifi settings.\r\n");
			return false;
		}

		INFO("done\r\n");
		return true;
	}

	return false;
}

bool ICACHE_FLASH_ATTR CFG_Load() {
	INFO("CONFIG: Load configuration from flash ...\r\n");
	spi_flash_read((CFG_LOCATION) * SPI_FLASH_SEC_SIZE, (uint32 *) &myCfg, sizeof(SYSCFG));
	if (myCfg.cfg_holder != CFG_HOLDER) {
		return false;
	}

	INFO("SSID: %s\r\n", myCfg.sta_ssid);
	INFO("PSK: %s\r\n", myCfg.sta_pwd);

	sysCfg = myCfg;
	INFO("done\r\n");
	return true;
}
