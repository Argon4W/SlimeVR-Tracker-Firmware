#include "string.h"
#include "tgmath.h"
#include "slime_wifi.h"

/**
 * @brief The event group bit indicating that there was an error occurred.
 */
#define SLIME_WIFI_ERROR (1U << 0U)

/**
 * @brief The event group bit indicating that Wi-Fi station is started.
 */
#define SLIME_WIFI_STARTED (1U << 1U)

/**
 * @brief The event group bit indicating that Wi-Fi station is connected to AP.
 */
#define SLIME_WIFI_CONNECTED (1U << 2U)

/**
 * @brief The event group bit indicating that Wi-Fi station failed to connect to AP.
 */
#define SLIME_WIFI_FAILED (1U << 3U)

/**
 * @brief The Wi-Fi station disconnected event type.
 */
typedef wifi_event_sta_disconnected_t slime_wifi_disconn_t;

/**
 * @brief The Wi-Fi station got IPv4 address event type.
 */
typedef ip_event_got_ip_t slime_wifi_got_ip4_t;

/**
 * @brief The log tag of the Slime Wi-Fi.
 */
static const char* TAG = "slime_wifi";

/**
 * @brief The state of the Wi-Fi context. Only 1 context is allowed at runtime due to global Wi-Fi driver.
 */
static uint8_t wifi_context_created = false;

/**
 * @brief				The event handler when station started.
 * @param event_arg		The Wi-Fi context that receives the event.
 * @param event_base	The unique pointer to a subsystem that expose the event.
 * @param event_id		The id of the event.
 * @param event_data	The data of the event.
 */
static void slime_wifi_on_sta_start(
	void*				event_arg,
	esp_event_base_t	event_base,
	int32_t				event_id,
	void*				event_data
) {
	esp_err_t ret = ESP_OK;

	// Log the event if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context started, connecting.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Cast the argument to Wi-Fi context.
	const slime_wifi_context_t* wifi_context = (slime_wifi_context_t*) event_arg;

	// Connect the Wi-Fi station to the AP of credential in the Wi-Fi configuration.
	ESP_GOTO_ON_ERROR(esp_wifi_connect(), error, TAG, "Failed to connect Wi-Fi station to AP.");

	// Set the Wi-Fi context started and disconnected.
	xEventGroupSetBits(wifi_context->event_group, SLIME_WIFI_STARTED);

	return;

	// Error occurred.
	error:

	// Log the error if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred while connecting: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Marking the Wi-Fi context errored.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Set the Wi-Fi context errored.
	xEventGroupSetBits(wifi_context->event_group, SLIME_WIFI_ERROR);
}

/**
 * @brief				The event handler when station disconnected.
 * @param event_arg		The Wi-Fi context that receives the event.
 * @param event_base	The unique pointer to a subsystem that expose the event.
 * @param event_id		The id of the event.
 * @param event_data	The data of the event.
 */
