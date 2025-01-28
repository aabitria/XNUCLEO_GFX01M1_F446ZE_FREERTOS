#include "mx25.h"

extern SPI_HandleTypeDef hspi2;

static inline void mx25_select (void)
{
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_RESET);
}


static inline void mx25_unselect (void)
{
	HAL_GPIO_WritePin(SPI2_NSS_GPIO_Port, SPI2_NSS_Pin, GPIO_PIN_SET);
}


static void spi_read (uint8_t *data, uint16_t size)
{
	HAL_SPI_Receive(&hspi2, data, size, HAL_MAX_DELAY);
}


static void spi_write (uint8_t *data, uint16_t size)
{
	HAL_SPI_Transmit(&hspi2, data, size, HAL_MAX_DELAY);
}


static void mx25_write_enable (void)
{
	uint8_t spi_buf = WRITE_ENABLE;

    mx25_select();
    spi_write(&spi_buf, 1);
    mx25_unselect();
}


static void mx25_write_disable (void)
{
	uint8_t spi_buf = WRITE_DISABLE;

    mx25_select();
    spi_write(&spi_buf, 1);
    mx25_unselect();
}


/*
 * How much can be written within a page given some offset within page
 * and the size to write.  Size can be less than page size, and can
 * be across pages.
 */
uint32_t bytes_to_write (uint32_t size, uint32_t offset)
{
    if ((size + offset) < MEMORY_PAGE_SIZE)
    {
    	return size;
    }
    else
    {
    	return (MEMORY_PAGE_SIZE - offset);
    }
}


uint32_t bytes_to_modify (uint32_t size, uint32_t offset)
{
	if ((size + offset) < MEMORY_SECTOR_SIZE)
	{
		return size;
	}
	else
	{
		return MEMORY_SECTOR_SIZE - offset;
	}
}

#if 0
/* No SEL */
static uint8_t mx25_read_sr (void)
{
    uint8_t spi_tx_buf = 0, spi_rx_buf = 0;

    spi_tx_buf = READ_STATUS_REG;

    mx25_select();
    spi_write(&spi_tx_buf, 1);
    spi_read(&spi_rx_buf, 1);
    mx25_unselect();

    return spi_rx_buf;
}

/* No SEL, no WREN */
static void mx25_write_sr (const uint8_t sr)
{
	uint8_t sreg = sr;
    uint8_t spi_tx_buf = WRITE_STATUS_REG;

	mx25_write_enable();

    mx25_select();
    spi_write(&spi_tx_buf, 1);
    spi_write(&sreg, 1);
    mx25_unselect();

    mx25_write_disable();
}
#endif

void mx25_wait_for_write (void)
{
    uint8_t spi_tx_buf = READ_STATUS_REG;
    uint8_t spi_rx_buf = 0;
	uint16_t cnt = 0;

    mx25_select();
    spi_write(&spi_tx_buf, 1);

    do
    {
    	spi_read(&spi_rx_buf, 1);
    	cnt++;
    } while (spi_rx_buf & 0x01);

    mx25_unselect();

	return;
}


