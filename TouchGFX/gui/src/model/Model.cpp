#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdint>
#include "cmsis_os2.h"

extern "C" {
    extern osMessageQueueId_t sensor_queueHandle;
}

uint16_t raw_temp;
uint8_t prio;

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	float temp;
	osMessageQueueGet(sensor_queueHandle, &raw_temp, &prio, 0);

	temp = (float)raw_temp / 16.0f;

	modelListener->show_temp(temp);
}
