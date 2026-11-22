#include "stdlib.h"
#include "esp_log.h"
#include "slime_button.h"

/**
 * @brief The log tag of the Slime Button.
 */
static const char* TAG = "slime_button";

/**
 * @brief The definition of the debug logging function of the Slime Button.
 */
#ifdef CONFIG_SLIME_BUTTON_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_BUTTON_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_BUTTON_LOG(format, ...)
#endif // CONFIG_SLIME_BUTTON_DEBUG_LOGGING

/**
 * @brief									The callback function that is invoked when a button is short-press clicked.
 * @param button_handle						The clicked button handle.
 * @param button_callback_context_opaque	The button callback context of the callback function.
 */
void slime_button_on_single_clicked(
	void *button_handle,
	void *button_callback_context_opaque
) {
	esp_err_t ret = ESP_OK;

	// Cast the opaque type pointer back to callback context pointer.
	const slime_button_callback_context_t*	callback_context	= (slime_button_callback_context_t*) button_callback_context_opaque;
	const slime_button_context_t*			button_context		= callback_context	->button_context;
	const slime_button_type_t				button_type			= callback_context	->button_type;
	const QueueHandle_t						button_event_queue	= button_context	->button_event_queue;

	// If the queue is full, discard the eldest event in the queue.
	if (uxQueueSpacesAvailable(button_event_queue) == 0U) {
		// Reserve the discarded event.
		slime_button_type_t discarded_event = {0};

		// Discard the eldest event in the queue.
		ESP_GOTO_ON_FALSE(xQueueReceive(
			/* xQueue		= */ button_context->button_event_queue,
			/* pvBuffer		= */ &discarded_event,
			/* xTicksToWait	= */ 0U
		) == pdTRUE, ESP_ERR_INVALID_STATE, error, TAG, "Failed to discard eldest event in the button event queue.");

		// Log the discarded event
		switch (discarded_event) {
			case BUTTON_RETURN:		SLIME_BUTTON_LOG("Button context has discarded a return button click event.");	break;
			case BUTTON_SWITCH:		SLIME_BUTTON_LOG("Button context has discarded a switch button click event.");	break;
			case BUTTON_CONFIRM:	SLIME_BUTTON_LOG("Button context has discarded a confirm button click event.");	break;
			default:				SLIME_BUTTON_LOG("Failed log a discarded event of an invalid button type.");	break;
		}
	}

	switch (button_type) {
		case BUTTON_RETURN:		SLIME_BUTTON_LOG("Button context is enqueueing a new return button click event.");	break;
		case BUTTON_SWITCH:		SLIME_BUTTON_LOG("Button context is enqueueing a new switch button click event.");	break;
		case BUTTON_CONFIRM:	SLIME_BUTTON_LOG("Button context is enqueueing a new confirm button click event.");	break;
		default:				SLIME_BUTTON_LOG("Failed log a click event of an invalid button type.");			break;
	}

	// Enqueue the new event.
	ESP_GOTO_ON_FALSE(xQueueSend(
		/* xQueue			= */ button_event_queue,
		/* pvItemToQueue	= */ &button_type,
		/* xTicksToWait		= */ 0U
	) == pdTRUE, ESP_ERR_INVALID_STATE, error, TAG, "Failed to enqueue event to the button event queue.");

	return;

	// Error occurred.
	error:

	// Log the error.
	SLIME_BUTTON_LOG("Error occurred while handling button event: %s", esp_err_to_name(ret));
	SLIME_BUTTON_LOG("Terminating current event handling.");
}

