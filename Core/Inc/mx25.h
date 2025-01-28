/*
 * mx25.h
 *
 *  Created on: Jan 3, 2025
 *      Author: Lenovo310
 */

#ifndef __MX25_H__
#define __MX25_H__

#include "spi.h"
#include "main.h"

#define MEMORY_FLASH_SIZE                       0x800000
#define MEMORY_BLOCK_SIZE                       0x10000
#define MEMORY_SECTOR_SIZE                      0x1000
#define MEMORY_PAGE_SIZE                        0x100
#define NUM_PAGES_PER_SECTOR                    (MEMORY_SECTOR_SIZE / MEMORY_PAGE_SIZE)

/* MX25L Commands */
#define RESET_EN                                0x66
#define RESET                                   0x99
#define READ_ID                                 0x9F

#define WRITE_ENABLE                            0x06
#define WRITE_DISABLE                           0x04
#define WRITE_STATUS_REG                        0x01
#define READ_STATUS_REG                         0x05

#define READ                                    0x03
#define FASTREAD                                0x0B
#define PAGE_PROGRAM                            0x02

#define SECTOR_ERASE_4K                        	0x20
#define BLK_ERASE_64K                           0xD8
#define CHIP_ERASE                              0x60


#define SPI                                     hspi2
#define SPI2_NSS_GPIO_Port                      SPI2_CS_GPIO_Port
#define SPI2_NSS_Pin                            SPI2_CS_Pin


void mx25_write (uint32_t page, uint32_t offset, uint8_t *data, uint16_t size);
void mx25_write_page (uint32_t page, uint32_t offset, uint8_t *data, uint16_t size);

void mx25_read (uint32_t start_page, uint32_t offset, uint8_t *data, uint16_t size);
void mx25_fastread (uint32_t start_page, uint32_t offset, uint8_t *data, uint16_t size);

void mx25_erase_sector (uint32_t sector);
void mx25_erase_chip (void);

void mx25_reset (void);
uint32_t mx25_readid (void);

/* Additional functions */
void flash_write_memory (uint8_t *buffer, uint32_t address, uint32_t buffer_size);
void flash_read_memory (uint32_t address, uint32_t buffer_size, uint8_t *buffer);
void flash_sector_erase (uint32_t erase_start_addr, uint32_t erase_end_addr);
void flash_chip_erase (void);
void flash_reset (void);

void DataReader_WaitForReceiveDone(void);
void DataReader_ReadData(uint32_t address24, uint8_t* buffer, uint32_t length);
void DataReader_StartDMAReadData(uint32_t address24, uint8_t* buffer, uint32_t length);


#endif /* INC_MX25_H_ */
