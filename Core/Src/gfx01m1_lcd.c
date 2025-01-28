/*
 * bsp_lcd.c
 *
 *  Created on: Mar 9, 2024
 *      Author: abitr
 */
#include <gfx01m1_lcd.h>
#include "spi.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"


#define MADCTL_MY 					0x80  ///< Bottom to top
#define MADCTL_MX 					0x40  ///< Right to left
#define MADCTL_MV 					0x20  ///< Reverse Mode
#define MADCTL_ML 					0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 					0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 					0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 					0x04  ///< LCD refresh right to left


static void array_endian_swap16 (uint8_t *buf, uint16_t len)
{
	uint8_t temp;

	while (len > 0)
	{
		temp = *(buf + 1);
		*(buf + 1) = *buf;
		*buf = temp;
		buf += 2;  //sizeof(uint16_t)
		len -= 2;
	}
}

static void gfx01m1_lcd_resx_low (void)
{
	HAL_GPIO_WritePin(RESX_GPIO_Port, RESX_Pin, GPIO_PIN_RESET);
}


static void gfx01m1_lcd_resx_hi (void)
{
	HAL_GPIO_WritePin(RESX_GPIO_Port, RESX_Pin, GPIO_PIN_SET);
}


static void gfx01m1_lcd_csx_low (void)
{
	HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_RESET);
}


static void gfx01m1_lcd_csx_hi (void)
{
	HAL_GPIO_WritePin(CSX_GPIO_Port, CSX_Pin, GPIO_PIN_SET);
}


static void gfx01m1_lcd_dcx_low (void)
{
	HAL_GPIO_WritePin(DCX_GPIO_Port, DCX_Pin, GPIO_PIN_RESET);
}


static void gfx01m1_lcd_dcx_hi (void)
{
	HAL_GPIO_WritePin(DCX_GPIO_Port, DCX_Pin, GPIO_PIN_SET);
}


void gfx01m1_lcd_write_cmd(uint8_t cmd)
{
    gfx01m1_lcd_csx_low();
    gfx01m1_lcd_dcx_low();   // DCX=0 is for cmd
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    gfx01m1_lcd_dcx_hi();   // DCX=0 is for cmd
    gfx01m1_lcd_csx_hi();
}


void gfx01m1_lcd_write_data(uint8_t *buffer, uint32_t len)
{
    gfx01m1_lcd_csx_low();
    HAL_SPI_Transmit(&hspi1, buffer, len, HAL_MAX_DELAY);
    gfx01m1_lcd_csx_hi();
}


void gfx01m1_lcd_set_orientation(uint8_t orientation)
{
	uint8_t param;

	if (orientation == LANDSCAPE) {
		param = MADCTL_MV | MADCTL_MY | MADCTL_BGR; /*Memory Access Control <Landscape setting>*/
	}else if(orientation == PORTRAIT){
		param = MADCTL_MY| MADCTL_MX| MADCTL_BGR;  /* Memory Access Control <portrait setting> */
	}

	gfx01m1_lcd_write_cmd(ILI9341_MAC);    // Memory Access Control command
	gfx01m1_lcd_write_data(&param, 1);
}


void gfx01m1_lcd_reset(void)
{
	// pull RESX pin low for some time, then pull high
	gfx01m1_lcd_resx_low();
	//HAL_Delay(1);
	vTaskDelay(pdMS_TO_TICKS(1));

	gfx01m1_lcd_resx_hi();
	//HAL_Delay(1);
	vTaskDelay(pdMS_TO_TICKS(1));
}


