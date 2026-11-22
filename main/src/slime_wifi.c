#include "string.h"
#include "tgmath.h"
#include "esp_bit_defs.h"
#include "slime_wifi.h"

/**
 * @brief The event group bit indicating that there was an error occurred.
 */
#define SLIME_WIFI_ERROR BIT0

/**
 * @brief The event group bit indicating that Wi-Fi station is started.
 */
#define SLIME_WIFI_STARTED BIT1

/**
 * @brief The event group bit indicating that Wi-Fi station is connected to AP.
 */
#define SLIME_WIFI_CONNECTED BIT2

/**
 * @brief The event group bit indicating that Wi-Fi station failed to connect to AP.
 */
#define SLIME_WIFI_FAILED BIT3

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
 * @brief The definition of the debug logging function of the Slime Wi-Fi
 */
#ifdef CONFIG_SLIME_WIFI_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_WIFI_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_WIFI_LOG(format, ...)
#endif // CONFIG_SLIME_WIFI_DEBUG_LOGGING

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

	// Log the event.
	SLIME_WIFI_LOG("Wi-Fi context started, connecting.");

	// Cast the argument to Wi-Fi context.
	const slime_wifi_context_t* wifi_context = (slime_wifi_context_t*) event_arg;

	// Connect the Wi-Fi station to the AP of credential in the Wi-Fi configuration.
	ESP_GOTO_ON_ERROR(esp_wifi_connect(), error, TAG, "Failed to connect Wi-Fi station to AP.");

	// Set the Wi-Fi context started and disconnected.
	xEventGroupSetBits(wifi_context->wifi_events, SLIME_WIFI_STARTED);

	return;

	// Error occurred.
	error:

	// Log the error.
	SLIME_WIFI_LOG("Error occurred while connecting: %s", esp_err_to_name(ret));
	SLIME_WIFI_LOG("Marking the Wi-Fi context errored.");

	// Set the Wi-Fi context errored.
	xEventGroupSetBits(wifi_context->wifi_events, SLIME_WIFI_ERROR);
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

	// Log the disconnection.
	SLIME_WIFI_LOG("Wi-Fi context has disconnected from AP of SSID \"%.32s\"(%" PRIu8 ") because of reason 0x%02" PRIX8 ". RSSI when disconnected: %" PRId8,
		/* s		*/ wifi_disconn->ssid,
		/* PRIu8	*/ wifi_disconn->ssid_len,
		/* PRIX8	*/ wifi_disconn->reason,
		/* PRId8	*/ wifi_disconn->rssi
	);

	// Set the Wi-Fi context disconnected from AP.
	xEventGroupClearBits(wifi_context->wifi_events, SLIME_WIFI_CONNECTED);

	// Acquire the lock and take the ownership of the credential in the Wi-Fi context.
	xSemaphoreTake(wifi_context->credential_lock, portMAX_DELAY);

	// Abort reconnecting if the retry count reaches the maximum retry count.
	if (wifi_context->credential_retry_count >= wifi_context->credential_retry_count_max) {
		// Log the operation.
		SLIME_WIFI_LOG("Wi-Fi context has tried reconnecting to the same AP for %" PRIu8 "/%" PRIu8 " times. Abort reconnecting until there is new credential.",
			/* PRIu8 */ wifi_context->credential_retry_count,
			/* PRIu8 */ wifi_context->credential_retry_count_max
		);

		// Set the Wi-Fi context failed to connect to AP.
		xEventGroupSetBits(wifi_context->wifi_events, SLIME_WIFI_FAILED);

		// Release the lock, exit critical.
		xSemaphoreGive(wifi_context->credential_lock);

		// Abort the reconnecting.
		return;
	}

	// Increase the retry count (manually disconnecting excluded).
	if (wifi_disconn->reason != WIFI_REASON_ASSOC_LEAVE) {
		wifi_context->credential_retry_count ++;
	}

	// Log the operation.
	SLIME_WIFI_LOG("reconnecting. (%" PRIu8 "/%" PRIu8 ")",
		/* PRIu8 */ wifi_context->credential_retry_count,
		/* PRIu8 */ wifi_context->credential_retry_count_max
	);

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
		// Log the operation.
		SLIME_WIFI_LOG("Wi-Fi context is trying reconnecting to the same AP with credential of SSID \"%.32s\"(%zu) and password length %zu.",
			/* s	*/ credential_ssid,
			/* zu	*/ strnlen(credential_ssid, 32U),
			/* zu	*/ strnlen(credential_pass, 64U)
		);
	} else {
		// Log the operation.
		SLIME_WIFI_LOG("Wi-Fi context is trying connecting to a new AP with credential of SSID \"%.32s\"(%zu) and password length %zu.",
			/* s	*/ credential_ssid,
			/* zu	*/ strnlen(credential_ssid, 32U),
			/* zu	*/ strnlen(credential_pass, 64U)
		);

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

	// Log the error.
	SLIME_WIFI_LOG("Error occurred while reconnecting: %s", esp_err_to_name(ret));
	SLIME_WIFI_LOG("Releasing the credential lock then marking the Wi-Fi context errored.");

	// Release the lock.
	xSemaphoreGive(wifi_context->credential_lock);

	// Set the Wi-Fi context errored.
	xEventGroupSetBits(wifi_context->wifi_events, SLIME_WIFI_ERROR);
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
	// Cast the opaque data to handle, cast the argument to Wi-Fi context.
			slime_wifi_context_t* wifi_context = (slime_wifi_context_t*) event_arg;
	const	slime_wifi_got_ip4_t* wifi_got_ip4 = (slime_wifi_got_ip4_t*) event_data;

	/// Log the IP info.
	SLIME_WIFI_LOG("Wi-Fi context has got IPv4 address: ");
	SLIME_WIFI_LOG("Interface IPv4 address: "			IPSTR, IP2STR(&wifi_got_ip4->ip_info.ip));
	SLIME_WIFI_LOG("Interface IPv4 netmask: "			IPSTR, IP2STR(&wifi_got_ip4->ip_info.netmask));
	SLIME_WIFI_LOG("Interface IPv4 gateway address: "	IPSTR, IP2STR(&wifi_got_ip4->ip_info.gw));

	// Set the Wi-Fi context connected.
	xEventGroupSetBits(wifi_context->wifi_events, SLIME_WIFI_CONNECTED);

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
	// Log the event.
	SLIME_WIFI_LOG("Wi-Fi context lost IP address.");

	// Cast the argument to Wi-Fi context,
	// then set the Wi-Fi context disconnected.
	xEventGroupClearBits(((slime_wifi_context_t*) event_arg)->wifi_events, SLIME_WIFI_CONNECTED);
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
		/* xEventGroup		= */ wifi_context->wifi_events,
		/* uxBitsToWaitFor	= */ SLIME_WIFI_STARTED | SLIME_WIFI_ERROR,
		/* xClearOnExit		= */ pdFALSE,
		/* xWaitForAllBits	= */ pdFALSE,
		/* xTicksToWait		= */ portMAX_DELAY
	) & SLIME_WIFI_ERROR) == 0U, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	// Calculate the length of the SSID and password with '\0' included.
	const size_t credential_ssid_length = strlen(credential_ssid);
	const size_t credential_pass_length = strlen(credential_pass);

	// Log the operation.
	SLIME_WIFI_LOG("Wi-Fi context is trying setting new credential of SSID \"%.32s\"(%zu) and password length %zu to connect.",
		/* s	*/ credential_ssid,
		/* zu	*/ strnlen(credential_ssid, 32U),
		/* zu	*/ strnlen(credential_pass, 64U)
	);

	// Check the length of the credential.
	ESP_RETURN_ON_FALSE(credential_ssid_length <= 32U, ESP_ERR_INVALID_SIZE, TAG, "The SSID is too long.");
	ESP_RETURN_ON_FALSE(credential_pass_length <= 64U, ESP_ERR_INVALID_SIZE, TAG, "The password is too long.");

	// Acquire the lock and take the ownership of the credential in the Wi-Fi context.
	xSemaphoreTake(wifi_context->credential_lock, portMAX_DELAY);

	// Clear the retry count.
	wifi_context->credential_retry_count = 0U;

	// Check if the Wi-Fi context has aborted the reconnecting previously.
	if ((xEventGroupGetBits(wifi_context->wifi_events) & SLIME_WIFI_FAILED) != 0) {
		// Clear the failed bit.
		xEventGroupClearBits(wifi_context->wifi_events, SLIME_WIFI_FAILED);

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

		// Log the operation.
		SLIME_WIFI_LOG("Restart reconnecting cycle to connect to new AP.");

		// Set the Wi-Fi station configuration to the global Wi-Fi driver, then connect the Wi-Fi station to AP.
		// The reconnecting cycle has aborted previously, which means we cannot use disconnect to trigger reconnecting.
		ESP_GOTO_ON_ERROR(esp_wifi_set_config	(WIFI_IF_STA, &wifi_config),	error, TAG, "Failed to set Wi-Fi station configuration.");
		ESP_GOTO_ON_ERROR(esp_wifi_connect		(),								error, TAG, "Failed to connect Wi-Fi station to AP.");
	} else {
		// Update the new credential inside the lock.
		strncpy(wifi_context->credential_ssid, credential_ssid, 32U);
		strncpy(wifi_context->credential_pass, credential_pass, 64U);

		// Log the operation.
		SLIME_WIFI_LOG("Disconnect from current connected/connecting AP to connect to new AP.");

		// Disconnect the Wi-Fi station from AP or abort the ongoing connecting process to trigger the reconnecting using the new credential.
		ESP_GOTO_ON_ERROR(esp_wifi_disconnect(), error, TAG, "Failed to disconnect Wi-Fi station from AP.");
	}

	// Release the lock, exit critical, reconnecting will be blocked until the lock is released to ensure that it can acquire
	// the latest credential.
	xSemaphoreGive(wifi_context->credential_lock);

	return ESP_OK;

	// Error occurred.
	error:

	// Log the error.
	SLIME_WIFI_LOG("Error occurred while dynamically connecting new AP: %s", esp_err_to_name(ret));
	SLIME_WIFI_LOG("Releasing the credential lock.");

	// Release the lock.
	xSemaphoreGive(wifi_context->credential_lock);

	return ret;
}

