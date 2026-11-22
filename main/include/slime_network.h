#ifndef SLIME_NETWORK_H
#define SLIME_NETWORK_H

#include "lwip/sockets.h"
#include "esp_check.h"
#include "ceigen.h"
#include "slime_wifi.h"
#include "slime_udp.h"

/**
 * @brief The packet type enum of the network tick.
 */
typedef enum {
	C2S_HEARTBEAT			= 0U,	/*!< The packet type of the client-to-server heartbeat packet. */
	S2C_HEARTBEAT			= 1U,	/*!< The packet type of the server-to-client heartbeat packet. */
	BC_HANDSHAKE			= 3U,	/*!< The packet type of the client-to-server broadcasted handshake packet. */
	C2S_ACCELERATION_DATA	= 4U,	/*!< The packet type of the client-to-server acceleration data packet. */
	BD_PING_PONG			= 10U,	/*!< The packet type of the bidirectional ping-pong packet. */
	C2S_BATTERY_LEVEL		= 12U,	/*!< The packet type of the client-to-server battery level data packet. */
	BD_SENSOR_INFO			= 15U,	/*!< The packet type of the bidirectional sensor info packet. */
	C2S_ROTATION_DATA		= 17U,	/*!< The packet type of the client-to-server rotation data packet. */
	C2S_SIGNAL_STRENGTH		= 19U,	/*!< The packet type of the client-to-server signal strength data packet. */
	BD_FEATURE_FLAG			= 22U	/*!< The packet type of the bidirectional feature flag packet. */
} slime_network_tick_packet_type_t;

/**
 * @brief The state enum of the network tick.
 */
typedef enum {
	HANDSHAKE,	/*!< Network tick is still searching for server. */
	CONNECTED	/*!< Network tick has successfully connected to a server. */
} slime_network_tick_state_t;

/**
 * @brief The timer type enum of the network tick.
 */
typedef enum {
	SERVER_KEEP_ALIVE,		/*!< The type of server keep alive timer. */
	PACKET_HANDSHAKE,		/*!< The type of handshake packet timer. */
	PACKET_SENSOR_INFO,		/*!< The type of sensor info packet timer */
	PACKET_FEATURE_FLAG,	/*!< The type of feature flag packet timer. */
	PACKET_SIGNAL_STRENGTH,	/*!< The type of signal strength packet timer. */
	TIMER_COUNT				/*!< The count of timers. */
} slime_network_tick_timer_type_t;

/**
 * @brief The timer struct of the network tick.
 */
typedef struct {
	uint8_t		enabled;	/*!< True if the timer is enabled. */
	TickType_t	period;		/*!< The update period of the timer. */
	TickType_t	count;		/*!< The time passed since last update in tick count. */
} slime_network_tick_timer_t ;

/**
 * @brief The quaternion data struct.
 */
typedef struct {
	float_t w; /*!< The coefficient W of the quaternion in ENU frame. */
	float_t x; /*!< The coefficient X of the quaternion in ENU frame. */
	float_t y; /*!< The coefficient Y of the quaternion in ENU frame. */
	float_t z; /*!< The coefficient Z of the quaternion in ENU frame. */
} slime_network_quaternion_t;

/**
 * @brief The linear acceleration data struct.
 */
typedef struct {
	float_t x; /*!< The linear acceleration of X-axis in m/s^2 in ENU frame. */
	float_t y; /*!< The linear acceleration of Y-axis in m/s^2 in ENU frame. */
	float_t z; /*!< The linear acceleration of Z-axis in m/s^2 in ENU frame. */
} slime_network_acceleration_t;

/**
 * @brief The battery data struct.
 */
typedef struct {
	float_t battery_voltage;	/*!< The voltage of the battery. */
	float_t battery_percent;	/*!< The percentage of the battery level. */
} slime_network_battery_level_t;

/**
 * @brief The network task configuration struct.
 */
typedef struct {
	uint32_t	task_core_id;		/*!< The ID of the core to which the task is pinned to. */
	uint32_t	task_stack_depth;	/*!< The size of the task stack in bytes. */
	uint32_t	task_priority;		/*!< The priority of the task. */
	char*		task_name;			/*!< The name of the task. */
} slime_network_task_config_t;

/**
 * @brief The network tick configuration struct.
 */
