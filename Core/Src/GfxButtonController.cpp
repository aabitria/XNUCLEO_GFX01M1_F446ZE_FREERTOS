/*
 * GfxButtonController.c
 *
 *  Created on: Jan 11, 2025
 *      Author: Lenovo310
 */
#include "GfxButtonController.hpp"
#include "main.h"
#include "gpio.h"

#define KEY_UP_IDX                      (0)
#define KEY_DOWN_IDX                    KEY_UP_IDX + 1

void GfxButtonController::init()
{
	for (uint16_t i = 0; i < num_buttons; i++)
	{
		active_val[i] = active_common;
		idle_val[i] = idle_common;

		if (active_val[i])		// high when pressed
		{
			active_mask_debounce[i] = mask;
			idle_mask_debounce[i] = 0;
		}
		else					// low when pressed
		{
			active_mask_debounce[i] = 0;
			idle_mask_debounce[i] = mask;
		}

		if (idle_val[i] != 0)
		{
			steady_state[i] = 1;
			state[i] = 1;
			prev_steady_state[i] = 1;
		}
		else
		{
			steady_state[i] = 0;
			state[i] = 0;
			prev_steady_state[i] = 0;
		}
	}
}

bool GfxButtonController::sample(uint8_t& key)
{
    if (detect_press_event(KEY_UP_IDX))
    {
    	key = KEY_UP_IDX;		// 0
    	return true;
    }

    if (detect_press_event(KEY_DOWN_IDX))
    {
    	key = KEY_DOWN_IDX;		// 1
    	return true;
    }

	return false;
}


uint16_t GfxButtonController::detect_press_event(uint16_t btn_idx)
{
	uint16_t btn_state = debounce(btn_idx);

	if ((btn_state != prev_steady_state[btn_idx]) && (btn_state == 0))
	{
		prev_steady_state[btn_idx] = btn_state;
		return 1;
	}

	prev_steady_state[btn_idx] = btn_state;
	return 0;
}


uint16_t GfxButtonController::debounce(uint16_t btn_idx)
{
	if (btn_idx == KEY_UP_IDX)
	{
		raw_val[KEY_UP_IDX] = HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin);
	}
	else
	{
		raw_val[KEY_DOWN_IDX] = HAL_GPIO_ReadPin(KEY_DN_GPIO_Port, KEY_DN_Pin);
	}

	state[btn_idx] = (state[btn_idx] << 1) | raw_val[btn_idx];

	// continuous pressed state
	if ((state[btn_idx] & 0x0003) == active_mask_debounce[btn_idx])
	{
		steady_state[btn_idx] = active_val[btn_idx];
		return active_val[btn_idx];
	}

	// continuous not pressed state
	if ((state[btn_idx] & 0x0003) == idle_mask_debounce[btn_idx])
	{
		steady_state[btn_idx] = idle_val[btn_idx];
		return idle_val[btn_idx];
	}

	// none of the above; glitch input that got filtered
	return steady_state[btn_idx];
}
