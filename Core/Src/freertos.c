/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DS18B20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
TaskHandle_t sensor_taskHandle = NULL;
/* USER CODE END Variables */
/* Definitions for sensor_task */
//osThreadId_t sensor_taskHandle;
//const osThreadAttr_t sensor_task_attributes = {
//  .name = "sensor_task",
//  .stack_size = 512 * 4,
//  .priority = (osPriority_t) osPriorityNormal,
//};
/* Definitions for TouchGFXTask */
osThreadId_t TouchGFXTaskHandle;
const osThreadAttr_t TouchGFXTask_attributes = {
  .name = "TouchGFXTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for sensor_queue */
osMessageQueueId_t sensor_queueHandle;
const osMessageQueueAttr_t sensor_queue_attributes = {
  .name = "sensor_queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void start_sensor_task(void *argument);
extern void TouchGFX_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sensor_queue */
  sensor_queueHandle = osMessageQueueNew (1, sizeof(uint16_t), &sensor_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of sensor_task */
  //sensor_taskHandle = osThreadNew(start_sensor_task, NULL, &sensor_task_attributes);
  if (xTaskCreate(start_sensor_task, "SENSOR", 512, NULL, 5, &sensor_taskHandle) != pdPASS)
  {
	  while(1);
  }

  /* creation of TouchGFXTask */
  TouchGFXTaskHandle = osThreadNew(TouchGFX_Task, NULL, &TouchGFXTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_start_sensor_task */
/**
  * @brief  Function implementing the sensor_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_start_sensor_task */
void start_sensor_task(void *argument)
{
  /* USER CODE BEGIN start_sensor_task */
  uint16_t Raw_Temp;
  uint32_t ulNotifValue;

  //DS18B20_Generate_Reset();
  //vTaskDelay(pdMS_TO_TICKS(1));
  /* Infinite loop */
  for(;;)
  {
//	  if (ds18b20_read_raw_temp(&raw_temp) == 1)
//	  {
//		  osMessageQueuePut(sensor_queueHandle, &raw_temp, 1, 0);
//	  }

	  DS18B20_Generate_Reset();
	  //xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  if (DS18B20_Get_Presence() <= 510)
	  {
          while(1);
	  }
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Send_Cmd_SkipRom();
	  xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Send_Cmd_ConverT();
	  xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Generate_Reset();
	  //xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  if (DS18B20_Get_Presence() <= 510)
	  {
          while(1);
	  }
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Send_Cmd_SkipRom();
	  xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Send_Cmd_RdScr();
	  xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();
	  vTaskDelay(pdMS_TO_TICKS(2));

	  DS18B20_Read();
	  xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifValue, portMAX_DELAY);
	  DS18B20_Idle();

	  DS18B20_Update_Data();
	  Raw_Temp = DS18B20_Get_Temp();
	  osMessageQueuePut(sensor_queueHandle, &Raw_Temp, 1, 0);
	  vTaskDelay(pdMS_TO_TICKS(300));
  }
  /* USER CODE END start_sensor_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