static void slime_wifi_on_sta_disconnected(
	void*				event_arg,
	esp_event_base_t	event_base,
	int32_t				event_id,
	void*				event_data
) {
	esp_err_t ret = ESP_OK;

	// Cast the opaque data to handle, cast the argument to Wi-Fi context.
	const	slime_wifi_disconn_t* wifi_disconn = (slime_wifi_disconn_t*) event_data;
			slime_wifi_context_t* wifi_context = (slime_wifi_context_t*) event_arg;

	// Log the disconnection if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context has disconnected from AP of SSID \"%.32s\"(%" PRIu8 ") because of reason 0x%02" PRIX8 ". RSSI when disconnected: %" PRId8,
			/* s		*/ wifi_disconn->ssid,
			/* PRIu8	*/ wifi_disconn->ssid_len,
			/* PRIX8	*/ wifi_disconn->reason,
			/* PRId8	*/ wifi_disconn->rssi
		);
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Set the Wi-Fi context disconnected from AP.
	xEventGroupClearBits(wifi_context->event_group, SLIME_WIFI_CONNECTED);

	// Acquire the lock and take the ownership of the credential in the Wi-Fi context.
	xSemaphoreTake(wifi_context->credential_lock, portMAX_DELAY);

	// Abort reconnecting if the retry count reaches the maximum retry count.
	if (wifi_context->credential_retry_count >= wifi_context->credential_retry_count_max) {
		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Wi-Fi context has tried reconnecting to the same AP for %" PRIu8 "/%" PRIu8 " times. Abort reconnecting until there is new credential.",
				/* PRIu8 */ wifi_context->credential_retry_count,
				/* PRIu8 */ wifi_context->credential_retry_count_max
			);
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Set the Wi-Fi context failed to connect to AP.
		xEventGroupSetBits(wifi_context->event_group, SLIME_WIFI_FAILED);

		// Release the lock, exit critical.
		xSemaphoreGive(wifi_context->credential_lock);

		// Abort the reconnecting.
		return;
	}

	// Increase the retry count (manually disconnecting excluded).
	if (wifi_disconn->reason != WIFI_REASON_ASSOC_LEAVE) {
		wifi_context->credential_retry_count ++;
	}

	// Log the event if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "reconnecting. (%" PRIu8 "/%" PRIu8 ")",
			/* PRIu8 */ wifi_context->credential_retry_count,
			/* PRIu8 */ wifi_context->credential_retry_count_max
		);
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Reserve space for the credential.
	char credential_ssid[32U];
	char credential_pass[64U];

	// Load the complete credential from the Wi-Fi context.
	strncpy(credential_ssid, wifi_context->credential_ssid, 32U);
	strncpy(credential_pass, wifi_context->credential_pass, 64U);

	// Reserve space for Wi-Fi station configuration.
	wifi_config_t wifi_config;

	// Get the Wi-Fi station configuration from the global Wi-Fi driver.
	ESP_GOTO_ON_ERROR(esp_wifi_get_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to get Wi-Fi station configuration.");

	// Compare the new credential with the old credential.
	// Skip the configuration if the Wi-Fi context is connected or connecting to AP.
	if (	strncmp(credential_ssid, (char*) wifi_config.sta.ssid,		32U) == 0U
		&&	strncmp(credential_pass, (char*) wifi_config.sta.password,	64U) == 0U
	) {
		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Wi-Fi context is trying reconnecting to the same AP with credential of SSID \"%.32s\"(%zu) and password length %zu.",
				/* s	*/ credential_ssid,
				/* zu	*/ strnlen(credential_ssid, 32U),
				/* zu	*/ strnlen(credential_pass, 64U)
			);
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING
	} else {
		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Wi-Fi context is trying connecting to a new AP with credential of SSID \"%.32s\"(%zu) and password length %zu.",
				/* s	*/ credential_ssid,
				/* zu	*/ strnlen(credential_ssid, 32U),
				/* zu	*/ strnlen(credential_pass, 64U)
			);
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Replace the SSID and password to the Wi-Fi station configuration.
		strncpy((char*) wifi_config.sta.ssid,		credential_ssid, 32U);
		strncpy((char*) wifi_config.sta.password,	credential_pass, 64U);

		// Set the Wi-Fi station configuration to the global Wi-Fi driver
		ESP_GOTO_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to set Wi-Fi station configuration.");
	}

	// Connect the Wi-Fi station to the AP in the station configuration.
	ESP_GOTO_ON_ERROR(esp_wifi_connect(), error, TAG, "Failed to connect Wi-Fi station to AP.");

	// Release the lock, exit critical.
	// We release the lock at the end of the reconnecting to ensure that WHENEVER slime_wifi_connect acquires the lock,
	// either the reconnecting has completed or the reconnecting has not yet begun.
	xSemaphoreGive(wifi_context->credential_lock);

	return;

	// Error occurred.
	error:

	// Log the error if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred while reconnecting: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Releasing the credential lock then marking the Wi-Fi context errored.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Release the lock.
	xSemaphoreGive(wifi_context->credential_lock);

	// Set the Wi-Fi context errored.
	xEventGroupSetBits(wifi_context->event_group, SLIME_WIFI_ERROR);
}

