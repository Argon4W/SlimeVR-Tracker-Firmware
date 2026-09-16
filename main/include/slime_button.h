//
// Created by progr on 2026/9/15.
//

#ifndef SLIME_BUTTON_H
#define SLIME_BUTTON_H

#include "esp_check.h"
#include "iot_button.h"
#include "freertos/FreeRTOS.h"
#include "button_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of a button.
 */
typedef struct {
	button_config_t			button_config;		/*!< The press configuration of the button. */
	button_gpio_config_t	button_gpio_config;	/*!< The GPIO configuration of the button. */
} slime_button_config_t;

/**
 * @brief The configuration struct of the button context.
 */
typedef struct {
	slime_button_config_t	button_return_config;		/*!< The configuration of the return button */
	slime_button_config_t	button_switch_config;		/*!< The configuration of the switch button */
	slime_button_config_t	button_confirm_config;		/*!< The configuration of the confirm button */
	uint32_t				button_event_queue_size;	/*!< The size of the button event queue. */
} slime_button_context_config_t;

/**
 * @brief The enum of button types.
 */
typedef enum {
	BUTTON_NULL,	/*!< No button. */
	BUTTON_RETURN,	/*!< The return button. */
	BUTTON_SWITCH,	/*!< The switch button. */
	BUTTON_CONFIRM	/*!< The confirm button. */
} slime_button_type_t;

/**
 * @brief The button context struct.
 */
typedef struct {
	button_handle_t		button_return;				/*!< The return IOT button handle */
	button_handle_t		button_switch;				/*!< The switch IOT button handle. */
	button_handle_t		button_confirm;				/*!< The confirm IOT button handle. */
	QueueHandle_t		button_event_queue;			/*!< The button event queue. */
	void*				button_callback_contexts;	/*!< The internal button callback contexts. */
} slime_button_context_t;

/**
 * @brief					Get an event from the button event queue of the button context, or BUTTON_NULL if there is no event.
 * @param button_context	The button context to get the event.
 * @return					The button of the event or BUTTON_NULL if there is no event.
 */
slime_button_type_t slime_button_poll_event(slime_button_context_t* button_context);

/**
 * @brief						Create the button context.
 * @param button_context_out	The handle to receive the created button context.
 * @param button_context_config	The configuration of the button context.
 * @return						The status of the creation.
 */
esp_err_t slime_button_context_new(
			slime_button_context_t**		button_context_out,
	const	slime_button_context_config_t*	button_context_config
);

/**
 * @brief					Release the button context.
 * @param button_context_in	The button context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_button_context_del(slime_button_context_t* button_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_BUTTON_H
