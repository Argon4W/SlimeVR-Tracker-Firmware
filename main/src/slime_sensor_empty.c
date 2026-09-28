#include "stdlib.h"
#include "esp_log.h"
#include "slime_sensor_empty.h"

/**
 * @brief The log tag of the Slime Empty Sensor.
 */
static const char* TAG = "slime_sensor_empty";

/**
 * @brief					Release the empty sensor context.
 * @param sensor_context_in The empty sensor context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_empty_sensor_context_del(slime_sensor_context_t* sensor_context_in);

slime_sensor_error_t slime_empty_sensor_context_new(
			slime_sensor_context_t**	sensor_context_out,
	const	char*						sensor_context_name,
	const	void*						sensor_context_config,
	const	slime_i2c_context_t*		i2c_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a name, a handle to receive the created empty sensor context.
	ESP_GOTO_ON_FALSE(sensor_context_out	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No sensor_context_out handle provided when creating empty sensor context.");
	ESP_GOTO_ON_FALSE(sensor_context_name	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No name provided when creating empty sensor context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating empty sensor context \"%s\".", sensor_context_name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the sensor context struct.
	slime_sensor_context_t* sensor_context = calloc(1U, sizeof(slime_sensor_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(sensor_context != NULL, ESP_ERR_NO_MEM, error_host, TAG, "Failed to create sensor context struct for empty sensor context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing empty sensor context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the empty sensor context.
	sensor_context->name				= sensor_context_name;
	sensor_context->register_callbacks	= NULL;
	sensor_context->poll_fifo			= NULL;
	sensor_context->delete				= slime_empty_sensor_context_del;

	// Return the created empty sensor context.
	*sensor_context_out = sensor_context;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Empty sensor context \"%s\" has been created.", sensor_context_name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return SLIME_SENSOR_OK();

	// Wrap the ESP error to the slime sensor error.
	error_host: return SLIME_HOST_ERROR(ret);
}

esp_err_t slime_empty_sensor_context_del(slime_sensor_context_t* sensor_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(sensor_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when releasing empty sensor context.");

	// Reserve the name of the sensor context.
	const char* name = sensor_context_in->name;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing empty sensor context \"%s\".", name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the empty sensor context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Detach all fields.
	sensor_context_in->name					= NULL;
	sensor_context_in->register_callbacks	= NULL;
	sensor_context_in->poll_fifo			= NULL;
	sensor_context_in->delete				= NULL;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing empty sensor context struct.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the empty sensor context struct.
	free(sensor_context_in);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Empty sensor context \"%s\" has been released.", name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}