// #ifndef FLASH_PAGE_SIZE
// #define FLASH_PAGE_SIZE										0x800U
// #endif
/*
 * Internal flash distribution
 */
// #define FLASH_START_ADDR			0x08000000UL										// Flash start address
// #define FLASH_MAX_SIZE				0x00040000UL										// Max FLASH size - 256 Kbyte
// #define FLASH_END_ADDR				(FLASH_START_ADDR + FLASH_MAX_SIZE)					// FLASH end address
// #define BOOT_START_ADDR				0x08000000UL										// Bootloader start address
// #define BOOT_SIZE					0x0000e000UL										// 56 Kbyte for bootloader
// #define VERSION_ADDR				(FLASH_END_ADDR - 8)								// Version of software (0xffff_ffff - Error, try to reload firmware)

#define APP_START_ADDR				0x08020000UL										//User application start address
#define APP_START_SECTOR 			FLASH_SECTOR_5									//Setup used sector if start address is changing
#define LAST_SECTOR_NUM				FLASH_SECTOR_11
// #define APP_SIZE					0x00032000UL										//200 Kbyte for user application = ?? pages
// #define APP_NPAGES					(APP_SIZE / FLASH_PAGE_SIZE)