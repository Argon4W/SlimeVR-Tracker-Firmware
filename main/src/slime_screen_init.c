#include "stdlib.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "slime_screen.h"

/**
 * @brief The log tag of the Slime Screen Initialization.
 */
static const char* TAG = "slime_screen_init";

/**
 * @brief			The transmission task of the screen.
 * @param parameter	The context of the screen to be transmitted, should always be slime_screen_context_t*.
 */
static void slime_screen_transmit_task(void* parameter);

esp_err_t slime_screen_context_new(
			slime_screen_context_t**		screen_context_out,
			slime_lcd_context_t*			lcd_context,
	const	slime_screen_context_config_t*	screen_context_config
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration, an LCD context, and a handle to receive the created screen context.
	ESP_RETURN_ON_FALSE(screen_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t handle provided when creating screen context.");
	ESP_RETURN_ON_FALSE(screen_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_config_t handle provided when creating screen context.");
	ESP_RETURN_ON_FALSE(lcd_context				!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_lcd_context_t handle provided when creating screen context.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving handles of screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Reserve the handles.
	slime_screen_context_t*				screen_context						= NULL;
	esp_fast_lcd_panel_device_t*		screen_fast_lcd_panel_device		= NULL;
	esp_fast_text_engine_instance_t*	screen_fast_text_engine_instance	= NULL;
	TaskHandle_t						screen_transmit_task_handle			= NULL;

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating fast LCD panel device.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Create the fast LCD panel device.
	ESP_GOTO_ON_ERROR(esp_fast_lcd_new_lcd_panel_device(
		/* panel_device_ret				= */ &screen_fast_lcd_panel_device,
		/* panel_device_configuration	= */ screen_context_config	->screen_fast_lcd_panel_config,
		/* panel_handle					= */ lcd_context			->panel_handle,
		/* panel_io						= */ lcd_context			->panel_io_handle
	), error, TAG, "Failed to create fast LCD panel device.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating fast text engine instance.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Create the fast text engine instance.
	ESP_GOTO_ON_ERROR(esp_fast_text_engine_new_text_engine_instance(
		/* engine_instance_ret				= */ &screen_fast_text_engine_instance,
		/* engine_instance_configuration	= */ screen_context_config->screen_fast_text_engine_instance_config,
		/* engine_instance_font				= */ screen_context_config->screen_fast_text_engine_font
	), error, TAG, "Failed to create fast text engine instance.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating screen context struct.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Allocate the screen context handle.
	screen_context = calloc(1, sizeof(slime_screen_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(screen_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the screen context struct.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Filling the screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Fill the screen context.
	screen_context->lcd_context					= lcd_context;
	screen_context->fast_lcd_panel_device		= screen_fast_lcd_panel_device;
	screen_context->fast_text_engine_instance	= screen_fast_text_engine_instance;
	screen_context->transmit_task_handle		= screen_transmit_task_handle;
	screen_context->transmit_framerate			= screen_context_config->screen_transmit_task_config.transmit_task_framerate;

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Registering transmission task.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Create the transmission task.
	ESP_GOTO_ON_FALSE(xTaskCreatePinnedToCore(
		/* pxTaskCode		= */ slime_screen_transmit_task,
		/* pcName			= */ "slime_screen_transmission_task",
		/* usStackDepth		= */ screen_context_config->screen_transmit_task_config.transmit_task_stack_depth,
		/* pvParameters		= */ screen_context,
		/* uxPriority		= */ screen_context_config->screen_transmit_task_config.transmit_task_priority,
		/* pxCreatedTask	= */ &screen_transmit_task_handle,
		/* xCoreID			= */ screen_context_config->screen_transmit_task_config.transmit_task_core_id
	) == pdPASS, ESP_ERR_INVALID_STATE, error, TAG, "Failed to create transmission task.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Return the created screen context.
	*screen_context_out	= screen_context;

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Screen context has been created.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	if (screen_transmit_task_handle)						vTaskDelete										(screen_transmit_task_handle);			// Cleanup the transmission task.
	if (screen_context)										free											(screen_context);						// Cleanup the screen context struct.
	if (screen_fast_text_engine_instance)	ESP_ERROR_CHECK(esp_fast_text_engine_del_text_engine_instance	(screen_fast_text_engine_instance));	// Cleanup the text engine instance.
	if (screen_fast_lcd_panel_device)		ESP_ERROR_CHECK(esp_fast_lcd_del_lcd_panel_device				(screen_fast_lcd_panel_device));		// Cleanup the fast LCD panel device.

	return ret;
}

esp_err_t slime_screen_context_del(slime_screen_context_t* screen_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(screen_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_screen_context_t handle provided when releasing screen context.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deleting transmission task.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Delete the transmission task.
	vTaskDelete(screen_context_in->transmit_task_handle);

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing rendering backends.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Release the fast LCD panel device and fast text engine instance.
	ESP_RETURN_ON_ERROR(esp_fast_text_engine_del_text_engine_instance	(screen_context_in->fast_text_engine_instance),	TAG, "Failed to release fast text engine instance.");
	ESP_RETURN_ON_ERROR(esp_fast_lcd_del_lcd_panel_device				(screen_context_in->fast_lcd_panel_device),		TAG, "Failed to release fast LCD panel device.");

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of screen context.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Detach all fields.
	screen_context_in->lcd_context					= NULL;
	screen_context_in->fast_lcd_panel_device		= NULL;
	screen_context_in->fast_text_engine_instance	= NULL;
	screen_context_in->transmit_task_handle			= NULL;
	screen_context_in->transmit_framerate			= 0U;

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing screen context struct.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Free the screen context struct.
	free(screen_context_in);

	// Log the progress if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Screen context has been released.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	return ESP_OK;
}

static void slime_screen_transmit_task(void* parameter) {
	esp_err_t ret = ESP_OK;

	// Get all necessary properties and handles for transmitting.
	const slime_screen_context_t*		screen_context		= (slime_screen_context_t*) parameter;
	const esp_fast_lcd_panel_device_t*	screen_panel_device	= screen_context->fast_lcd_panel_device;
	const uint32_t						screen_framerate	= screen_context->transmit_framerate;

	// Calculate the wait period in FreeRTOS ticks.
	const TickType_t period = pdMS_TO_TICKS (1000U / screen_framerate);

	// Log if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Screen context will start to transmit pending frames every %" PRIu32 " tick(s).", period);
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

	// Reserve the last awake time in ticks.
	TickType_t last_awake_time = xTaskGetTickCount();

	// Transmit the data.
	while (true) {
		// Delay before transmitting next pending frames.
		TickType_t delayed = xTaskDelayUntil(&last_awake_time, period);

		// Log if screen debug logging is enabled.
		#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
			ESP_LOGD(TAG, "Screen context is transmitting a pending frame at tick %" PRIu32 ".", last_awake_time);
			if (!last_delayed) {
				ESP_LOGD(TAG, "Last pending frame took too long to transmit.");
			}
		#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING

		// Transmit the pending frame
		ESP_GOTO_ON_ERROR(esp_fast_lcd_transmit(screen_panel_device), error, TAG, "Failed to transmit pending frame");
	}

	// Error occurred, terminate the transmission task.
	error:

	// Log the error if screen debug logging is enabled.
	#ifdef CONFIG_SLIME_SCREEN_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_SCREEN_DEBUG_LOGGING
}