/**
 * @brief				The event handler when station got IP from connected AP.
 * @param event_arg		The Wi-Fi context that receives the event.
 * @param event_base	The unique pointer to a subsystem that expose the event.
 * @param event_id		The id of the event.
 * @param event_data	The data of the event.
 */
static void slime_wifi_on_sta_got_ip(
	void*				event_arg,
	esp_event_base_t	event_base,
	int32_t				event_id,
	void*				event_data
) {
	// Cast the argument to Wi-Fi context.
	slime_wifi_context_t* wifi_context = (slime_wifi_context_t*) event_arg;

	// Log the IP info if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		// Cast the data to event data struct.
		const slime_wifi_got_ip4_t* wifi_got_ip4 = (slime_wifi_got_ip4_t*) event_data;

		// Log the IP info.
		ESP_LOGD(TAG, "Wi-Fi context has got IPv4 address: ");
		ESP_LOGD(TAG, "Interface IPv4 address: "			IPSTR, IP2STR(&wifi_got_ip4->ip_info.ip));
		ESP_LOGD(TAG, "Interface IPv4 netmask: "			IPSTR, IP2STR(&wifi_got_ip4->ip_info.netmask));
		ESP_LOGD(TAG, "Interface IPv4 gateway address: "	IPSTR, IP2STR(&wifi_got_ip4->ip_info.gw));
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Set the Wi-Fi context connected.
	xEventGroupSetBits(wifi_context->event_group, SLIME_WIFI_CONNECTED);

	// Acquire the lock and take the ownership of the retry count of the credential in the Wi-Fi context.
	xSemaphoreTake(wifi_context->credential_lock, portMAX_DELAY);

	// Reset the retry count because the Wi-Fi station has successfully connected to AP.
	wifi_context->credential_retry_count = 0U;

	// Release the lock.
	xSemaphoreGive(wifi_context->credential_lock);
}

/**
 * @brief				The event handler when station lost IP from connected AP.
 * @param event_arg		The Wi-Fi context that receives the event.
 * @param event_base	The unique pointer to a subsystem that expose the event.
 * @param event_id		The id of the event.
 * @param event_data	The data of the event.
 */
static void slime_wifi_on_sta_lost_ip(
	void*				event_arg,
	esp_event_base_t	event_base,
	int32_t				event_id,
	void*				event_data
) {
	// Log the event if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context lost IP address.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Cast the argument to Wi-Fi context,
	// then set the Wi-Fi context disconnected.
	xEventGroupClearBits(((slime_wifi_context_t*) event_arg)->event_group, SLIME_WIFI_CONNECTED);
}