slime_button_type_t slime_button_poll_event(slime_button_context_t* button_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(button_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_button_context_t handle provided when performing polling button events.");

	// Reserve the event received from the queue.
	slime_button_type_t event;

	// Log the operation.
	SLIME_BUTTON_LOG("Button context is polling events from the button event queue.");

	// Try receiving an event from the queue immediately.
	const BaseType_t result = xQueueReceive(
		/* xQueue		= */ button_context->button_event_queue,
		/* pvBuffer		= */ &event,
		/* xTicksToWait	= */ 0U
	);

	// Return the received event if an event was successfully received from the queue.
	if (result == pdTRUE) {
		// Log the discarded event.
		switch (event) {
			case BUTTON_RETURN:		SLIME_BUTTON_LOG("Button context has polled a return button click event from the queue.");	break;
			case BUTTON_SWITCH:		SLIME_BUTTON_LOG("Button context has polled a switch button click event from the queu.");	break;
			case BUTTON_CONFIRM:	SLIME_BUTTON_LOG("Button context has polled a confirm button click event from the queue.");	break;
			default:				SLIME_BUTTON_LOG("Failed log a polled event of an invalid button type.");					break;
		}
		return event;
	}

	// Log the invalid event.
	SLIME_BUTTON_LOG("No event found in the button event queue of the button context.");

	// Otherwise, return BUTTON_NULL indicating no event.
	return BUTTON_NULL;
}

esp_err_t slime_button_context_new(slime_button_context_t** button_context_out, const slime_button_context_config_t* button_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created I2C context.
	ESP_RETURN_ON_FALSE(button_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_button_context_t handle provided when creating button context.");
	ESP_RETURN_ON_FALSE(button_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_button_context_config_t handle provided when creating button context.");

	// Log the progress
	SLIME_BUTTON_LOG("Creating button context.");
	SLIME_BUTTON_LOG("Reserving handles of button context.");

	// Reserve the handles of button context.
	slime_button_context_t*				button_context		= NULL;
	button_handle_t						button_return		= NULL;
	button_handle_t						button_switch		= NULL;
	button_handle_t						button_confirm		= NULL;
	QueueHandle_t						button_event_queue	= NULL;
	slime_button_callback_context_t*	callback_contexts	= NULL;

	// Log the progress.
	SLIME_BUTTON_LOG("Caching all button configurations to stack.");

	// Extract the configuration of all buttons from the context configuration.
	const slime_button_config_t* return_button_config	= &button_context_config->return_button_config;
	const slime_button_config_t* switch_button_config	= &button_context_config->switch_button_config;
	const slime_button_config_t* confirm_button_config	= &button_context_config->confirm_button_config;

	// Create the button devices.
	ESP_GOTO_ON_ERROR(iot_button_new_gpio_device(&return_button_config	->button_config, &return_button_config	->button_gpio_config, &button_return),	error, TAG, "Failed to create return button device.");
	ESP_GOTO_ON_ERROR(iot_button_new_gpio_device(&switch_button_config	->button_config, &switch_button_config	->button_gpio_config, &button_switch),	error, TAG, "Failed to create switch button device.");
	ESP_GOTO_ON_ERROR(iot_button_new_gpio_device(&confirm_button_config	->button_config, &confirm_button_config	->button_gpio_config, &button_confirm),	error, TAG, "Failed to create confirm button device.");

	// Log the progress.
	SLIME_BUTTON_LOG("Creating button event queue.");

	// Create the button event queue.
	button_event_queue = xQueueCreate(button_context_config->button_event_queue_size, sizeof(slime_button_type_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(button_event_queue != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create button event queue.");

	// Log the progress.
	SLIME_BUTTON_LOG("Creating button context struct and button callback contexts.");

	// Create the button context struct and callback contexts
	button_context		= calloc(1U, sizeof(slime_button_context_t));
	callback_contexts	= calloc(3U, sizeof(slime_button_callback_context_t));

	// Check the allocations.
	ESP_GOTO_ON_FALSE(button_context	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create button context struct.");
	ESP_GOTO_ON_FALSE(callback_contexts	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create button callback contexts.");

	// Log the progress.
	SLIME_BUTTON_LOG("Filling button context.");

	// Fill the button context.
	button_context->return_button_handle	= button_return;
	button_context->switch_button_handle	= button_switch;
	button_context->confirm_button_handle	= button_confirm;
	button_context->button_event_queue		= button_event_queue;
	button_context->callback_contexts		= callback_contexts;

	// Log the progress.
	SLIME_BUTTON_LOG("Filling button callback contexts.");

	// Fill the button context of the callback contexts.
	callback_contexts[0].button_context = button_context;
	callback_contexts[1].button_context = button_context;
	callback_contexts[2].button_context = button_context;

	// Fill the button type of the callback contexts.
	callback_contexts[0].button_type = BUTTON_RETURN;
	callback_contexts[1].button_type = BUTTON_SWITCH;
	callback_contexts[2].button_type = BUTTON_CONFIRM;

	// Log the progress.
	SLIME_BUTTON_LOG("Registering button event callbacks.");

	// Register the callback of the buttons.
	ESP_GOTO_ON_ERROR(iot_button_register_cb(button_return,		BUTTON_SINGLE_CLICK, NULL, slime_button_on_single_clicked, &callback_contexts[0]), error, TAG, "Failed to register event callback for return button.");
	ESP_GOTO_ON_ERROR(iot_button_register_cb(button_switch,		BUTTON_SINGLE_CLICK, NULL, slime_button_on_single_clicked, &callback_contexts[1]), error, TAG, "Failed to register event callback for switch button.");
	ESP_GOTO_ON_ERROR(iot_button_register_cb(button_confirm,	BUTTON_SINGLE_CLICK, NULL, slime_button_on_single_clicked, &callback_contexts[2]), error, TAG, "Failed to register event callback for confirm button.");

	// Log the progress.
	SLIME_BUTTON_LOG("Finalizing button context.");

	// Return the created button context.
	*button_context_out = button_context;

	// Log the progress.
	SLIME_BUTTON_LOG("Button context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_BUTTON_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_BUTTON_LOG("Cleaning up resources.");

	if (callback_contexts)					free				(callback_contexts);	// Cleanup the callback contexts.
	if (button_context)						free				(button_context);		// Cleanup the button context struct.
	if (button_event_queue)					vQueueDelete		(button_event_queue);	// Cleanup the button event queue.
	if (button_confirm)		ESP_ERROR_CHECK(iot_button_delete	(button_confirm));		// Cleanup the confirm button handle.
	if (button_switch)		ESP_ERROR_CHECK(iot_button_delete	(button_switch));		// Cleanup the switch button handle.
	if (button_return)		ESP_ERROR_CHECK(iot_button_delete	(button_return));		// Cleanup the return button handle.

	return ret;
}

esp_err_t slime_button_context_del(slime_button_context_t* button_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(button_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_button_context_t handle provided when releasing button context.");

	// Log the progress.
	SLIME_BUTTON_LOG("Releasing button context.");
	SLIME_BUTTON_LOG("Unregistering button event callbacks.");

	// Unregister callbacks of all buttons.
	ESP_RETURN_ON_ERROR(iot_button_unregister_cb(button_context_in->return_button_handle,	BUTTON_SINGLE_CLICK, NULL), TAG, "Failed to unregister event callback for return button.");
	ESP_RETURN_ON_ERROR(iot_button_unregister_cb(button_context_in->switch_button_handle,	BUTTON_SINGLE_CLICK, NULL), TAG, "Failed to unregister event callback for switch button.");
	ESP_RETURN_ON_ERROR(iot_button_unregister_cb(button_context_in->confirm_button_handle,	BUTTON_SINGLE_CLICK, NULL), TAG, "Failed to unregister event callback for confirm button.");

	// Log the progress.
	SLIME_BUTTON_LOG("Deleting button devices.");

	// Delete all button device handless.
	ESP_RETURN_ON_ERROR(iot_button_delete(button_context_in->return_button_handle),		TAG, "Failed to delete return button handle.");
	ESP_RETURN_ON_ERROR(iot_button_delete(button_context_in->switch_button_handle),		TAG, "Failed to delete switch button handle.");
	ESP_RETURN_ON_ERROR(iot_button_delete(button_context_in->confirm_button_handle),	TAG, "Failed to delete confirm button handle.");

	// Log the progress.
	SLIME_BUTTON_LOG("Deleting button event queue.");

	// Delete the button event queue.
	vQueueDelete(button_context_in->button_event_queue);

	// Log the progress.
	SLIME_BUTTON_LOG("Detaching all fields of callback contexts.");

	// Fetch the callback contexts from the button context.
	slime_button_callback_context_t* callback_contexts = button_context_in->callback_contexts;

	// Detach the button context field in all callback contexts.
	callback_contexts[0].button_context = NULL;
	callback_contexts[1].button_context = NULL;
	callback_contexts[2].button_context = NULL;

	// Detach the button type field in all callback contexts.
	callback_contexts[0].button_type = BUTTON_NULL;
	callback_contexts[1].button_type = BUTTON_NULL;
	callback_contexts[2].button_type = BUTTON_NULL;

	// Log the progress.
	SLIME_BUTTON_LOG("Releasing callback contexts.");

	// Free the button event callback contexts.
	free(callback_contexts);

	// Log the progress.
	SLIME_BUTTON_LOG("Detaching all fields of button context.");

	// Detaching all fields.
	button_context_in->return_button_handle		= NULL;
	button_context_in->switch_button_handle		= NULL;
	button_context_in->confirm_button_handle	= NULL;
	button_context_in->button_event_queue		= NULL;
	button_context_in->callback_contexts		= NULL;

	// Log the progress.
	SLIME_BUTTON_LOG("Releasing button context struct.");

	// Free the button context struct.
	free(button_context_in);

	// Log the progress.
	SLIME_BUTTON_LOG("Deleting button context handles.");

	return ESP_OK;
}