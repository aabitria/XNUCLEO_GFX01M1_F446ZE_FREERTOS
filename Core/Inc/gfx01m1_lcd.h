/*
 * bsp_lcd.h
 *
 *  Created on: Mar 9, 2024
 *      Author: abitr
 */

#ifndef BSP_LCD_H_
#define BSP_LCD_H_

#include "ili9341_reg.h"
#include "main.h"

#define BSP_LCD_WIDTH				240
#define BSP_LCD_HEIGHT				320

#define RESX_GPIO_Port              LCD_RST_GPIO_Port
#define RESX_Pin                    LCD_RST_Pin
#define CSX_GPIO_Port               SPI1_CS_GPIO_Port
#define CSX_Pin                     SPI1_CS_Pin
#define DCX_GPIO_Port               LCD_DCX_GPIO_Port
#define DCX_Pin                     LCD_DCX_Pin

/* pixel format */
#define BSP_LCD_PIXEL_FMT_L8		1
#define BSP_LCD_PIXEL_FMT_RGB565	2
#define BSP_LCD_PIXEL_FMT_RGB666	3
#define BSP_LCD_PIXEL_FMT_RGB888	4
#define BSP_LCD_PIXEL_FMT			BSP_LCD_PIXEL_FMT_RGB565

/* orientation */
#define PORTRAIT					0
#define LANDSCAPE					1
#define BSP_LCD_ORIENTATION			LANDSCAPE

#if (BSP_LCD_ORIENTATION == PORTRAIT)
#define BSP_LCD_ACTIVE_WIDTH		BSP_LCD_WIDTH
#define BSP_LCD_ACTIVE_HEIGHT		BSP_LCD_HEIGHT
#else
#define BSP_LCD_ACTIVE_WIDTH		BSP_LCD_HEIGHT
#define BSP_LCD_ACTIVE_HEIGHT		BSP_LCD_WIDTH
#endif



void gfx01m1_lcd_init(void);

int touchgfxDisplayDriverTransmitActive(void);
void touchgfxDisplayDriverTransmitBlock(const uint8_t* pixels, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#endif /* BSP_LCD_H_ */