void mx25_write_page (uint32_t page, uint32_t offset, uint8_t *data, uint16_t size)
{
	uint8_t  spi_buf[266];
    uint32_t start_page = page;
    uint32_t end_page = start_page + ((size + offset - 1) / MEMORY_PAGE_SIZE);
    // total number of page to change, partially or whole regardless
    uint32_t num_pages = end_page - start_page + 1;

    uint32_t start_sector = start_page / NUM_PAGES_PER_SECTOR;
    uint32_t end_sector = end_page / NUM_PAGES_PER_SECTOR;
    uint32_t num_sectors = end_sector - start_sector + 1;

    // Erase by sector (can't erase by page)
    for (uint16_t i = 0; i < num_sectors; i++)
    {
    	mx25_erase_sector(start_sector++);
    }

    // Indicates offset within the TOTAL data bytes, esp if it is > 1 page
    uint32_t data_pos = 0;

    /*
     * write the data per page iteratively
     */
    for (uint16_t i = 0; i < num_pages; i++)
    {
    	uint32_t mem_addr = (start_page * MEMORY_PAGE_SIZE) + offset;  // convert page/offset to absolute address
        // # of data bytes to send this cycle
    	uint32_t remaining_bytes = bytes_to_write(size, offset);
    	uint32_t idx;

    	mx25_write_enable();

    	spi_buf[0] = PAGE_PROGRAM;
    	spi_buf[1] = (mem_addr >> 16) & 0xFF;
    	spi_buf[2] = (mem_addr >> 8) & 0xFF;
    	spi_buf[3] = (mem_addr) & 0xFF;

    	// 4 bytes have been filled up out of 266, remember this location
    	idx = 4;

    	// send_bytes is #of data bytes plus cmd and addr bytes
    	uint32_t send_bytes = remaining_bytes + idx;

    	for (uint16_t i = 0; i < remaining_bytes; i++)
    	{
    		spi_buf[idx++] = data[data_pos + i];
    	}

    	if (send_bytes > 250)
    	{
        	mx25_select();
        	spi_write(spi_buf, 100);
        	spi_write(spi_buf + 100, send_bytes - 100);
        	mx25_unselect();
    	}
    	else
    	{
			mx25_select();
			spi_write(spi_buf, send_bytes);
			mx25_unselect();
    	}
    	/*
    	 * Update vars for next page cycle: start_page will advance to next,
    	 * offset will be 0 since it will be aligned to next page (only the first
    	 * and last may not be aligned or with offset), size will take into account
    	 * what has been written already, data_pos will advance to go to the next
    	 * unwritten bytes yet (so it is incremented by remaining_bytes)
    	 */
    	start_page++;
    	offset = 0; 		// for next page, offset will be aligned to page
        size = size - remaining_bytes;
        data_pos = data_pos + remaining_bytes;

        // Delay based on MX25 datasheet p. 32 on PP max duration
        //HAL_Delay(5);
        mx25_wait_for_write();
        mx25_write_disable();
    }
}


void mx25_write (uint32_t page, uint32_t offset, uint8_t *data, uint16_t size)
{
	uint16_t start_sector = page / NUM_PAGES_PER_SECTOR;
	uint16_t end_sector = (page + ((size + offset - 1) / MEMORY_PAGE_SIZE)) / NUM_PAGES_PER_SECTOR;
	uint16_t num_sectors = end_sector - start_sector + 1;

	uint8_t previous_data[MEMORY_SECTOR_SIZE];
	uint32_t sector_offset = ((page % NUM_PAGES_PER_SECTOR) * MEMORY_PAGE_SIZE) + offset;
	uint32_t data_idx = 0;

	for (uint16_t i = 0; i < num_sectors; i++)
	{
		// Do for every sector involved
		uint32_t start_page = start_sector * NUM_PAGES_PER_SECTOR;
		mx25_fastread(start_page, 0, previous_data, MEMORY_SECTOR_SIZE);

		uint16_t remaining_bytes = bytes_to_modify(size, sector_offset);
		for (uint16_t i = 0; i < remaining_bytes; i++)
		{
			previous_data[sector_offset + i] = data[data_idx + i];
		}

		mx25_write_page(start_page, 0, previous_data, MEMORY_SECTOR_SIZE);

		start_sector++;
		sector_offset = 0;
		data_idx = data_idx + remaining_bytes;
		size = size - remaining_bytes;

		//HAL_Delay(5);
		//mx25_write_disable();
	}
}


void mx25_write_byte (uint32_t addr, uint8_t data)
{
	uint8_t spi_tx_buf[5];

    spi_tx_buf[0] = PAGE_PROGRAM;
	spi_tx_buf[1] = (addr >> 16) & 0xFF;
	spi_tx_buf[2] = (addr >> 8) & 0xFF;
	spi_tx_buf[3] = (addr) & 0xFF;
	spi_tx_buf[4] = data;

	//check if location is erased first
	mx25_write_enable();

	mx25_select();
	spi_write(spi_tx_buf, 5);
	mx25_unselect();

	mx25_write_disable();
}


/*
 * The difference between normal read and fast read is that in MX25, normal
 * read is only up to 33MHz clock, while fast read is up to 66-86MHz clock,
 * depending on clock capacitance.
 */