esp_err_t slime_wifi_connect(
			slime_wifi_context_t*	wifi_context,
	const	char*					credential_ssid,
	const	char*					credential_pass
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(wifi_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when dynamically connecting to new AP.");
	ESP_RETURN_ON_FALSE(credential_ssid	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No SSID provided when dynamically connecting to new AP.");
	ESP_RETURN_ON_FALSE(credential_pass	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No password provided when dynamically connecting to new AP.");

	// Wait until the Wi-Fi station is started.
	ESP_RETURN_ON_FALSE((xEventGroupWaitBits(
		/* xEventGroup		= */ wifi_context->event_group,
		/* uxBitsToWaitFor	= */ SLIME_WIFI_STARTED | SLIME_WIFI_ERROR,
		/* xClearOnExit		= */ pdFALSE,
		/* xWaitForAllBits	= */ pdFALSE,
		/* xTicksToWait		= */ portMAX_DELAY
	) & SLIME_WIFI_ERROR) == 0U, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	// Calculate the length of the SSID and password with '\0' included.
	const size_t credential_ssid_length = strlen(credential_ssid);
	const size_t credential_pass_length = strlen(credential_pass);

	// Log the operation if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context is trying setting new credential of SSID \"%.32s\"(%zu) and password length %zu to connect.",
			/* s	*/ credential_ssid,
			/* zu	*/ strnlen(credential_ssid, 32U),
			/* zu	*/ strnlen(credential_pass, 64U)
		);
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Check the length of the credential.
	ESP_RETURN_ON_FALSE(credential_ssid_length <= 32U, ESP_ERR_INVALID_SIZE, TAG, "The SSID is too long.");
	ESP_RETURN_ON_FALSE(credential_pass_length <= 64U, ESP_ERR_INVALID_SIZE, TAG, "The password is too long.");

	// Acquire the lock and take the ownership of the credential in the Wi-Fi context.
	xSemaphoreTake(wifi_context->credential_lock, portMAX_DELAY);

	// Clear the retry count.
	wifi_context->credential_retry_count = 0U;

	// Check if the Wi-Fi context has aborted the reconnecting previously.
	if ((xEventGroupGetBits(wifi_context->event_group) & SLIME_WIFI_FAILED) != 0) {
		// Clear the failed bit.
		xEventGroupClearBits(wifi_context->event_group, SLIME_WIFI_FAILED);

		// Reserve space for Wi-Fi station configuration.
		wifi_config_t wifi_config;

		// Get the Wi-Fi station configuration from the global Wi-Fi driver.
		ESP_GOTO_ON_ERROR(esp_wifi_get_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to get Wi-Fi station configuration.");

		// Replace the SSID and password to the Wi-Fi station configuration.
		strncpy((char*)	wifi_config.sta.ssid,		credential_ssid, 32U);
		strncpy((char*)	wifi_config.sta.password,	credential_pass, 64U);

		// Also update the credential in the Wi-Fi context.
		strncpy(wifi_context->credential_ssid, credential_ssid, 32U);
		strncpy(wifi_context->credential_pass, credential_pass, 64U);

		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Restart reconnecting cycle to connect to new AP.");
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Set the Wi-Fi station configuration to the global Wi-Fi driver, then connect the Wi-Fi station to AP.
		// The reconnecting cycle has aborted previously, which means we cannot use disconnect to trigger reconnecting.
		ESP_GOTO_ON_ERROR(esp_wifi_set_config	(WIFI_IF_STA, &wifi_config),	error, TAG, "Failed to set Wi-Fi station configuration.");
		ESP_GOTO_ON_ERROR(esp_wifi_connect		(),								error, TAG, "Failed to connect Wi-Fi station to AP.");
	} else {
		// Update the new credential inside the lock.
		strncpy(wifi_context->credential_ssid, credential_ssid, 32U);
		strncpy(wifi_context->credential_pass, credential_pass, 64U);

		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Disconnect from current connected/connecting AP to connect to new AP.");
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Disconnect the Wi-Fi station from AP or abort the ongoing connecting process to trigger the reconnecting using the new credential.
		ESP_GOTO_ON_ERROR(esp_wifi_disconnect(), error, TAG, "Failed to disconnect Wi-Fi station from AP.");
	}

	// Release the lock, exit critical, reconnecting will be blocked until the lock is released to ensure that it can acquire
	// the latest credential.
	xSemaphoreGive(wifi_context->credential_lock);

	return ESP_OK;

	// Error occurred.
	error:

	// Log the error if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred while dynamically connecting new AP: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Releasing the credential lock.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Release the lock.
	xSemaphoreGive(wifi_context->credential_lock);

	return ret;
}

esp_err_t slime_wifi_is_connected(
	const	slime_wifi_context_t*	wifi_context,
			uint8_t*				wifi_connected_out
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(wifi_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when getting connection state.");
	ESP_RETURN_ON_FALSE(wifi_connected_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the connection state when getting connection state.");

	// Log the operation if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context is trying getting connection state of the Wi-Fi station.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Get the connected bit from the event group of the Wi-Fi context.
	const EventBits_t bits = xEventGroupGetBits(wifi_context->event_group);

	// Extract states from the bits of the event group.
	const uint8_t wifi_connected = (bits & SLIME_WIFI_CONNECTED)	!= 0U;
	const uint8_t wifi_has_error = (bits & SLIME_WIFI_ERROR)		!= 0U;

	// Check if error occurred inside the Wi-Fi context.
	ESP_RETURN_ON_FALSE(!wifi_has_error, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	// Return the got connection state.
	*wifi_connected_out = wifi_connected;

	return ESP_OK;
}

esp_err_t slime_wifi_wait_connected(const slime_wifi_context_t* wifi_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(wifi_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when waiting until connected.");

	// Log the operation if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context is waiting until the Wi-Fi station is connected to AP.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Wait until the Wi-Fi station is started.
	ESP_RETURN_ON_FALSE((xEventGroupWaitBits(
		/* xEventGroup		= */ wifi_context->event_group,
		/* uxBitsToWaitFor	= */ SLIME_WIFI_CONNECTED | SLIME_WIFI_ERROR,
		/* xClearOnExit		= */ pdFALSE,
		/* xWaitForAllBits	= */ pdFALSE,
		/* xTicksToWait		= */ portMAX_DELAY
	) & SLIME_WIFI_ERROR) == 0U, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	return ESP_OK;
}

esp_err_t slime_wifi_context_new(
			slime_wifi_context_t**			wifi_context_out,
	const	slime_wifi_context_config_t*	wifi_context_config
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created the only Wi-Fi context.
	ESP_RETURN_ON_FALSE(wifi_context_out		!= NULL,	ESP_ERR_INVALID_ARG,	TAG, "No slime_wifi_context_t handle provided when creating Wi-Fi context.");
	ESP_RETURN_ON_FALSE(wifi_context_config		!= NULL,	ESP_ERR_INVALID_ARG,	TAG, "No slime_wifi_context_config_t handle provided when creating Wi-Fi context.");
	ESP_RETURN_ON_FALSE(wifi_context_created	== false,	ESP_ERR_INVALID_STATE,	TAG, "Wi-Fi context already created.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving handles of Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Reserve handles of Wi-Fi context.
	slime_wifi_context_t*	wifi_context				= NULL;
	EventGroupHandle_t		context_event_group			= NULL;
	SemaphoreHandle_t		context_credential_lock		= NULL;
	esp_netif_t*			context_network_interface	= NULL;

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving initialization results of global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Reserve results.
	esp_err_t wifi_0_err	= ESP_FAIL; // Event loop creation result.
	esp_err_t wifi_1_err	= ESP_FAIL; // TCP/IP stack initialization result.
	esp_err_t wifi_2_err	= ESP_FAIL; // Wi-Fi driver initialization result.
	esp_err_t event_0_err	= ESP_FAIL; // On station started event handler registration result.
	esp_err_t event_1_err	= ESP_FAIL; // On station disconnected event handler registration result.
	esp_err_t event_2_err	= ESP_FAIL; // On station got IP event handler registration result.
	esp_err_t event_3_err	= ESP_FAIL; // On station lost IP event handler registration result.

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Initializing global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Load the default Wi-Fi initialized configuration.
	const wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

	// Initialize the global Wi-Fi driver.
	// We ONLY use the event loop inside the Wi-Fi context, so inline the creation of default event loop into the creation of Wi-Fi context.
	ESP_GOTO_ON_ERROR(wifi_0_err = esp_event_loop_create_default(),				error, TAG, "Failed to create default global event loop.");
	ESP_GOTO_ON_ERROR(wifi_1_err = esp_netif_init				(),				error, TAG, "Failed to initialize global TCP/IP stack.");
	ESP_GOTO_ON_ERROR(wifi_2_err = esp_wifi_init				(&init_config),	error, TAG, "Failed to initialize global Wi-Fi driver.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Configuring global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Configure the global Wi-Fi driver using
	ESP_GOTO_ON_ERROR(esp_wifi_set_mode	(WIFI_MODE_STA),	error, TAG, "Failed to set global Wi-Fi driver to station mode.");
	ESP_GOTO_ON_ERROR(esp_wifi_set_ps	(WIFI_PS_NONE),		error, TAG, "Failed to disable power saving of global Wi-Fi driver.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Getting saved configuration from the global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Reserve space for Wi-Fi configuration.
	wifi_config_t wifi_config;

	// Get the saved Wi-Fi station configuration from Wi-Fi driver.
	ESP_GOTO_ON_ERROR(esp_wifi_get_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to get configuration from global Wi-Fi driver.");

	// Check saved credential in configuration.
	if (strnlen((char*) wifi_config.sta.ssid, 32U) > 0U) {
		// Reserve space for the saved credential.
		char credential_ssid[32U];
		char credential_pass[64U];

		// Copy the saved credential from the Wi-Fi configuration.
		strncpy(credential_ssid, (char*) wifi_config.sta.ssid,		32U);
		strncpy(credential_pass, (char*) wifi_config.sta.password,	64U);

		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			ESP_LOGD(TAG, "Found saved credential of SSID \"%.32s\"(%zu) and password length %zu.",
				/* s	*/ credential_ssid,
				/* zu	*/ strnlen(credential_ssid, 32U),
				/* zu	*/ strnlen(credential_pass, 64U)
			);
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Load the default Wi-Fi station configuration into the Wi-Fi configuration.
		wifi_config.sta = wifi_context_config->wifi_sta_config;

		// Replace the default hardcoded credential in the Wi-Fi configuration to saved credential.
		strncpy((char*) wifi_config.sta.ssid,		credential_ssid, 32U);
		strncpy((char*) wifi_config.sta.password,	credential_pass, 64U);
	} else {
		// Log the operation if Wi-Fi debug logging is enabled.
		#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
			// Get the station configuration.
			const wifi_sta_config_t* sta_config = &wifi_context_config->wifi_sta_config;

			ESP_LOGD(TAG, "No saved credential found, use default hardcoded credential of SSID \"%.32s\"(%zu) and password length %zu.",
				/* s	*/ sta_config->ssid,
				/* zu	*/ strnlen((char*) sta_config->ssid,		32U),
				/* zu	*/ strnlen((char*) sta_config->password,	64U)
			);
		#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

		// Load the default Wi-Fi station configuration with the hardcoded credential into the Wi-Fi configuration.
		wifi_config.sta = wifi_context_config->wifi_sta_config;
	}

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Applying Wi-Fi station configuration to global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	ESP_GOTO_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to set configuration of global Wi-Fi driver.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating Wi-Fi context handles.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Create the event group, credential lock, and network interface.
	context_event_group			= xEventGroupCreate					();
	context_credential_lock		= xSemaphoreCreateMutex				();
	context_network_interface	= esp_netif_create_default_wifi_sta	();

	// Check the allocation.
	ESP_GOTO_ON_FALSE(context_event_group		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create event group.");
	ESP_GOTO_ON_FALSE(context_credential_lock	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create credential lock.");
	ESP_GOTO_ON_FALSE(context_network_interface	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create network interface.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Initializing event group.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Clear all valid bits in the event group.
	xEventGroupClearBits(context_event_group,
			SLIME_WIFI_ERROR
		|	SLIME_WIFI_STARTED
		|	SLIME_WIFI_CONNECTED
		|	SLIME_WIFI_FAILED
	);

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating Wi-Fi context struct.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Create the Wi-Fi context handle.
	wifi_context = calloc(1U, sizeof(slime_wifi_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(wifi_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create Wi-Fi context struct.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Filling Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Fill the Wi-Fi context.
	wifi_context->event_group					= context_event_group;
	wifi_context->credential_lock				= context_credential_lock;
	wifi_context->network_interface				= context_network_interface;
	wifi_context->credential_retry_count_max	= wifi_context_config->credential_retry_count_max;
	wifi_context->credential_retry_count		= 0U;

	// Copy the current credential to the Wi-Fi context.
	strncpy(wifi_context->credential_ssid, (char*) wifi_config.sta.ssid,		32U);
	strncpy(wifi_context->credential_pass, (char*) wifi_config.sta.password,	64U);

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Registering callbacks to global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Register callbacks.
	ESP_GOTO_ON_ERROR(event_0_err = esp_event_handler_register(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station start event");
	ESP_GOTO_ON_ERROR(event_1_err = esp_event_handler_register(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected,	wifi_context), error, TAG, "Failed to register handler of Wi-Fi station disconnected event");
	ESP_GOTO_ON_ERROR(event_2_err = esp_event_handler_register(IP_EVENT,	IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station got IP event");
	ESP_GOTO_ON_ERROR(event_3_err = esp_event_handler_register(IP_EVENT,	IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station lost IP event.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Starting global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Start the driver.
	ESP_GOTO_ON_ERROR(esp_wifi_start(), error, TAG, "Failed to start global Wi-Fi driver.");

	// Return the created Wi-Fi context.
	*wifi_context_out = wifi_context;

	// Mark the only Wi-Fi context created.
	wifi_context_created = true;

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context has been created.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	if (!event_3_err) ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,	IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip));		// Cleanup the station lost IP handler.
	if (!event_2_err) ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,	IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip));			// Cleanup the station got IP handler.
	if (!event_1_err) ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected));	// Cleanup the station disconnect handler.
	if (!event_0_err) ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start));			// Cleanup the station start handler.

	if (wifi_context)				free							(wifi_context);					// Cleanup the Wi-Fi context struct.
	if (context_network_interface)	esp_netif_destroy_default_wifi	(context_network_interface);	// Cleanup the Wi-Fi network interface.
	if (context_credential_lock)	vSemaphoreDelete				(context_credential_lock);		// Cleanup the credential lock.
	if (context_event_group)		vEventGroupDelete				(context_event_group);			// Cleanup the Wi-Fi event group.

	if (!wifi_2_err) ESP_ERROR_CHECK(esp_wifi_deinit				()); // Cleanup the global Wi-Fi driver.
	if (!wifi_1_err) ESP_ERROR_CHECK(esp_netif_deinit				()); // Cleanup the global TCP/IP stack.
	if (!wifi_0_err) ESP_ERROR_CHECK(esp_event_loop_delete_default	()); // Cleanup the global default event loop.

	return ret;
}

esp_err_t slime_wifi_context_del(slime_wifi_context_t* wifi_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(wifi_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when releasing Wi-Fi context.");

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Unregistering Wi-Fi event handlers.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Unregister the Wi-Fi event handlers.
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start));
	ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected));
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,		IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,		IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip));

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing Wi-Fi context handles.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Release the event group, credential lock, and network interface.
	vEventGroupDelete				(wifi_context_in->event_group);
	vSemaphoreDelete				(wifi_context_in->credential_lock);
	esp_netif_destroy_default_wifi	(wifi_context_in->network_interface);

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "De-initializing global Wi-Fi driver.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// De-initialize the global Wi-Fi driver.
	ESP_ERROR_CHECK(esp_wifi_deinit					());
	ESP_ERROR_CHECK(esp_netif_deinit				());
	ESP_ERROR_CHECK(esp_event_loop_delete_default	());

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Detach all fields.
	wifi_context_in->event_group				= NULL;
	wifi_context_in->credential_lock			= NULL;
	wifi_context_in->network_interface			= NULL;
	wifi_context_in->credential_retry_count_max	= 0U;
	wifi_context_in->credential_retry_count		= 0U;

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Clearing credential of the Wi-Fi context.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Clear the credential.
	memset(wifi_context_in->credential_ssid, '\0', 32U);
	memset(wifi_context_in->credential_pass, '\0', 64U);

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing Wi-Fi context struct.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	// Free the Wi-Fi context struct.
	free(wifi_context_in);

	// Mark the only Wi-Fi context released.
	wifi_context_created = false;

	// Log the progress if Wi-Fi debug logging is enabled.
	#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
		ESP_LOGD(TAG, "Wi-Fi context has been released.");
	#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

	return ESP_OK;
}