#include "stdlib.h"
#include "esp_log.h"
#include "slime_gpio.h"

/**
 * @brief The log tag of the Slime GPIO.
 */
static const char* TAG = "slime_gpio";

/**
 * @brief		Get the count of trailing zeros from the value safely.
 * @param value	The value to get the count of trailing zeros.
 * @return		The count of trailing zeros of the value.
 */
static inline uint32_t ctz(uint64_t value);

esp_err_t slime_gpio_led_set_level(
	const slime_gpio_context_t*	gpio_context,
	const uint8_t				gpio_level
) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(gpio_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when setting output level of the LED GPIO.");

	// Set the output level of the LED GPIO.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context->led_gpio_num, gpio_level), TAG, "Failed to set output level of the LED GPIO.");

	return ESP_OK;
}

esp_err_t slime_gpio_backlight_set_level(
	const slime_gpio_context_t*	gpio_context,
	const uint8_t				gpio_level
) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(gpio_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when setting output level of the Backlight GPIO.");

	// Set the output level of the Backlight GPIO.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context->backlight_gpio_num, gpio_level), TAG, "Failed to set output level of the Backlight GPIO.");

	return ESP_OK;
}

esp_err_t slime_gpio_context_new(
			slime_gpio_context_t**			gpio_context_out,
	const	slime_gpio_context_config_t*	gpio_context_config
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created gpio context.
	ESP_RETURN_ON_FALSE(gpio_context_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided to receive the result when creating GPIO context.");
	ESP_RETURN_ON_FALSE(gpio_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_config_t handle provided to receive the result when creating GPIO context.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating GPIO context.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Extracting GPIO context configuration.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Extract the GPIO nums from the configuration.
	const gpio_num_t gpio_led_num		= (gpio_num_t) ctz(gpio_context_config->gpio_led_config			.pin_bit_mask);
	const gpio_num_t gpio_backlight_num	= (gpio_num_t) ctz(gpio_context_config->gpio_backlight_config	.pin_bit_mask);

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving the handle of GPIO context.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Reserve the handles of the gpio context.
	slime_gpio_context_t* gpio_context = NULL;

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting functions of the GPIOs.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Reset the functions of the GPIOs.
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_led_num),		TAG, "Failed to reset the function of the LED GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_backlight_num),	TAG, "Failed to reset the function of the Backlight GPIO.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Configuring functions of the GPIOs.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Configure the function of the GPIOs.
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->gpio_led_config),		error, TAG, "Failed to configure GPIO for LED.");
	ESP_GOTO_ON_ERROR(gpio_config(&gpio_context_config->gpio_backlight_config),	error, TAG, "Failed to configure GPIO for Backlight.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting output levels of the GPIOs.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Set the output level of the GPIOs to low.
	ESP_GOTO_ON_ERROR(gpio_set_level(gpio_led_num,			0U), error, TAG, "Failed to set the output level of the LED GPIO to low.");
	ESP_GOTO_ON_ERROR(gpio_set_level(gpio_backlight_num,	0U), error, TAG, "Failed to set the output level of the Backlight GPIO to low.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Allocating the GPIO context struct.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Allocate the GPIO context handle.
	gpio_context = calloc(1, sizeof(*gpio_context));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(gpio_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the GPIO context struct.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing the GPIO context.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Fill the GPIO context.
	gpio_context->led_gpio_num			= gpio_led_num;
	gpio_context->backlight_gpio_num	= gpio_backlight_num;

	// Return the created GPIO context handle.
	*gpio_context_out = gpio_context;

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "GPIO context has been created.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred while creating GPIO context %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Cleanup the GPIO context struct.
	if (gpio_context) free(gpio_context);

	return ret;
}

esp_err_t slime_gpio_context_del(slime_gpio_context_t* gpio_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(gpio_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_gpio_context_t handle provided when releasing GPIO context.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing GPIO context.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting output level of the GPIOs.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Reset the output level of the GPIOs to low.
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context_in->led_gpio_num,		0U), TAG, "Failed to set the output level of the LED GPIO to low.");
	ESP_RETURN_ON_ERROR(gpio_set_level(gpio_context_in->backlight_gpio_num,	0U), TAG, "Failed to set the output level of the Backlight GPIO to low.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting functions of the GPIOs.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Reset the functions of the GPIOs.
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->led_gpio_num),			TAG, "Failed to reset the function of the LED GPIO.");
	ESP_RETURN_ON_ERROR(gpio_reset_pin(gpio_context_in->backlight_gpio_num),	TAG, "Failed to reset the function of the Backlight GPIO.");

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the GPIO context.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Detaching all GPIOs.
	gpio_context_in->backlight_gpio_num	= GPIO_NUM_NC;
	gpio_context_in->led_gpio_num		= GPIO_NUM_NC;

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing the GPIO context struct.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	// Free the GPIO context struct.
	free(gpio_context_in);

	// Log the progress if GPIO debug logging is enabled.
	#ifdef CONFIG_SLIME_GPIO_DEBUG_LOGGING
		ESP_LOGD(TAG, "GPIO context has been released.");
	#endif // CONFIG_SLIME_GPIO_DEBUG_LOGGING

	return ESP_OK;
}

static inline uint32_t ctz(uint64_t value) {
	return value == 0U ? 64U : __builtin_ctzll(value);
}