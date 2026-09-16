#ifndef SLIME_GPIO_H
#define SLIME_GPIO_H

#include "esp_check.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the GPIO context.
 */
typedef struct {
	gpio_config_t gpio_led_config;			/*!< The GPIO configuration of the LED on the sensor board. */
	gpio_config_t gpio_backlight_config;	/*!< The GPIO configuration of the Backlight of the LCD panel. */
} slime_gpio_context_config_t;

/**
 * @brief The GPIO context struct.
 */
typedef struct {
	gpio_num_t led_gpio_num;		/*!< The GPIO Num of the LED. */
	gpio_num_t backlight_gpio_num;	/*!< The GPIO Num of the Backlight. */
} slime_gpio_context_t;

/**
 * @brief				Setting the output level of the LED GPIO.
 * @param gpio_context	The context of the LED GPIO you want to control.
 * @param gpio_level	The output level of the LED GPIO ("1" = high; "0" = low).
 * @return				The status of setting the output level.
 */
esp_err_t slime_gpio_led_set_level(
	const	slime_gpio_context_t*	gpio_context,
			uint8_t					gpio_level
);

/**
 * @brief				Setting the output level of the Backlight GPIO.
 * @param gpio_context	The context of the Backlight GPIO you want to control.
 * @param gpio_level	The output level of the Backlight GPIO ("1" = high; "0" = low).
 * @return				The status of setting the output level.
 */
esp_err_t slime_gpio_backlight_set_level(
	const	slime_gpio_context_t*	gpio_context,
			uint8_t					gpio_level
);

/**
 * @brief						Create the GPIO context.
 * @param gpio_context_out		The handle to receive the created GPIO context.
 * @param gpio_context_config	The configuration of the GPIO context.
 * @return						The status of the creation.
 */
esp_err_t slime_gpio_context_new(
			slime_gpio_context_t**			gpio_context_out,
	const	slime_gpio_context_config_t*	gpio_context_config
);

/**
 * @brief					Release the GPIO context.
 * @param gpio_context_in	The GPIO context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_gpio_context_del(slime_gpio_context_t* gpio_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_GPIO_H