typedef struct {
	slime_udp_context_config_t			socket_context_config;		/*!< The configuration of the UDP context in network tick. */
	slime_buffer_context_config_t		buffer_context_config;		/*!< The configuration of the buffer context in network tick. */
	uint32_t							handshake_packet_period;	/*!< The time in millisecond(s) to wait before sending next client handshake packet. */
	uint32_t							info_packet_period;			/*!< The time in millisecond(s) to wait before sending next client info packet. */
	uint32_t							keep_alive_period;			/*!< The time in millisecond(s) to wait before assuming disconnected from server if no server packet received. */
	uint32_t 							loop_period;				/*!< The time in millisecond(s) to wait before next network tick loop when error occurred. */
	uint32_t							tick_rate;					/*!< The tick rate of the network tick in tick(s)-per-second. */
} slime_network_tick_config_t;

/**
 * @brief The configuration struct of the network context.
 */
typedef struct {
	slime_network_tick_config_t	network_tick_config;		/*!< The configuration of the network tick. */
	slime_network_task_config_t	network_tick_task_config;	/*!< The configuration of the network tick task. */
} slime_network_context_config_t;

/**
 * @brief The network context struct.
 */
typedef struct {
			slime_wifi_context_t*			wifi_context;				/*!< The Wi-Fi context of the network context. */
	const	slime_network_tick_config_t*	network_tick_config;		/*!< The configuration of the network tick. */
			EventGroupHandle_t				network_tick_events;		/*!< The event group of the network tick. */
			TaskHandle_t					network_tick_task_handle;	/*!< The task handle of the network tick task. */
			QueueHandle_t					quaternion_data_mailbox;	/*!< The mailbox of the client-to-server quaternion data. */
			QueueHandle_t					acceleration_data_mailbox;	/*!< The mailbox of the client-to-server acceleration data. */
			QueueHandle_t					battery_level_data_mailbox;	/*!< The mailbox of the client-to-server battery level data. */
} slime_network_context_t;

/**
 * @brief The network tick context struct.
 */
typedef struct {
	uint8_t							mac_address[6];			/*!< The MAC address. */
	slime_network_tick_timer_t		timers[TIMER_COUNT];	/*!< The timers of the network tick. */
	slime_network_tick_state_t		state;					/*!< The state of the network tick. */
	slime_buffer_context_t*			buffer_context;			/*!< The buffer to construct the packet. */
	slime_udp_context_t*			socket_context;			/*!< The UDP socket to send the packet. */
	slime_udp_address_t				server_address;			/*!< The socket address of the server. */
	uint64_t						packet_number;			/*!< The incremental packet number. */
} slime_network_tick_context_t;

/**
 * @brief					Update the quaternion sent to the server of the network context.
 * @param network_context	The network context to update the quaternion.
 * @param quaternion_enu	The quaternion data to be updated to the network context.
 * @return					The status of updating quaternion.
 */
esp_err_t slime_network_update_quaternion(const slime_network_context_t* network_context, ceigen_quaternion_handle_t quaternion_enu);

/**
 * @brief					Update the acceleration sent to the server of the network context.
 * @param network_context	The network context to update the acceleration.
 * @param acceleration_enu	The acceleration data to be updated to the network context.
 * @return					The status of updating acceleration.
 */
esp_err_t slime_network_update_acceleration(const slime_network_context_t* network_context, ceigen_matrix_handle_t acceleration_enu);

/**
 * @brief					Update the battery level sent to the server of the network context.
 * @param network_context	The network context to update the battery level.
 * @param battery_voltage	The battery voltage to be updated to the network context.
 * @param battery_percent	The battery percentage to be updated to the network context.
 * @return					The status of updating battery level.
 */
esp_err_t slime_network_update_battery_level(
	const	slime_network_context_t*	network_context,
			float_t						battery_voltage,
			float_t						battery_percent
);

/**
 * @brief							Create the network context.
 * @param network_context_out		The handle to receive the created network context.
 * @param network_context_config	The configuration of the network context.
 * @param wifi_context				The Wi-Fi context of the network.
 * @return							The status of the creation.
 */
esp_err_t slime_network_context_new(
			slime_network_context_t**		network_context_out,
	const	slime_network_context_config_t*	network_context_config,
			slime_wifi_context_t*			wifi_context
);

/**
 * @brief						Release the network context.
 * @param network_context_in	The network context to be released.
 * @return						The status of releasing.
 */
esp_err_t slime_network_context_del(slime_network_context_t* network_context_in);

#endif // SLIME_NETWORK_H
