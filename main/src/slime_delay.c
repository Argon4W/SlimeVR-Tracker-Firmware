#include "freertos/FreeRTOS.h"
#include "slime_delay.h"

void slime_delay_milliseconds(const uint32_t milliseconds) {
	vTaskDelay(pdMS_TO_TICKS(milliseconds));
}