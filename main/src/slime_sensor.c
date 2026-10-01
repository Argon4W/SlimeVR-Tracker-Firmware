#include "stdlib.h"
#include "stdint.h"
#include "esp_log.h"
#include "slime_sensor.h"

/**
 * @brief The log tag of the Slime Sensor.
 */
static const char* TAG = "slime_sensor";

esp_err_t slime_sensor_register_callbacks(
			slime_sensor_context_t*				sensor_context,
	const	slime_sensor_callbacks_config_t*	sensor_callbacks,
			void*								user_context
) {
	// We cannot proceed without a context and a callback configuration.
	ESP_RETURN_ON_FALSE(sensor_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when performing registering callbacks.");
	ESP_RETURN_ON_FALSE(sensor_callbacks	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_callbacks_config_t handle provided when performing registering callbacks.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Sensor context \"%s\" is trying registering callbacks.", sensor_context->name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Register the callback using the function handle in the sensor context if it exists.
	if (sensor_context->register_callbacks) {
		// Register the callback.
		ESP_RETURN_ON_ERROR(sensor_context->register_callbacks(
			/* sensor_context	= */ sensor_context,
			/* sensor_callbacks	= */ sensor_callbacks,
			/* user_context		= */ user_context
		), TAG, "Failed to register callbacks.");
	}

	return ESP_OK;
}

esp_err_t slime_sensor_get_sample_time(
	const	slime_sensor_context_t*	sensor_context,
			float_t*				gyroscope_sample_time_ms,
			float_t*				accelerometer_sample_time_ms,
			float_t*				magnetometer_sample_time_ms
) {
	// We cannot proceed without a context and handles to receive the sample times.
	ESP_RETURN_ON_FALSE(sensor_context					!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when performing getting sample times.");
	ESP_RETURN_ON_FALSE(gyroscope_sample_time_ms		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the gyroscope sample time when performing getting sample times.");
	ESP_RETURN_ON_FALSE(accelerometer_sample_time_ms	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the accelerometer sample time when performing getting sample times.");
	ESP_RETURN_ON_FALSE(magnetometer_sample_time_ms		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the magnetometer sample time when performing getting sample times.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Sensor context \"%s\" is trying getting sample times.", sensor_context->name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the sample times using the function handle in the sensor context if it exists.
	if (sensor_context->get_sample_time) {
		// Get the sample times.
		ESP_RETURN_ON_ERROR(sensor_context->get_sample_time(
			/* sensor_context				= */ sensor_context,
			/* gyroscope_sample_time_ms		= */ gyroscope_sample_time_ms,
			/* accelerometer_sample_time_ms	= */ accelerometer_sample_time_ms,
			/* magnetometer_sample_time_ms	= */ magnetometer_sample_time_ms
		), TAG, "Failed to get sample times.");
	} else {
		// No sample times, default to 0.
		*gyroscope_sample_time_ms		= 0.0f;
		*accelerometer_sample_time_ms	= 0.0f;
		*magnetometer_sample_time_ms	= 0.0f;
	}

	return ESP_OK;
}

slime_sensor_error_t slime_sensor_poll_fifo(slime_sensor_context_t* sensor_context) {
	slime_sensor_error_t	err;
	esp_err_t				ret;

	// We cannot proceed without a context.
	ESP_GOTO_ON_FALSE(sensor_context != NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_sensor_context_t handle provided when performing polling FIFO data.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Sensor context \"%s\" is trying polling FIFO data.", sensor_context->name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Poll the FIFO data using the function handle in the sensor context if it exists.
	if (sensor_context->poll_fifo) {
		// Poll the FIFO data.
		ESP_GOTO_ON_ERROR(SLIME_ESP_ERROR(sensor_context->poll_fifo(sensor_context)), error_sensor, TAG, "Failed to register timestamp callback.");
	}

	return SLIME_SENSOR_OK();

	// Wrap the ESP error to the slime sensor error.
	error_host:		return SLIME_HOST_ERROR(ret);
	error_sensor:	return err;
}

slime_sensor_error_t slime_sensor_context_new(
			slime_sensor_context_t**		sensor_context_out,
	const	slime_sensor_context_type_t*	sensor_context_type_table,
	const	slime_gpio_context_t*			gpio_context,
	const	slime_i2c_context_t*			i2c_context
) {
	slime_sensor_error_t	err;
	esp_err_t				ret;

	// We cannot proceed without a sensor type table, a GPIO context, an I2C context, and a handle to receive the created sensor context.
	ESP_GOTO_ON_FALSE(sensor_context_type_table	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_sensor_context_type_t table handle provided when creating sensor context.");
	ESP_GOTO_ON_FALSE(sensor_context_out		!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No sensor_context_out handle provided to received the created sensor context when creating sensor context.");
	ESP_GOTO_ON_FALSE(gpio_context				!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No gpio_context handle provided when creating sensor context.");
	ESP_GOTO_ON_FALSE(i2c_context				!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_gpio_context_t handle provided when creating sensor context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating sensor context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving sensor board ID of the sensor type.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve the space for sensor board ID.
	uint8_t sensor_board_id;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Getting sensor board ID from GPIO context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the sensor board ID from the GPIO context.
	ESP_GOTO_ON_ERROR(slime_gpio_get_sensor_board_id(
		/* gpio_context		= */ gpio_context,
		/* sensor_broad_id	= */ &sensor_board_id
	), error_host, TAG, "Failed to get sensor board ID.");

	// Check the range of the sensor board ID.
	ESP_GOTO_ON_FALSE(sensor_board_id < 8U, ESP_ERR_INVALID_STATE, error_host, TAG, "Sensor board ID must be smaller than 8");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Mapping sensor board ID to sensor context type.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the sensor context type struct handle of the corresponding sensor board ID.
	const slime_sensor_context_type_t* sensor_context_type = &sensor_context_type_table[sensor_board_id];

	// Check the sensor context type.
	ESP_GOTO_ON_FALSE(sensor_context_type != NULL, ESP_ERR_NOT_FOUND, error_host, TAG, "Failed to get a sensor context type from the sensor context type table.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating sensor context of the sensor context type.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the sensor context of the sensor context type.
	ESP_GOTO_ON_ERROR(SLIME_ESP_ERROR(sensor_context_type->sensor_context_new(
		/* sensor_context_out		= */ sensor_context_out,
		/* sensor_context_name		= */ sensor_context_type->name,
		/* sensor_context_config	= */ sensor_context_type->config,
		/* i2c_context				= */ i2c_context
	)), error_sensor, TAG, "Failed to create sensor context.");

	// The has-been-created message is logged in the impl function.
	return SLIME_SENSOR_OK();

	// Wrap the ESP error to the slime sensor error.
	error_host:		return SLIME_HOST_ERROR(ret);
	error_sensor:	return err;
}

esp_err_t slime_sensor_context_del(slime_sensor_context_t* sensor_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(sensor_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when releasing sensor context.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing sensor context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the sensor context using its own delete function handle.
	// A sensor context must have a delete function, so no null-check here.
	ESP_RETURN_ON_ERROR(sensor_context_in->delete(sensor_context_in), TAG, "Failed to release sensor context");

	// The has-been-released is logged in the impl function.
	return ESP_OK;
}