void mx25_read (uint32_t start_page, uint32_t offset, uint8_t *data, uint16_t size)
{
    uint8_t spi_buf[4] = {0};			// Since our MX25 is only 8MB, addr is 24-bits only
    uint32_t mem_addr = (start_page * MEMORY_PAGE_SIZE) + offset;

    spi_buf[0] = READ;
    spi_buf[1] = (mem_addr >> 16) & 0xFF;
    spi_buf[2] = (mem_addr >> 8) & 0xFF;
    spi_buf[3] = mem_addr & 0xFF;

    mx25_select();
    spi_write(spi_buf, sizeof(spi_buf));
    spi_read(data, size);
    mx25_unselect();
}


void mx25_fastread (uint32_t start_page, uint32_t offset, uint8_t *data, uint16_t size)
{
    uint8_t spi_buf[5] = {0};			// Since our MX25 is only 8MB, addr is 24-bits only
    uint32_t mem_addr = (start_page * MEMORY_PAGE_SIZE) + offset;

    spi_buf[0] = FASTREAD;
    spi_buf[1] = (mem_addr >> 16) & 0xFF;
    spi_buf[2] = (mem_addr >> 8) & 0xFF;
    spi_buf[3] = mem_addr & 0xFF;
    spi_buf[4] = 0;   // dummy clock

    mx25_select();
    spi_write(spi_buf, sizeof(spi_buf));
    spi_read(data, size);
    mx25_unselect();
}


void mx25_fastread_dma (uint32_t start_page, uint32_t offset, uint8_t *data, uint16_t size)
{
    uint8_t spi_buf[5] = {0};			// Since our MX25 is only 8MB, addr is 24-bits only
    uint32_t mem_addr = (start_page * MEMORY_PAGE_SIZE) + offset;

    spi_buf[0] = FASTREAD;
    spi_buf[1] = (mem_addr >> 16) & 0xFF;
    spi_buf[2] = (mem_addr >> 8) & 0xFF;
    spi_buf[3] = mem_addr & 0xFF;
    spi_buf[4] = 0;   // dummy clock

    mx25_select();
    spi_write(spi_buf, sizeof(spi_buf));
    //spi_read(data, size);
    //mx25_unselect();

    HAL_SPI_Receive_DMA(&hspi2, data, size);
}



void mx25_erase_sector (uint32_t sector)
{
    uint8_t spi_buf[6];
    uint32_t mem_addr = sector * NUM_PAGES_PER_SECTOR* MEMORY_PAGE_SIZE;

    spi_buf[0] = SECTOR_ERASE_4K;
    spi_buf[1] = (mem_addr >> 16) & 0xFF;
    spi_buf[2] = (mem_addr >> 8) & 0xFF;
    spi_buf[3] = (mem_addr) & 0xFF;

    mx25_write_enable();
    mx25_select();
    spi_write(spi_buf, 4);
    mx25_unselect();

    //HAL_Delay(300);      // Max delay in datasheet for sector erase
    mx25_wait_for_write();


    mx25_write_disable();
}


void mx25_erase_chip (void)
{
    uint8_t spi_buf;

    spi_buf = CHIP_ERASE;

    mx25_write_enable();
    mx25_select();
    spi_write(&spi_buf, 1);
    mx25_unselect();

    mx25_wait_for_write();

    mx25_write_disable();
}


void mx25_reset (void)
{
    uint8_t cmd[2] = {RESET_EN, RESET};

    mx25_select();
	spi_write(cmd, 2);
	mx25_unselect();

	HAL_Delay(1);
}


uint32_t mx25_readid (void)
{
	uint8_t cmd = READ_ID;
	uint8_t data[3] = {0};

	mx25_select();
	spi_write(&cmd, sizeof(cmd));
	spi_read(data, sizeof(data));
	mx25_unselect();

	return ((data[0] << 16) | (data[1] << 8) | data[2]);
}


