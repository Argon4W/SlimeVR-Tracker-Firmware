#ifndef SLIME_WIFI_H
#define SLIME_WIFI_H

#include "esp_check.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The configuration struct of the Wi-Fi context.
 */
typedef struct {
	wifi_sta_config_t	wifi_sta_config;			/*!< The Wi-Fi station configuration. */
	uint8_t				credential_retry_count_max;	/*!< The maximum retry count allowed before stop reconnecting. */
} slime_wifi_context_config_t;

/**
 * @brief The Wi-Fi context struct.
 */
typedef struct {
	EventGroupHandle_t	event_group;				/*!< The event group of the Wi-Fi context. */
	SemaphoreHandle_t	credential_lock;			/*!< The semaphore mutex lock of the credential. */
	esp_netif_t*		network_interface;			/*!< The TCP/IP network interface of the Wi-Fi context. */
	uint8_t				credential_retry_count_max;	/*!< The maximum retry count allowed before stop reconnecting. */
	uint8_t				credential_retry_count;		/*!< The retry count of the AP credential of the Wi-Fi context. */
	char				credential_ssid[32U];		/*!< The SSID of the AP credential of the Wi-Fi context */
	char				credential_pass[64U];		/*!< The password of the AP credential of the Wi-Fi context. */
} slime_wifi_context_t;

/**
 * @brief					Dynamically connect the Wi-Fi context to the AP of given credential.
 * @param wifi_context		The Wi-Fi context to connect to given credential.
 * @param cred_ssid			The SSID of the AP credential.
 * @param cred_pass			The password of the AP credential.
 * @return					The status of connecting.
 */
esp_err_t slime_wifi_connect(
			slime_wifi_context_t*	wifi_context,
	const	char*					cred_ssid,
	const	char*					cred_pass
);

/**
 * @brief						Get the connection state of the Wi-Fi station of the Wi-Fi context.
 * @param wifi_context			The Wi-Fi context to get the connection state.
 * @param wifi_connected_out	The handle to receive the connection state.
 * @return						The status of getting connection state.
 */
esp_err_t slime_wifi_is_connected(
	const	slime_wifi_context_t*	wifi_context,
			uint8_t*				wifi_connected_out
);

/**
 * @brief				Wait until the Wi-Fi context is connected.
 * @param wifi_context	The Wi-Fi context wait for.
 * @return				The status of waiting.
 */
esp_err_t slime_wifi_wait_connected(const slime_wifi_context_t* wifi_context);

/**
 * @brief						Create the Wi-Fi context.
 * @param wifi_context_out		The handle to receive the created Wi-Fi context.
 * @param wifi_context_config	The configuration of the Wi-Fi context.
 * @return						The status of the creation.
 */
esp_err_t slime_wifi_context_new(
			slime_wifi_context_t**			wifi_context_out,
	const	slime_wifi_context_config_t*	wifi_context_config
);

/**
 * @brief					Release the Wi-Fi context.
 * @param wifi_context_in	The Wi-Fi context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_wifi_context_del(slime_wifi_context_t* wifi_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_WIFI_H