esp_err_t slime_wifi_is_connected(const slime_wifi_context_t* wifi_context, uint8_t* wifi_connected_out) {
	// We cannot proceed without a context and handle to receive the connection state.
	ESP_RETURN_ON_FALSE(wifi_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when getting connection state.");
	ESP_RETURN_ON_FALSE(wifi_connected_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the connection state when getting connection state.");

	// Log the operation.
	SLIME_WIFI_LOG("Wi-Fi context is trying getting connection state of the Wi-Fi station.");

	// Get the bits from the event group of the Wi-Fi context.
	const EventBits_t bits = xEventGroupGetBits(wifi_context->wifi_events);

	// Extract states from the bits of the event group.
	const uint8_t wifi_conn = (bits & SLIME_WIFI_CONNECTED)	!= 0U;
	const uint8_t wifi_fail = (bits & SLIME_WIFI_ERROR)		== 0U;

	// Check if error occurred inside the Wi-Fi context.
	ESP_RETURN_ON_FALSE(wifi_fail, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	// Return the got connection state.
	*wifi_connected_out = wifi_conn;

	return ESP_OK;
}

esp_err_t slime_wifi_wait_connected(const slime_wifi_context_t* wifi_context, const uint32_t timeout_ticks) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(wifi_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when waiting until connected.");

	// Log the operation.
	SLIME_WIFI_LOG("Wi-Fi context is waiting until the Wi-Fi station is connected to AP.");

	EventBits_t event_group_bits;

	if (timeout_ticks == 0U) {
		// Get the bits from the event group of the Wi-Fi context if no waiting.
		event_group_bits = xEventGroupGetBits(wifi_context->wifi_events);
	} else {
		// Wait the connected/error bit until timeout.
		event_group_bits = xEventGroupWaitBits(
			/* xEventGroup		= */ wifi_context->wifi_events,
			/* uxBitsToWaitFor	= */ SLIME_WIFI_CONNECTED | SLIME_WIFI_ERROR,
			/* xClearOnExit		= */ pdFALSE,
			/* xWaitForAllBits	= */ pdFALSE,
			/* xTicksToWait		= */ timeout_ticks
		);
	}

	// Extract states from the bits of the event group.
	const uint8_t wifi_conn = (event_group_bits & SLIME_WIFI_CONNECTED)	!= 0U;
	const uint8_t wifi_fail = (event_group_bits & SLIME_WIFI_ERROR)		== 0U;

	// Wait until the Wi-Fi station is connected or encounters an error.
	ESP_RETURN_ON_FALSE(wifi_fail, ESP_ERR_WIFI_NOT_CONNECT,	TAG, "Error occurred in the Wi-Fi context.");
	ESP_RETURN_ON_FALSE(wifi_conn, ESP_ERR_INVALID_STATE,		TAG, "Timed out when waiting until connected.");

	return ESP_OK;
}

esp_err_t slime_wifi_get_mac_address(const slime_wifi_context_t* wifi_context, uint8_t* mac_address_out) {
	// We cannot proceed without a context and handle to receive the mac address.
	ESP_RETURN_ON_FALSE(wifi_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when getting mac address.");
	ESP_RETURN_ON_FALSE(mac_address_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the mac address when getting mac address.");

	// Log the operation.
	SLIME_WIFI_LOG("Wi-Fi context is getting mac address.");

	// Get the mac address from the network interface.
	ESP_RETURN_ON_ERROR(esp_netif_get_mac(wifi_context->network_interface, mac_address_out), TAG, "Failed to get mac address.");

	return ESP_OK;
}

esp_err_t slime_wifi_get_rssi(const slime_wifi_context_t* wifi_context, uint8_t* rssi_out) {
	// We cannot proceed without a context and handle to receive the mac address.
	ESP_RETURN_ON_FALSE(wifi_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when getting RSSI.");
	ESP_RETURN_ON_FALSE(rssi_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to receive the mac address when getting RSSI.");

	// Log the operation.
	SLIME_WIFI_LOG("Wi-Fi context is getting mac RSSI.");

	// Get the bits from the event group of the Wi-Fi context.
	const EventBits_t bits = xEventGroupGetBits(wifi_context->wifi_events);

	// Extract states from the bits of the event group.
	const uint8_t wifi_boot = (bits & SLIME_WIFI_STARTED)	!= 0U;
	const uint8_t wifi_fail = (bits & SLIME_WIFI_ERROR)		== 0U;

	// Check if the Wi-Fi context is started or error occurred inside the Wi-Fi context.
	ESP_RETURN_ON_FALSE(wifi_boot, ESP_ERR_INVALID_STATE, TAG, "Wi-Fi context has not started yet.");
	ESP_RETURN_ON_FALSE(wifi_fail, ESP_ERR_INVALID_STATE, TAG, "Error occurred in the Wi-Fi context.");

	// Reserve space for the AP info.
	wifi_ap_record_t wifi_ap_info = {0};

	// Get the RSSI from the AP info of the Wi-Fi station.
	ESP_RETURN_ON_ERROR(esp_wifi_sta_get_ap_info(&wifi_ap_info), TAG, "Failed to get Wi-Fi AP info.");

	// Return the got RSSI.
	*rssi_out = wifi_ap_info.rssi;

	return ESP_OK;
}

esp_err_t slime_wifi_context_new(slime_wifi_context_t** wifi_context_out,const slime_wifi_context_config_t* wifi_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created the only Wi-Fi context.
	ESP_RETURN_ON_FALSE(wifi_context_out		!= NULL,	ESP_ERR_INVALID_ARG,	TAG, "No slime_wifi_context_t handle provided when creating Wi-Fi context.");
	ESP_RETURN_ON_FALSE(wifi_context_config		!= NULL,	ESP_ERR_INVALID_ARG,	TAG, "No slime_wifi_context_config_t handle provided when creating Wi-Fi context.");
	ESP_RETURN_ON_FALSE(wifi_context_created	== false,	ESP_ERR_INVALID_STATE,	TAG, "Wi-Fi context already created.");

	// Log the progress.
	SLIME_WIFI_LOG("Creating Wi-Fi context.");
	SLIME_WIFI_LOG("Reserving handles of Wi-Fi context.");

	// Reserve handles of Wi-Fi context.
	slime_wifi_context_t*	wifi_context		= NULL;
	EventGroupHandle_t		wifi_events			= NULL;
	SemaphoreHandle_t		credential_lock		= NULL;
	esp_netif_t*			network_interface	= NULL;

	// Log the progress.
	SLIME_WIFI_LOG("Reserving initialization results of global Wi-Fi driver.");

	// Reserve results.
	esp_err_t wifi_0_err	= ESP_FAIL; // Event loop creation result.
	esp_err_t wifi_1_err	= ESP_FAIL; // TCP/IP stack initialization result.
	esp_err_t wifi_2_err	= ESP_FAIL; // Wi-Fi driver initialization result.
	esp_err_t event_0_err	= ESP_FAIL; // On station started event handler registration result.
	esp_err_t event_1_err	= ESP_FAIL; // On station disconnected event handler registration result.
	esp_err_t event_2_err	= ESP_FAIL; // On station got IP event handler registration result.
	esp_err_t event_3_err	= ESP_FAIL; // On station lost IP event handler registration result.

	// Log the progress.
	SLIME_WIFI_LOG("Initializing global Wi-Fi driver.");

	// Load the default Wi-Fi initialized configuration.
	const wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();

	// Initialize the global Wi-Fi driver.
	// We ONLY use the event loop inside the Wi-Fi context, so inline the creation of default event loop into the creation of Wi-Fi context.
	ESP_GOTO_ON_ERROR(wifi_0_err = esp_event_loop_create_default(),				error, TAG, "Failed to create default global event loop.");
	ESP_GOTO_ON_ERROR(wifi_1_err = esp_netif_init				(),				error, TAG, "Failed to initialize global TCP/IP stack.");
	ESP_GOTO_ON_ERROR(wifi_2_err = esp_wifi_init				(&init_config),	error, TAG, "Failed to initialize global Wi-Fi driver.");

	// Log the progress.
	SLIME_WIFI_LOG("Configuring global Wi-Fi driver.");

	// Configure the global Wi-Fi driver using
	ESP_GOTO_ON_ERROR(esp_wifi_set_mode	(WIFI_MODE_STA),	error, TAG, "Failed to set global Wi-Fi driver to station mode.");
	ESP_GOTO_ON_ERROR(esp_wifi_set_ps	(WIFI_PS_NONE),		error, TAG, "Failed to disable power saving of global Wi-Fi driver.");

	// Log the progress.
	SLIME_WIFI_LOG("Getting saved configuration from the global Wi-Fi driver.");

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

		// Log the operation.
		SLIME_WIFI_LOG("Found saved credential of SSID \"%.32s\"(%zu) and password length %zu.",
			/* s	*/			credential_ssid,
			/* zu	*/ strnlen(	credential_ssid, 32U),
			/* zu	*/ strnlen(	credential_pass, 64U)
		);

		// Load the default Wi-Fi station configuration into the Wi-Fi configuration.
		wifi_config.sta = wifi_context_config->wifi_sta_config;

		// Replace the default hardcoded credential in the Wi-Fi configuration to saved credential.
		strncpy((char*) wifi_config.sta.ssid,		credential_ssid, 32U);
		strncpy((char*) wifi_config.sta.password,	credential_pass, 64U);
	} else {
		// Log the operation.
		SLIME_WIFI_LOG("No saved credential found, use default hardcoded credential of SSID \"%.32s\"(%zu) and password length %zu.",
			/* s	*/ sta_config->ssid,
			/* zu	*/ strnlen((char*) wifi_context_config->wifi_sta_config.ssid,		32U),
			/* zu	*/ strnlen((char*) wifi_context_config->wifi_sta_config.password,	64U)
		);

		// Load the default Wi-Fi station configuration with the hardcoded credential into the Wi-Fi configuration.
		wifi_config.sta = wifi_context_config->wifi_sta_config;
	}

	// Log the progress.
	SLIME_WIFI_LOG("Applying Wi-Fi station configuration to global Wi-Fi driver.");

	ESP_GOTO_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), error, TAG, "Failed to set configuration of global Wi-Fi driver.");

	// Log the progress.
	SLIME_WIFI_LOG("Creating Wi-Fi context handles.");

	// Create the event group, credential lock, and network interface.
	wifi_events			= xEventGroupCreate					();
	credential_lock		= xSemaphoreCreateMutex				();
	network_interface	= esp_netif_create_default_wifi_sta	();

	// Check the allocation.
	ESP_GOTO_ON_FALSE(wifi_events		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create event group.");
	ESP_GOTO_ON_FALSE(credential_lock	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create credential lock.");
	ESP_GOTO_ON_FALSE(network_interface	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create network interface.");

	// Log the progress.
	SLIME_WIFI_LOG("Initializing event group.");

	// Clear all valid bits in the event group.
	xEventGroupClearBits(wifi_events,
			SLIME_WIFI_ERROR
		|	SLIME_WIFI_STARTED
		|	SLIME_WIFI_CONNECTED
		|	SLIME_WIFI_FAILED
	);

	// Log the progress.
	SLIME_WIFI_LOG("Creating Wi-Fi context struct.");

	// Create the Wi-Fi context handle.
	wifi_context = calloc(1U, sizeof(slime_wifi_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(wifi_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create Wi-Fi context struct.");

	// Log the progress.
	SLIME_WIFI_LOG("Filling Wi-Fi context.");

	// Fill the Wi-Fi context.
	wifi_context->wifi_events					= wifi_events;
	wifi_context->credential_lock				= credential_lock;
	wifi_context->network_interface				= network_interface;
	wifi_context->credential_retry_count_max	= wifi_context_config->credential_retry_count_max;
	wifi_context->credential_retry_count		= 0U;

	// Copy the current credential to the Wi-Fi context.
	strncpy(wifi_context->credential_ssid, (char*) wifi_config.sta.ssid,		32U);
	strncpy(wifi_context->credential_pass, (char*) wifi_config.sta.password,	64U);

	// Log the progress.
	SLIME_WIFI_LOG("Registering callbacks to global Wi-Fi driver.");

	// Register callbacks.
	ESP_GOTO_ON_ERROR(event_0_err = esp_event_handler_register(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station start event");
	ESP_GOTO_ON_ERROR(event_1_err = esp_event_handler_register(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected,	wifi_context), error, TAG, "Failed to register handler of Wi-Fi station disconnected event");
	ESP_GOTO_ON_ERROR(event_2_err = esp_event_handler_register(IP_EVENT,	IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station got IP event");
	ESP_GOTO_ON_ERROR(event_3_err = esp_event_handler_register(IP_EVENT,	IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip,		wifi_context), error, TAG, "Failed to register handler of Wi-Fi station lost IP event.");

	// Log the progress.
	SLIME_WIFI_LOG("Starting global Wi-Fi driver.");

	// Start the driver.
	ESP_GOTO_ON_ERROR(esp_wifi_start(), error, TAG, "Failed to start global Wi-Fi driver.");

	// Reduce the max TX power.
	// ESP_GOTO_ON_ERROR(esp_wifi_set_max_tx_power(32), error, TAG, "Failed to set max TX power.");

	// Return the created Wi-Fi context.
	*wifi_context_out = wifi_context;

	// Mark the only Wi-Fi context created.
	wifi_context_created = true;

	// Log the progress.
	SLIME_WIFI_LOG("Wi-Fi context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_WIFI_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_WIFI_LOG("Cleaning up resources.");

	if (!event_3_err) ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,	IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip));		// Cleanup the station lost IP handler.
	if (!event_2_err) ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT,	IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip));			// Cleanup the station got IP handler.
	if (!event_1_err) ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected));	// Cleanup the station disconnect handler.
	if (!event_0_err) ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start));			// Cleanup the station start handler.

	if (wifi_context)		free							(wifi_context);			// Cleanup the Wi-Fi context struct.
	if (network_interface)	esp_netif_destroy_default_wifi	(network_interface);	// Cleanup the Wi-Fi network interface.
	if (credential_lock)	vSemaphoreDelete				(credential_lock);		// Cleanup the credential lock.
	if (wifi_events)		vEventGroupDelete				(wifi_events);			// Cleanup the Wi-Fi event group.

	if (!wifi_2_err) ESP_ERROR_CHECK(esp_wifi_deinit				()); // Cleanup the global Wi-Fi driver.
	if (!wifi_1_err) ESP_ERROR_CHECK(esp_netif_deinit				()); // Cleanup the global TCP/IP stack.
	if (!wifi_0_err) ESP_ERROR_CHECK(esp_event_loop_delete_default	()); // Cleanup the global default event loop.

	return ret;
}

esp_err_t slime_wifi_context_del(slime_wifi_context_t* wifi_context_in) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(wifi_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when releasing Wi-Fi context.");

	// Log the progress.
	SLIME_WIFI_LOG("Releasing Wi-Fi context.");
	SLIME_WIFI_LOG("Unregistering Wi-Fi event handlers.");

	// Unregister the Wi-Fi event handlers.
	ESP_RETURN_ON_ERROR(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_START,			slime_wifi_on_sta_start),			TAG, "Failed to unregister handler of Wi-Fi station start event");
	ESP_RETURN_ON_ERROR(esp_event_handler_unregister(WIFI_EVENT,	WIFI_EVENT_STA_DISCONNECTED,	slime_wifi_on_sta_disconnected),	TAG, "Failed to unregister handler of Wi-Fi station disconnected event");
	ESP_RETURN_ON_ERROR(esp_event_handler_unregister(IP_EVENT,		IP_EVENT_STA_GOT_IP,			slime_wifi_on_sta_got_ip),			TAG, "Failed to unregister handler of Wi-Fi station got IP event");
	ESP_RETURN_ON_ERROR(esp_event_handler_unregister(IP_EVENT,		IP_EVENT_STA_LOST_IP,			slime_wifi_on_sta_lost_ip),			TAG, "Failed to unregister handler of Wi-Fi station lost IP event.");

	// Log the progress.
	SLIME_WIFI_LOG("Releasing Wi-Fi context handles.");

	// Release the event group, credential lock, and network interface.
	vEventGroupDelete				(wifi_context_in->wifi_events);
	vSemaphoreDelete				(wifi_context_in->credential_lock);
	esp_netif_destroy_default_wifi	(wifi_context_in->network_interface);

	// Log the progress.
	SLIME_WIFI_LOG("De-initializing global Wi-Fi driver.");

	// De-initialize the global Wi-Fi driver.
	ESP_RETURN_ON_ERROR(esp_wifi_deinit					(), TAG, "Failed to de-initialize global Wi-Fi driver.");
	ESP_RETURN_ON_ERROR(esp_netif_deinit				(), TAG, "Failed to de-initialize global TCP/IP stack.");
	ESP_RETURN_ON_ERROR(esp_event_loop_delete_default	(), TAG, "Failed to delete default global event loop.");

	// Log the progress.
	SLIME_WIFI_LOG("Detaching all fields of the Wi-Fi context.");

	// Detach all fields.
	wifi_context_in->wifi_events				= NULL;
	wifi_context_in->credential_lock			= NULL;
	wifi_context_in->network_interface			= NULL;
	wifi_context_in->credential_retry_count_max	= 0U;
	wifi_context_in->credential_retry_count		= 0U;

	// Log the progress.
	SLIME_WIFI_LOG("Clearing credential of the Wi-Fi context.");

	// Clear the credential.
	memset(wifi_context_in->credential_ssid, '\0', 32U);
	memset(wifi_context_in->credential_pass, '\0', 64U);

	// Log the progress.
	SLIME_WIFI_LOG("Releasing Wi-Fi context struct.");

	// Free the Wi-Fi context struct.
	free(wifi_context_in);

	// Mark the only Wi-Fi context released.
	wifi_context_created = false;

	// Log the progress.
	SLIME_WIFI_LOG("Wi-Fi context has been released.");

	return ESP_OK;
}