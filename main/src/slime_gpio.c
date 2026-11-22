#include "stdlib.h"
#include "esp_log.h"
#include "slime_gpio.h"

/**
 * @brief The log tag of the Slime GPIO.
 */
static const char* TAG = "slime_gpio";

/**
 * @brief The definition of the debug logging function of the Slime GPIO.
 */
#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_GPIO_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_GPIO_LOG(format, ...)
#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

/**
 * @brief		Get the count of trailing zeros from the value safely.
 * @param value	The value to get the count of trailing zeros.
 * @return		The count of trailing zeros of the value.
 */
static inline uint32_t ctz(uint64_t value);

esp_err_t slime_gpio_led_set_level(const slime_gpio_context_t* gpio_context, const uint8_t gpio_level) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(gpio_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when setting output level of the LED GPIO.");

	// Log the operation.
	SLIME_GPIO_LOG("GPIO context is setting the LED GPIO output level to %s.", gpio_level ? "high" : "low");

	// Set the output level of the LED GPIO.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context->led_gpio_num, gpio_level), TAG, "Failed to set output level of the LED GPIO.");

	return ESP_OK;
}

esp_err_t slime_gpio_backlight_set_level(const slime_gpio_context_t* gpio_context, const uint8_t gpio_level) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(gpio_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when setting output level of the Backlight GPIO.");

	// Log the operation.
	SLIME_GPIO_LOG("GPIO context is setting the Backlight GPIO output level to %s.", gpio_level ? "high" : "low");

	// Set the output level of the Backlight GPIO.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context->backlight_gpio_num, gpio_level), TAG, "Failed to set output level of the Backlight GPIO.");

	return ESP_OK;
}