void flash_write_memory (uint8_t *buffer, uint32_t address, uint32_t buffer_size)
{
    uint32_t page = address / MEMORY_PAGE_SIZE;
    uint32_t offset = address % MEMORY_PAGE_SIZE;
    uint32_t size = buffer_size;
	uint8_t  spi_buf[266];
    uint32_t start_page = page;
    uint32_t end_page = start_page + ((size + offset - 1) / MEMORY_PAGE_SIZE);
    uint32_t num_pages = end_page - start_page + 1;

    // Indicates offset within the TOTAL data bytes, esp if it is > 1 page
    uint32_t data_pos = 0;

    /*
     * write the data per page iteratively
     */
    for (uint16_t i = 0; i < num_pages; i++)
    {
    	uint32_t mem_addr = (start_page * MEMORY_PAGE_SIZE) + offset;  // convert page/offset to absolute address
    	uint32_t remaining_bytes = bytes_to_write(size, offset);
    	uint32_t idx = 0;

    	mx25_write_enable();

    	spi_buf[0] = 0x02;
    	spi_buf[1] = (mem_addr >> 16) & 0xFF;
    	spi_buf[2] = (mem_addr >> 8) & 0xFF;
    	spi_buf[3] = (mem_addr) & 0xFF;

    	// 4 bytes have been filled up out of 266, remember this location
    	idx = 4;

    	// send_bytes is #of data bytes plus cmd and addr bytes
    	uint32_t send_bytes = remaining_bytes + idx;

    	for (uint16_t i = 0; i < remaining_bytes; i++)
    	{
    		spi_buf[idx++] = buffer[data_pos + i];
    	}

    	if (send_bytes > 250)
    	{
        	mx25_select();
        	spi_write(spi_buf, 100);
        	spi_write(spi_buf + 100, send_bytes - 100);
        	mx25_unselect();
    	}
    	else
    	{
			mx25_select();
			spi_write(spi_buf, send_bytes);
			mx25_unselect();
    	}
    	/*
    	 * Update vars for next page cycle: start_page will advance to next,
    	 * offset will be 0 since it will be aligned to next page (only the first
    	 * and last may not be aligned or with offset), size will take into account
    	 * what has been written already, data_pos will advance to go to the next
    	 * unwritten bytes yet (so it is incremented by remaining_bytes)
    	 */
    	start_page++;
    	offset = 0; 		// for next page, offset will be aligned to page
        size = size - remaining_bytes;
        data_pos = data_pos + remaining_bytes;

        // Delay based on MX25 datasheet p. 32 on PP max duration
        //HAL_Delay(5);
        mx25_wait_for_write();
        mx25_write_disable();
    }

}


void flash_read_memory (uint32_t address, uint32_t buffer_size, uint8_t *buffer)
{
	/* Convert to page and offset as those are what fastread() needs */
	uint32_t page = address / MEMORY_PAGE_SIZE;
	uint32_t offset = address % MEMORY_PAGE_SIZE;

	mx25_fastread(page, offset, buffer, buffer_size);
}


void flash_read_memory_dma (uint32_t address, uint32_t buffer_size, uint8_t *buffer)
{
	/* Convert to page and offset as those are what fastread() needs */
	uint32_t page = address / MEMORY_PAGE_SIZE;
	uint32_t offset = address % MEMORY_PAGE_SIZE;

	mx25_fastread_dma(page, offset, buffer, buffer_size);
}


void flash_sector_erase (uint32_t erase_start_addr, uint32_t erase_end_addr)
{
    uint32_t start_sector = erase_start_addr / MEMORY_SECTOR_SIZE;
    uint32_t end_sector = erase_end_addr / MEMORY_SECTOR_SIZE;
    uint32_t num_sectors = end_sector - start_sector + 1;

    // Erase per sector
    for (uint16_t i = 0; i < num_sectors; i++)
    {
    	mx25_erase_sector(start_sector++);
    }

}


void flash_chip_erase (void)
{
    mx25_erase_chip();
}


void flash_reset (void)
{
	mx25_reset();
}

static uint8_t datareader_dma_active = 0;

void DataReader_WaitForReceiveDone(void)
{
	while (datareader_dma_active != 0);

	return;
}


void DataReader_ReadData(uint32_t address24, uint8_t* buffer, uint32_t length)
{
	flash_read_memory(address24, length, buffer);
}


void DataReader_StartDMAReadData(uint32_t address24, uint8_t* buffer, uint32_t length)
{
	datareader_dma_active = 1;

	flash_read_memory_dma(address24, length, buffer);
}


void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI2)
	{
		mx25_unselect();
		datareader_dma_active = 0;
	}
}
