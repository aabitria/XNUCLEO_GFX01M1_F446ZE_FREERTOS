# XNUCLEO_GFX01M1_F446ZE_FREERTOS

This project displays ambient temperature on an LCD screen along with other demo images.  It uses XNUCLEO_GFX01M1 board with NUCLEO-F446ZE for display and uses DS18B20 for temperature sensing.

The DS18B20 driver uses Timer1 and DMA2 direct register access for control which was developed in another project.  The LCD uses TouchGFX graphics framework from STM32 and also uses FreeRTOS for overall tasks control.

The other GPIOs used are the following:

PE11 - Input capture, directly connected to DS18B20 DQ line

PE13 - PWM, drives a switching NPN transistor base, with the collector connected to DQ line.

