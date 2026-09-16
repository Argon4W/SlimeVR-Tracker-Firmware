#ifndef SLIME_CONFIG_H
#define SLIME_CONFIG_H

#include "slime_nvs.h"
#include "slime_gpio.h"
#include "slime_i2c.h"
#include "slime_lcd.h"
#include "slime_screen.h"
#include "slime_button.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration of the NVS context.
 */
extern const slime_nvs_context_config_t nvs_context_config;

/**
 * @brief The configuration of the GPIO context.
 */
extern const slime_gpio_context_config_t gpio_context_config;

/**
 * @brief The configuration of the I2C context.
 */
extern const slime_i2c_context_config_t i2c_context_config;

/**
 * @brief The configuration of the LCD context.
 */
extern const slime_lcd_context_config_t lcd_context_config;

/**
 * @brief The configuration of the screen context.
 */
extern const slime_screen_context_config_t screen_context_config;

/**
 * @brief The configuration of the button context.
 */
extern const slime_button_context_config_t button_context_config;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_CONFIG_H