void gfx01m1_lcd_config(void)
{
	uint8_t params[15];

	gfx01m1_lcd_write_cmd(ILI9341_SWRESET);

	gfx01m1_lcd_write_cmd(ILI9341_POWERB);
	params[0] = 0x00;
	params[1] = 0xD9;
	params[2] = 0x30;
	gfx01m1_lcd_write_data(params, 3);

	gfx01m1_lcd_write_cmd(ILI9341_POWER_SEQ);
	params[0]= 0x64;
	params[1]= 0x03;
	params[2]= 0x12;
	params[3]= 0x81;
	gfx01m1_lcd_write_data(params, 4);

	gfx01m1_lcd_write_cmd(ILI9341_DTCA);
	params[0]= 0x85;
	params[1]= 0x10;
	params[2]= 0x7A;
	gfx01m1_lcd_write_data(params, 3);

	gfx01m1_lcd_write_cmd(ILI9341_POWERA);
	params[0]= 0x39;
	params[1]= 0x2C;
	params[2]= 0x00;
	params[3]= 0x34;
	params[4]= 0x02;
	gfx01m1_lcd_write_data(params, 5);

	gfx01m1_lcd_write_cmd(ILI9341_PRC);
	params[0]= 0x20;
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_DTCB);
	params[0]= 0x00;
	params[1]= 0x00;
	gfx01m1_lcd_write_data(params, 2);

	gfx01m1_lcd_write_cmd(ILI9341_POWER1);
	params[0]= 0x1B;
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_POWER2);
	params[0]= 0x12;
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_VCOM1);
	params[0]= 0x08;
	params[1]= 0x26;
	gfx01m1_lcd_write_data(params, 2);

	gfx01m1_lcd_write_cmd(ILI9341_VCOM2);
	params[0]= 0xB7;
	gfx01m1_lcd_write_data(params, 1);


	gfx01m1_lcd_write_cmd(ILI9341_PIXEL_FORMAT);
	params[0]= 0x55; //select RGB565
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_FRMCTR1);
	params[0]= 0x00;
	params[1]= 0x1B;//frame rate = 70
	gfx01m1_lcd_write_data(params, 2);

	gfx01m1_lcd_write_cmd(ILI9341_DFC);    // Display Function Control
	params[0]= 0x0A;
	params[1]= 0xA2;
	gfx01m1_lcd_write_data(params, 2);

	gfx01m1_lcd_write_cmd(ILI9341_3GAMMA_EN);    // 3Gamma Function Disable
	params[0]= 0x02; //LCD_WR_DATA(0x00);
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_GAMMA);
	params[0]= 0x01;
	gfx01m1_lcd_write_data(params, 1);

	gfx01m1_lcd_write_cmd(ILI9341_PGAMMA);    //Set Gamma
	params[0]= 0x0F;
	params[1]= 0x1D;
	params[2]= 0x1A;
	params[3]= 0x0A;
	params[4]= 0x0D;
	params[5]= 0x07;
	params[6]= 0x49;
	params[7]= 0X66;
	params[8]= 0x3B;
	params[9]= 0x07;
	params[10]= 0x11;
	params[11]= 0x01;
	params[12]= 0x09;
	params[13]= 0x05;
	params[14]= 0x04;
	gfx01m1_lcd_write_data(params, 15);

	gfx01m1_lcd_write_cmd(ILI9341_NGAMMA);
	params[0]= 0x00;
	params[1]= 0x18;
	params[2]= 0x1D;
	params[3]= 0x02;
	params[4]= 0x0F;
	params[5]= 0x04;
	params[6]= 0x36;
	params[7]= 0x13;
	params[8]= 0x4C;
	params[9]= 0x07;
	params[10]= 0x13;
	params[11]= 0x0F;
	params[12]= 0x2E;
	params[13]= 0x2F;
	params[14]= 0x05;
	gfx01m1_lcd_write_data(params, 15);

	gfx01m1_lcd_write_cmd(ILI9341_SLEEP_OUT); //Exit Sleep
	vTaskDelay(pdMS_TO_TICKS(50));
	vTaskDelay(pdMS_TO_TICKS(50));
	gfx01m1_lcd_write_cmd(ILI9341_DISPLAY_ON); //display on

	gfx01m1_lcd_set_orientation(LANDSCAPE);
}



/* get pixels number given number of bytes.  assumes RGB565, so divide by 2 */
uint32_t bytes_to_pixels(uint32_t nbytes, uint32_t pixel_format)
{
	return (nbytes / 2);
}

uint32_t pixels_to_bytes(uint32_t pixels, uint32_t pixel_format)
{
	return (pixels * 2);
}


void gfx01m1_lcd_init(void)
{
	__HAL_SPI_ENABLE(&hspi1);

    gfx01m1_lcd_reset();
    gfx01m1_lcd_config();
}


void gfx01m1_lcd_set_display_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t params[4];

    // Column address set (2Ah)
    params[0] = (x1 >> 8U) & 0xFF;
    params[1] = (x1 >> 0U) & 0xFF;
    params[2] = (x2 >> 8U) & 0xFF;
    params[3] = (x2 >> 0U) & 0xFF;
    gfx01m1_lcd_write_cmd(ILI9341_CASET);
    gfx01m1_lcd_write_data(params, 4);

    params[0] = (y1 >> 8U) & 0xFF;
    params[1] = (y1 >> 0U) & 0xFF;
    params[2] = (y2 >> 8U) & 0xFF;
    params[3] = (y2 >> 0U) & 0xFF;
    gfx01m1_lcd_write_cmd(ILI9341_RASET);
    gfx01m1_lcd_write_data(params, 4);
}


void gfx01m1_lcd_draw_display(uint8_t *buf, uint16_t w, uint16_t h)
{
	uint16_t len = w * h * sizeof(uint16_t);

	array_endian_swap16(buf, len);

	gfx01m1_lcd_write_cmd(ILI9341_GRAM);
	gfx01m1_lcd_write_data(buf, len);
}


void gfx01m1_lcd_draw_display_dma(uint8_t *buf, uint16_t w, uint16_t h)
{
	uint16_t len = w * h * sizeof(uint16_t);

	array_endian_swap16(buf, len);

	gfx01m1_lcd_write_cmd(ILI9341_GRAM);

	gfx01m1_lcd_csx_low();
    HAL_SPI_Transmit_DMA(&hspi1, buf, len);
    //gfx01m1_lcd_csx_hi();

}


extern void DisplayDriver_TransferCompleteCallback(void);

static int display_driver_transmit_active = 0;

int touchgfxDisplayDriverTransmitActive(void)
{
	return display_driver_transmit_active;
}


void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	display_driver_transmit_active = 1;
	gfx01m1_lcd_set_display_area(x, y, (x + w - 1), (y + h - 1));
	gfx01m1_lcd_draw_display_dma((uint8_t *)pixels, w, h);
//	display_driver_transmit_active = 0;
//	DisplayDriver_TransferCompleteCallback();
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1)
	{
		// Unselect the SPI line
		gfx01m1_lcd_csx_hi();
		display_driver_transmit_active = 0;
		DisplayDriver_TransferCompleteCallback();
	}
}

