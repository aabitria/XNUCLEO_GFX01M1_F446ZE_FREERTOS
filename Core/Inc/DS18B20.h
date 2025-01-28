/*
 * DS18B20.h
 *
 *  Created on: Jan 26, 2025
 *      Author: Lenovo310
 */

#ifndef INC_DS18B20_H_
#define INC_DS18B20_H_

#include "tim.h"


void DS18B20_Init (void);
void DS18B20_Generate_Reset (void);
void DS18B20_Idle (void);
uint16_t DS18B20_Get_Presence (void);
void DS18B20_Send_Cmd_SkipRom (void);
void DS18B20_Send_Cmd_ConverT (void);
void DS18B20_Send_Cmd_RdScr (void);
void DS18B20_Read (void);
uint16_t DS18B20_Get_Temp (void);
void DS18B20_Update_Data (void);

#endif /* INC_DS18B20_H_ */