esp_err_t slime_gpio_get_sensor_board_id(const slime_gpio_context_t* gpio_context, uint8_t* sensor_board_id) {
	// We cannot proceed without a context and a handle to receive the sensor board ID.
	ESP_RETURN_ON_FALSE(gpio_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when getting sensor board ID.");
	ESP_RETURN_ON_FALSE(sensor_board_id	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No uint8_t handle provided to receive the sensor board ID when getting sensor board ID.");

	// Get the all 3 bits of the sensor board ID from the input GPIOs.
	const uint8_t sensor_board_id_bit_0 = gpio_get_level(gpio_context->sensor_id_0_gpio_num);
	const uint8_t sensor_board_id_bit_1 = gpio_get_level(gpio_context->sensor_id_1_gpio_num);
	const uint8_t sensor_board_id_bit_2 = gpio_get_level(gpio_context->sensor_id_2_gpio_num);

	// Log the sensor board ID.
	SLIME_GPIO_LOG("GPIO context has read sensor board ID: bit0=%" PRIu8 ", bit1= %" PRIu8 ", bit2=%" PRIu8 ".",
		/* PRIu8 */ sensor_board_id_bit_0,
		/* PRIu8 */ sensor_board_id_bit_1,
		/* PRIu8 */ sensor_board_id_bit_2
	);

	// Combine the 3 bits into the complete sensor board ID then return.
	*sensor_board_id =	((sensor_board_id_bit_0 & 0b1) << 0U)
	|					((sensor_board_id_bit_1 & 0b1) << 1U)
	|					((sensor_board_id_bit_2 & 0b1) << 2U);

	return ESP_OK;
}

esp_err_t slime_gpio_context_new(slime_gpio_context_t** gpio_context_out, const slime_gpio_context_config_t* gpio_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created gpio context.
	ESP_RETURN_ON_FALSE(gpio_context_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when creating GPIO context.");
	ESP_RETURN_ON_FALSE(gpio_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_config_t handle provided when creating GPIO context.");

	// Log the progress.
	SLIME_GPIO_LOG("Creating GPIO context.");
	SLIME_GPIO_LOG("Extracting GPIO nums from context configuration.");

	// Extract the GPIO nums from the configuration.
	const gpio_num_t gpio_led_num			= (gpio_num_t) ctz(gpio_context_config->led_gpio_config			.pin_bit_mask);
	const gpio_num_t gpio_backlight_num		= (gpio_num_t) ctz(gpio_context_config->backlight_gpio_config	.pin_bit_mask);
	const gpio_num_t gpio_sensor_id_0_num	= (gpio_num_t) ctz(gpio_context_config->sensor_id_0_gpio_config	.pin_bit_mask);
	const gpio_num_t gpio_sensor_id_1_num	= (gpio_num_t) ctz(gpio_context_config->sensor_id_1_gpio_config	.pin_bit_mask);
	const gpio_num_t gpio_sensor_id_2_num	= (gpio_num_t) ctz(gpio_context_config->sensor_id_2_gpio_config	.pin_bit_mask);

	// Log the progress.
	SLIME_GPIO_LOG("Reserving the handle of GPIO context.");

	// Reserve the handles of the gpio context.
	slime_gpio_context_t* gpio_context = NULL;

	// Log the progress
	SLIME_GPIO_LOG("Resetting functions of the GPIOs.");

	// Reset the functions of the GPIOs.
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_led_num),			TAG, "Failed to reset the function of the LED GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_backlight_num),		TAG, "Failed to reset the function of the Backlight GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_sensor_id_0_num),	TAG, "Failed to reset the function of the sensor board ID bit 0 GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_sensor_id_1_num),	TAG, "Failed to reset the function of the sensor board ID bit 1 GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_sensor_id_2_num),	TAG, "Failed to reset the function of the sensor board ID bit 2 GPIO.");

	// Log the progress.
	SLIME_GPIO_LOG("Configuring functions of the GPIOs.");

	// Configure the function of the GPIOs.
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->led_gpio_config),			error, TAG, "Failed to configure GPIO for LED.");
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->backlight_gpio_config),		error, TAG, "Failed to configure GPIO for Backlight.");
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->sensor_id_0_gpio_config),	error, TAG, "Failed to configure GPIO for Bit 0 of the sensor board ID.");
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->sensor_id_1_gpio_config),	error, TAG, "Failed to configure GPIO for Bit 1 of the sensor board ID.");
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->sensor_id_2_gpio_config),	error, TAG, "Failed to configure GPIO for Bit 2 of the sensor board ID.");

	// Log the progress.
	SLIME_GPIO_LOG("Resetting output levels of the GPIOs.");

	// Set the output level of the output GPIOs to low.
	ESP_GOTO_ON_ERROR(gpio_set_level(gpio_led_num,			0U), error, TAG, "Failed to set the output level of the LED GPIO to low.");
	ESP_GOTO_ON_ERROR(gpio_set_level(gpio_backlight_num,	0U), error, TAG, "Failed to set the output level of the Backlight GPIO to low.");

	// Log the progress.
	SLIME_GPIO_LOG("Creating the GPIO context struct.");

	// Create the GPIO context handle.
	gpio_context = calloc(1U, sizeof(*gpio_context));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(gpio_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the GPIO context struct.");

	// Log the progress.
	SLIME_GPIO_LOG("Finalizing the GPIO context.");

	// Fill the GPIO context.
	gpio_context->led_gpio_num			= gpio_led_num;
	gpio_context->backlight_gpio_num	= gpio_backlight_num;
	gpio_context->sensor_id_0_gpio_num	= gpio_sensor_id_0_num;
	gpio_context->sensor_id_1_gpio_num	= gpio_sensor_id_1_num;
	gpio_context->sensor_id_2_gpio_num	= gpio_sensor_id_2_num;

	// Return the created GPIO context handle.
	*gpio_context_out = gpio_context;

	// Log the progress.
	SLIME_GPIO_LOG("GPIO context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_GPIO_LOG("Error occurred while creating GPIO context %s", esp_err_to_name(ret));
	SLIME_GPIO_LOG("Cleaning up resources.");

	// Cleanup the GPIO context struct.
	if (gpio_context) free(gpio_context);

	return ret;
}

esp_err_t slime_gpio_context_del(slime_gpio_context_t* gpio_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(gpio_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when releasing GPIO context.");

	// Log the progress.
	SLIME_GPIO_LOG("Releasing GPIO context.");
	SLIME_GPIO_LOG("Resetting output level of the GPIOs.");

	// Reset the output level of the GPIOs to low.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context_in->led_gpio_num,		0U), TAG, "Failed to set the output level of the LED GPIO to low.");
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context_in->backlight_gpio_num,	0U), TAG, "Failed to set the output level of the Backlight GPIO to low.");

	// Log the progress.
	SLIME_GPIO_LOG("Resetting functions of the GPIOs.");

	// Reset the functions of the GPIOs.
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->led_gpio_num),			TAG, "Failed to reset the function of the LED GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->backlight_gpio_num),	TAG, "Failed to reset the function of the Backlight GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->sensor_id_0_gpio_num),	TAG, "Failed to reset the function of the sensor board ID bit 0 GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->sensor_id_1_gpio_num),	TAG, "Failed to reset the function of the sensor board ID bit 1 GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->sensor_id_2_gpio_num),	TAG, "Failed to reset the function of the sensor board ID bit 2 GPIO.");

	// Log the progress.
	SLIME_GPIO_LOG("Detaching all fields of the GPIO context.");

	// Detaching all GPIOs.
	gpio_context_in->backlight_gpio_num		= GPIO_NUM_NC;
	gpio_context_in->led_gpio_num			= GPIO_NUM_NC;
	gpio_context_in->sensor_id_0_gpio_num	= GPIO_NUM_NC;
	gpio_context_in->sensor_id_1_gpio_num	= GPIO_NUM_NC;
	gpio_context_in->sensor_id_2_gpio_num	= GPIO_NUM_NC;

	// Log the progress.
	SLIME_GPIO_LOG("Releasing the GPIO context struct.");

	// Free the GPIO context struct.
	free(gpio_context_in);

	// Log the progress.
	SLIME_GPIO_LOG("GPIO context has been released.");

	return ESP_OK;
}

static inline uint32_t ctz(const uint64_t value) {
	return value == 0U ? 64U : __builtin_ctzll(value);
}