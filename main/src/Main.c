/*
* SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SlimeI2C.h"

void app_main(void) {
	printf("Hello world!\n");

	ptr(I2CRuntimeContext) I2CContext = alloc_ptr(I2CRuntimeContext);

	ESP_ERROR_CHECK(newI2CRuntimeContext(I2CContext));

	while (true) {
		u8 buffer[1];

		I2CContext->platformI2CReadFunc(I2CContext->QMC6309_I2CDeviceContext, 0x00, buffer, 1);

		printf("QMC6309 ChipID: 0x%02" PRIX8 "\n", buffer[0]);

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}