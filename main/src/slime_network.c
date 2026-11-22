#include "endian.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "esp_bit_defs.h"
#include "esp_log.h"
#include "slime_network.h"

/**
 * @brief The event group bit indicating that network UDP task should stop.
 */
#define SLIME_NETWORK_STOP BIT0

/**
 * @brief The event group bit indicating that network UDP task has stopped.
 */
#define SLIME_NETWORK_STOPPED BIT1

/**
 * @brief The log tag of the Slime Network.
 */
static const char* TAG = "slime_network";

/**
 * @brief The definition of the debug logging function of the Slime Network.
 */
#ifdef CONFIG_SLIME_NETWORK_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_NETWORK_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_NETWORK_LOG(format, ...)
#endif // CONFIG_SLIME_NETWORK_DEBUG_LOGGING

/**
 * @brief The content of the reference server handshake packet string.
 */
static const char* packet_handshake_string_ref = "Hey OVR =D 5";

/**
 * @brief The length of the reference server handshake packet string.
 */
static const size_t packet_handshake_string_ref_len = 12U;

/**
 * @brief						Reset the passed time of the given timer of the network tick context.
 * @param network_tick_context	The network tick context to reset the timer.
 * @param timer_type			The type of the timer to be reset.
 * @return						The status of resetting timer.
 */
esp_err_t slime_network_tick_reset_timer(slime_network_tick_context_t* network_tick_context, const slime_network_tick_timer_type_t timer_type) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is resetting timer % " PRIu32 ".", timer_type);

	// Reset the passed time of the timer.
	network_tick_context->timers[timer_type].count = 0U;

	return ESP_OK;
}

/**
 * @brief						Disable the given timer of the network tick context.
 * @param network_tick_context	The network tick context to disable the timer.
 * @param timer_type			The type of the timer to be disabled.
 * @return						The status of disabling timer.
 */
esp_err_t slime_network_tick_disable_timer(slime_network_tick_context_t* network_tick_context, const slime_network_tick_timer_type_t timer_type) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is disabling timer % " PRIu32 ".", timer_type);

	// Disable the timer and reset its passed time.
	network_tick_context->timers[timer_type].enabled	= false;
	network_tick_context->timers[timer_type].count		= 0U;

	return ESP_OK;
}

/**
 * @brief						Enable the given timer of the network tick context.
 * @param network_tick_context	The network tick context to enable the timer.
 * @param timer_type			The type of the timer to be enabled.
 * @return						The status of enabling timer.
 */
esp_err_t slime_network_tick_enable_timer(slime_network_tick_context_t* network_tick_context, const slime_network_tick_timer_type_t timer_type) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is enabling timer % " PRIu32 ".", timer_type);

	// Enable the timer and reset its passed time.
	network_tick_context->timers[timer_type].enabled	= true;
	network_tick_context->timers[timer_type].count		= 0U;

	return ESP_OK;
}

/**
 * @brief						Check if a timer has timed out.
 * @param network_tick_context	The network tick context to check the timer.
 * @param timer_type			The type of the timer to check.
 * @param check_out				The result of the timer check.
 * @return						The status of checking timer.
 */
esp_err_t slime_network_tick_check_timer(
			slime_network_tick_context_t*	network_tick_context,
	const	slime_network_tick_timer_type_t	timer_type,
			uint8_t*						check_out
) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is checking timer % " PRIu32 ".", timer_type);

	// Reset the timer if it is timed out.
	if (network_tick_context->timers[timer_type].count >= network_tick_context->timers[timer_type].period) {
		network_tick_context->timers[timer_type].count = 0U;

		// Return the check result.
		*check_out = true;
	}

	return ESP_OK;
}

/**
 * @brief						Begin constructing a new packet.
 * @param network_tick_context	The tick context to construct the packet.
 * @param packet_type			The type of the packet to be constructed.
 * @return						The status of beginning constructing packet.
 */
esp_err_t slime_network_tick_begin_packet(slime_network_tick_context_t* network_tick_context, const slime_network_tick_packet_type_t packet_type) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is constructing a packet of type % " PRIu32 ".", packet_type);

	// Get the packet number of the packet to be sent.
	const uint64_t packet_number = network_tick_context->packet_number ++;

	// Reset the buffer context before constructing packet.
	ESP_RETURN_ON_ERROR(slime_buffer_reset(network_tick_context->buffer_context), TAG, "Failed to reset buffer context.");

	// Write the packet type and the packet number to the buffer context.
	ESP_RETURN_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, packet_type),	TAG, "Failed to write packet type to buffer context.");
	ESP_RETURN_ON_ERROR(slime_buffer_write_uint64(network_tick_context->buffer_context, packet_number),	TAG, "Failed to write packet number to buffer context.");

	return ESP_OK;
}

/**
 * @brief						Send the currently constructing packet to the server.
 * @param network_tick_context	The network tick context to send the packet.
 * @return						The status of sending packet.
 */
esp_err_t slime_network_tick_send_packet(const slime_network_tick_context_t* network_tick_context) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is sending a packet.");

	// Send the packet.
	ESP_RETURN_ON_ERROR(slime_udp_send_packet(
		/* udp_context			= */ network_tick_context->socket_context,
		/* send_address			= */ network_tick_context->server_address,
		/* send_buffer_context	= */ network_tick_context->buffer_context
	), TAG, "Failed to send packet.");

	return ESP_OK;
}

/**
 * @brief						Send an empty packet to the server.
 * @param network_tick_context	The network tick context to send the empty packet.
 * @param packet_type			The packet type of the empty packet.
 * @return						The status of sending empty packet.
 */
esp_err_t slime_network_tick_send_empty_packet(slime_network_tick_context_t* network_tick_context, const slime_network_tick_packet_type_t packet_type) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is sending an empty packet of packet type %" PRIu32 ".", packet_type);

	// Begin the packet.
	ESP_RETURN_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, packet_type), TAG, "Failed to send packet.");

	// Send the packet.
	ESP_RETURN_ON_ERROR(slime_udp_send_packet(
		/* udp_context			= */ network_tick_context->socket_context,
		/* send_address			= */ network_tick_context->server_address,
		/* send_buffer_context	= */ network_tick_context->buffer_context
	), TAG, "Failed to send packet.");

	return ESP_OK;
}

/**
 * @brief						Return the received packet to the server.
 * @param network_tick_context	The network tick context to return the packet.
 * @return						The status of returning packet.
 */
esp_err_t slime_network_tick_return_packet(const slime_network_tick_context_t* network_tick_context) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is returning a packet.");

	// Rewind the buffer context.
	ESP_RETURN_ON_ERROR(slime_buffer_rewind(network_tick_context->buffer_context), TAG, "Failed to rewind buffer context.");

	// Send the packet.
	ESP_RETURN_ON_ERROR(slime_udp_send_packet(
		/* udp_context			= */ network_tick_context->socket_context,
		/* send_address			= */ network_tick_context->server_address,
		/* send_buffer_context	= */ network_tick_context->buffer_context
	), TAG, "Failed to send packet.");

	return ESP_OK;
}

/**
 * @brief						Broadcast the packet to the network.
 * @param network_tick_context	The network tick context to broadcast the packet.
 * @return						The status of broadcasting packet.
 */
esp_err_t slime_network_tick_broadcast_packet(const slime_network_tick_context_t* network_tick_context) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is broadcasting a packet.");

	// The default address is already set to broadcast address.
	ESP_RETURN_ON_ERROR(slime_udp_broadcast_packet(
		/* udp_context			= */ network_tick_context->socket_context,
		/* send_buffer_context	= */ network_tick_context->buffer_context
	), TAG, "Failed to broadcast packet.");

	return ESP_OK;
}

/**
 * @brief						Try receiving a packet from network.
 * @param network_tick_context	The network tick context to try receiving a packet.
 * @param received_address		The handle to receive the socket address of the received packet, can be NULL.
 * @return						The status of try receiving packet.
 */
esp_err_t slime_network_tick_receive_packet(slime_network_tick_context_t* network_tick_context, slime_udp_address_t* received_address) {
	// Log the operation.
	SLIME_NETWORK_LOG("Network tick context is trying receiving a packet.");

	// Reset the buffer context before receiving packet.
	ESP_RETURN_ON_ERROR(slime_buffer_reset(network_tick_context->buffer_context), TAG, "Failed to reset buffer context.");

	// Try receiving a packet from the UDP context.
	ESP_RETURN_ON_ERROR(slime_udp_receive_packet(
		/* udp_context				= */ network_tick_context->socket_context,
		/* received_buffer_context	= */ network_tick_context->buffer_context,
		/* received_address			= */ received_address
	), TAG, "Failed to receive server handshake packet.");

	// Get the length of the bytes available to read in the buffer context.
	const uint32_t length =	network_tick_context->buffer_context->writer_index -
							network_tick_context->buffer_context->reader_index;

	// Reset if we receive a packet.
	if (length > 0U) {
		ESP_RETURN_ON_ERROR(slime_network_tick_reset_timer(network_tick_context, SERVER_KEEP_ALIVE), TAG, "Failed to reset server keep alive timer.");
	}

	return ESP_OK;
}

/**
 * @brief						The network tick function when not connected to server.
 * @param network_context		The network context of the network tick.
 * @param network_tick_context	The network tick context of the network tick.
 */
void slime_network_handshake_tick(const slime_network_context_t* network_context, slime_network_tick_context_t* network_tick_context) {
	esp_err_t ret = ESP_OK;

	// Reserve space for the handshake packet timer check.
	uint8_t handshake_packet = false;

	// Check if the handshake packet has timed out.
	ESP_GOTO_ON_ERROR(slime_network_tick_check_timer(network_tick_context, PACKET_HANDSHAKE, &handshake_packet), error, TAG, "Failed to check handshake packet timer.");

	// Send the handshake packet if timed out.
	if (handshake_packet) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client handshake packet.");

		// Begin constructing client handshake packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, BC_HANDSHAKE), error, TAG, "Failed to begin client handshake packet.");

		// Write the board, sensor type, and MCU to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 2U), error, TAG, "Failed to write to buffer context.");

		// Write the 3 unused IMU data for backwards compatibility to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");

		// Write the firmware version to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint32(network_tick_context->buffer_context, 22U), error, TAG, "Failed to write to buffer context.");

		// Write the firmware version to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_short_string(network_tick_context->buffer_context, "0.0.1-alpha"), error, TAG, "Failed to write to buffer context.");

		// Append the mac address to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_buffer(
			/* buffer_context	= */ network_tick_context->buffer_context,
			/* src_buffer		= */ network_tick_context->mac_address,
			/* src_length		= */ 6U
		), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Broadcasting the client handshake packet.");

		// Broadcast the client handshake packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_broadcast_packet(network_tick_context), error, TAG, "Failed to send client handshake packet.");
	}

	// Try receiving the server handshake packet.
	while (true) {
		// Log the operation.
		SLIME_NETWORK_LOG("Try receiving a server handshake packet.");
		SLIME_NETWORK_LOG("Reserving space for the socket address of the received packet.");

		// Reserve space for received address.
		slime_udp_address_t	received_address = {0};

		// Try receiving a packet from the UDP context.
		ESP_GOTO_ON_ERROR(slime_network_tick_receive_packet(network_tick_context, &received_address), error, TAG, "Failed to receive server handshake packet.");

		// Get the length of the bytes available to read in the buffer context.
		const uint32_t length =	network_tick_context->buffer_context->writer_index -
								network_tick_context->buffer_context->reader_index;

		// If there is no packet received, skip to next tick.
		if (length <= 0U) {
			// Log the operation.
			SLIME_NETWORK_LOG("No packet received, skipping to next tick.");

			// Skip to next tick.
			break;
		}

		// Log the operation.
		SLIME_NETWORK_LOG("Parsing received packet.");
		SLIME_NETWORK_LOG("Reserving space for the received packet header.");

		// Reserve space for the special packet header of server handshake packet.
		uint8_t packet_header = 0U;

		// Read the special packet header from the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_read_uint8(network_tick_context->buffer_context, &packet_header), error, TAG, "Failed to read from buffer context.");

		// Check if the packet is the server handshake packet.
		if (packet_header == 3U) {
			// Log the operation.
			SLIME_NETWORK_LOG("Server handshake detected, checking server handshake string.");
			SLIME_NETWORK_LOG("SReserving space for the received handshake string.");

			// Reserve the space for the handshake string to be checked.
			char packet_handshake_string[packet_handshake_string_ref_len];

			// Read the handshake string from the packet.
			ESP_GOTO_ON_ERROR(slime_buffer_read_buffer(
				/* buffer_context	= */	network_tick_context->buffer_context,
				/* dst_buffer		= */ &	packet_handshake_string,
				/* dst_length		= */	packet_handshake_string_ref_len
			), error, TAG, "Failed to read from buffer context.");

			// Check if the handshake is successful.
			if (strncmp(
				/* a_string	= */ packet_handshake_string,
				/* b_string	= */ packet_handshake_string_ref,
				/* max_len	= */ packet_handshake_string_ref_len
			) == 0) {
				// Log the socket address of the connected server, this only works in Little-Endian device since LwIP stores IP in Big-Endian (who use Big-Endian?).
				SLIME_NETWORK_LOG("Successfully connected to server: %" PRIu8 "%." PRIu8 "%." PRIu8 "%." PRIu8 ":%" PRIu16 ", ignored.",
					/* PRIu8	*/ (uint8_t)	(received_address.addr >> 0U)	& 0xFFU,
					/* PRIu8	*/ (uint8_t)	(received_address.addr >> 8U)	& 0xFFU,
					/* PRIu8	*/ (uint8_t)	(received_address.addr >> 16U)	& 0xFFU,
					/* PRIu8	*/ (uint8_t)	(received_address.addr >> 24U)	& 0xFFU,
					/* PRIu16	*/ ntohs		(received_address.port)
				);

				// Handshake successful.
				// Enable the packet timers.
				ESP_GOTO_ON_ERROR(slime_network_tick_enable_timer(network_tick_context, PACKET_SENSOR_INFO),		error, TAG, "Failed to enable sensor info packet timer.");
				ESP_GOTO_ON_ERROR(slime_network_tick_enable_timer(network_tick_context, PACKET_FEATURE_FLAG),		error, TAG, "Failed to enable feature flag packet timer.");
				ESP_GOTO_ON_ERROR(slime_network_tick_enable_timer(network_tick_context, PACKET_SIGNAL_STRENGTH),	error, TAG, "Failed to enable signal strength packet timer.");

				// Disable the handshake timer.
				ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer(network_tick_context, PACKET_HANDSHAKE), error, TAG, "Failed to disable handshake packet timer.");

				// Store the socket address of the connected server.
				network_tick_context->server_address = received_address;

				// Set the state to connected.
				network_tick_context->state = CONNECTED;

				// Exit the tick.
				break;
			}

			// Log the operation.
			SLIME_NETWORK_LOG("Invalid handshake packet, ignored.");
		}
	}

	return;

	// Error occurred.
	error:

	// Reserve space for error string.
	char error_string[64U];

	// Log the error.
	SLIME_NETWORK_LOG("Error occurred while handshake ticking: %s", esp_err_to_name_r(ret, error_string, 64U));
	SLIME_NETWORK_LOG("Skipping current handshake tick.");
}

/**
 * @brief						The network tick function when successfully connected to server.
 * @param network_context		The network context of the network tick.
 * @param network_tick_context	The network tick context of the network tick.
 */
void slime_network_connected_tick(const slime_network_context_t* network_context, slime_network_tick_context_t* network_tick_context) {
	esp_err_t ret = ESP_OK;

	// Reserve space for the server keep alive timer check.
	uint8_t keep_alive = false;

	// Check if the server has timed out.
	ESP_GOTO_ON_ERROR(slime_network_tick_check_timer(network_tick_context, SERVER_KEEP_ALIVE, &keep_alive), error, TAG, "Failed to check server keep alive timer.");

	// Check if the server timed out.
	if (keep_alive) {
		// Log the operation.
		SLIME_NETWORK_LOG("Server timed out.");

		// Disable the packet timers.
		ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer(network_tick_context, PACKET_SENSOR_INFO),		error, TAG, "Failed to disable sensor info packet timer.");
		ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer(network_tick_context, PACKET_FEATURE_FLAG),		error, TAG, "Failed to disable feature flag packet timer.");
		ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer(network_tick_context, PACKET_SIGNAL_STRENGTH),	error, TAG, "Failed to disable signal strength packet timer.");

		// Enable the handshake timer.
		ESP_GOTO_ON_ERROR(slime_network_tick_enable_timer(network_tick_context, PACKET_HANDSHAKE), error, TAG, "Failed to enable handshake packet timer.");

		// Reset the state to handshake.
		network_tick_context->state = HANDSHAKE;

		// Abort the current tick.
		return;
	}

	// Reserve space for the packet timer checks.
	uint8_t sensor_info_packet		= false;
	uint8_t feature_flag_packet		= false;
	uint8_t signal_strength_packet	= false;

	// Check if the packet timers have timed out.
	ESP_GOTO_ON_ERROR(slime_network_tick_check_timer(network_tick_context, PACKET_SENSOR_INFO,		&sensor_info_packet),		error, TAG, "Failed to check sensor info packet timer.");
	ESP_GOTO_ON_ERROR(slime_network_tick_check_timer(network_tick_context, PACKET_FEATURE_FLAG,		&feature_flag_packet),		error, TAG, "Failed to check feature flag packet timer.");
	ESP_GOTO_ON_ERROR(slime_network_tick_check_timer(network_tick_context, PACKET_SIGNAL_STRENGTH,	&signal_strength_packet),	error, TAG, "Failed to check signal strength packet timer.");

	// Send the sensor info packet if timed out.
	if (sensor_info_packet) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client sensor info packet.");

		// Begin constructing client sensor info packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, BD_SENSOR_INFO), error, TAG, "Failed to begin client sensor info packet.");

		// Write the sensor ID, sensor status, and sensor type to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 1U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");

		// Write the sensor config data bitmap to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint16(network_tick_context->buffer_context, 0b1111U), error, TAG, "Failed to write to buffer context.");

		// Write rest calibration, sensor position, and sensor data type to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 1U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client sensor info packet.");

		// Send the sensor info packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client sensor info packet.");
	}

	// Send the feature flag packet if timed out.
	if (feature_flag_packet) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client feature flag packet.");

		// Construct client feature flag packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, BD_FEATURE_FLAG), error, TAG, "Failed to begin client feature flag packet.");

		// Write the client feature flags to the buffer context. (No example feature, no sensor config feature)
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0b00U), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client feature flag packet.");

		// Send the feature flag packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client feature flag packet.");
	}

	// Send the signal strength packet if timed out.
	if (signal_strength_packet) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client signal strength packet.");

		// Construct client feature flag packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, C2S_SIGNAL_STRENGTH), error, TAG, "Failed to begin client signal strength packet.");

		// Reserve space for the signal strength.
		uint8_t rssi = 0U;

		// Get the signal strength from the Wi-Fi context.
		ESP_GOTO_ON_ERROR(slime_wifi_get_rssi(network_context->wifi_context, &rssi), error, TAG, "Failed to get signal strength.");

		// Write the sensor ID and the signal strength to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 255U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, rssi), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client signal strength packet.");

		// Send the signal strength packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client signal strengt packet.");
	}

	// Reserve space for the incoming mailbox data.
	slime_network_quaternion_t		incoming_quaternion		= {0};
	slime_network_acceleration_t	incoming_acceleration	= {0};
	slime_network_battery_level_t	incoming_battery_level	= {0};

	// Send if there is new incoming rotation data in the mailbox.
	if (xQueueReceive(network_context->quaternion_data_mailbox, &incoming_quaternion, 0U) == pdTRUE) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client rotation data packet.");

		// Begin constructing client rotation data packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, C2S_ROTATION_DATA), error, TAG, "Failed to begin client rotation data packet.");

		// Append the sensor ID and the data type to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 1U), error, TAG, "Failed to write to buffer context.");

		// Write the four quaternion coefficients to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_quaternion.x), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_quaternion.y), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_quaternion.z), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_quaternion.w), error, TAG, "Failed to write to buffer context.");

		// Write the accuracy info to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client rotation data packet.");

		// Send the rotation data packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client rotation data packet.");
	}

	// Send if there is new incoming acceleration data in the mailbox.
	if (xQueueReceive(network_context->acceleration_data_mailbox, &incoming_acceleration, 0U) == pdTRUE) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client acceleration data packet.");

		// Begin constructing client acceleration data packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, C2S_ACCELERATION_DATA), error, TAG, "Failed to begin client acceleration data packet.");

		// Write the four quaternion coefficients to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_acceleration.x), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_acceleration.y), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_acceleration.z), error, TAG, "Failed to write to buffer context.");

		// Write the sensor ID to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_uint8(network_tick_context->buffer_context, 0U), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client acceleration data packet.");

		// Send the acceleration data packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client acceleration data packet.");
	}

	// Send if there is new incoming battery level data in the mailbox.
	if (xQueueReceive(network_context->battery_level_data_mailbox, &incoming_battery_level, 0U) == pdTRUE) {
		// Log the operation.
		SLIME_NETWORK_LOG("Constructing client battery level data packet.");

		// Begin constructing client battery level data packet.
		ESP_GOTO_ON_ERROR(slime_network_tick_begin_packet(network_tick_context, C2S_BATTERY_LEVEL), error, TAG, "Failed to begin client battery level data packet.");

		// Write the battery voltage and battery level percentage to the buffer context.
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_battery_level.battery_voltage), error, TAG, "Failed to write to buffer context.");
		ESP_GOTO_ON_ERROR(slime_buffer_write_float(network_tick_context->buffer_context, incoming_battery_level.battery_percent), error, TAG, "Failed to write to buffer context.");

		// Log the operation.
		SLIME_NETWORK_LOG("Sending client battery level data packet.");

		// Send the battery level data packet to server.
		ESP_GOTO_ON_ERROR(slime_network_tick_send_packet(network_tick_context), error, TAG, "Failed to send client battery level data packet.");
	}

	// Log the operation.
	SLIME_NETWORK_LOG("Try receiving a packet.");
	SLIME_NETWORK_LOG("Reserving space for the socket address of the received packet.");

	// Reserve space for received address.
	slime_udp_address_t	received_address = {0};

	// Try receiving a packet from the UDP context.
	ESP_GOTO_ON_ERROR(slime_network_tick_receive_packet(network_tick_context, &received_address), error, TAG, "Failed to try receiving server packet.");

	// Get the length of the bytes available to read in the buffer context.
	const uint32_t length =	network_tick_context->buffer_context->writer_index -
							network_tick_context->buffer_context->reader_index;

	// If there is no packet received, skip to next tick.
	if (length <= 0) {
		// Log the operation.
		SLIME_NETWORK_LOG("No packet received, skipping to next tick.");

		// Skip to next tick.
		return;
	}

	// Check the socket address of the packet.
	if (	received_address.addr != network_tick_context->server_address.addr
		||	received_address.port != network_tick_context->server_address.port
	) {
		// Log the socket address of the received packet, this only works in Little-Endian device since LwIP stores IP in Big-Endian (who use Big-Endian?).
		SLIME_NETWORK_LOG("Unknown server: %" PRIu8 "%." PRIu8 "%." PRIu8 "%." PRIu8 ":%" PRIu16 ", ignored.",
			/* PRIu8	*/ (uint8_t)	(received_address.addr >> 0U)	& 0xFFU,
			/* PRIu8	*/ (uint8_t)	(received_address.addr >> 8U)	& 0xFFU,
			/* PRIu8	*/ (uint8_t)	(received_address.addr >> 16U)	& 0xFFU,
			/* PRIu8	*/ (uint8_t)	(received_address.addr >> 24U)	& 0xFFU,
			/* PRIu16	*/ ntohs		(received_address.port)
		);

		// Skip to next tick.
		return;
	}

	// Log the operation.
	SLIME_NETWORK_LOG("Parsing received packet.");
	SLIME_NETWORK_LOG("Reserving space for the received packet type.");

	// Reserve space for the packet header.
	uint32_t received_packet_type = 0U;

	// Read the packet type from the receiving packet buffer.
	ESP_GOTO_ON_ERROR(slime_buffer_read_uint32(network_tick_context->buffer_context, &received_packet_type), error, TAG, "Failed to read from buffer context.");

	switch (received_packet_type) {
		// Process all known packet types.
		case S2C_HEARTBEAT:		ESP_GOTO_ON_ERROR(slime_network_tick_send_empty_packet	(network_tick_context, C2S_HEARTBEAT),			error, TAG, "Failed to send client heartbeat packet.");			break;
		case BD_PING_PONG:		ESP_GOTO_ON_ERROR(slime_network_tick_return_packet		(network_tick_context),							error, TAG, "Failed to return ping-pong packet.");				break;
		case BD_SENSOR_INFO:	ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer		(network_tick_context, PACKET_SENSOR_INFO),		error, TAG, "Failed to disable sensor info packet timer.");		break;
		case BD_FEATURE_FLAG:	ESP_GOTO_ON_ERROR(slime_network_tick_disable_timer		(network_tick_context, PACKET_FEATURE_FLAG),	error, TAG, "Failed to disable feature flag packet timer.");	break;

		// Unknown.
		default:
			// Log the unknown packet type.
			SLIME_NETWORK_LOG("Unknown packet type %" PRIu32 " received. ignored.", received_packet_type);
	}

	return;

	// Error occurred.
	error:

	// Reserve space for error string.
	char error_string[64U];

	// Log the error.
	SLIME_NETWORK_LOG("Error occurred while connected ticking: %s", esp_err_to_name_r(ret, error_string, 64U));
	SLIME_NETWORK_LOG("Skipping current tick.");
}

/**
 * @brief				The network tick task function of the network tick.
 * @param pvParameters	The network context of the network tick task, should always by slime_network_context_t*.
 */
void slime_network_tick_task(void *pvParameters) {
	esp_err_t ret = ESP_OK;

	// Cast the opaque parameter handle to network context handle.
	const slime_network_context_t* network_context = (slime_network_context_t*) pvParameters;

	// Log the progress.
	SLIME_NETWORK_LOG("Starting network tick task.");
	SLIME_NETWORK_LOG("Reserving handles of network tick context.");

	// Reserve handles of the tick context.
	slime_network_tick_context_t*	network_tick_context	= NULL;
	slime_buffer_context_t*			buffer_context			= NULL;
	slime_udp_context_t*			socket_context			= NULL;

	// Log the progress.
	SLIME_NETWORK_LOG("Waiting until Wi-Fi context is connected.");

	// Wait until the Wi-Fi context is connected.
	ESP_GOTO_ON_ERROR(slime_wifi_wait_connected(network_context->wifi_context, portMAX_DELAY), stop, TAG, "Failed to wait until the Wi-Fi context is connected.");

	// Log the progress.
	SLIME_NETWORK_LOG("Creating network tick context struct.");

	// Create the network tick context struct.
	network_tick_context = calloc(1U, sizeof(slime_network_tick_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(network_tick_context != NULL, ESP_ERR_NO_MEM, stop, TAG, "Failed to create network tick context struct.");

	// Get the network tick configuration from the network context.
	const slime_network_tick_config_t* network_tick_config = network_context->network_tick_config;

	// Log the progress.
	SLIME_NETWORK_LOG("Initializing time periods in ticks from network tick configuration.");
	SLIME_NETWORK_LOG("Initializing time periods of network tick context.");

	// Initialize the periods of the timers.
	network_tick_context->timers[PACKET_HANDSHAKE]		.period = pdMS_TO_TICKS(network_tick_config->handshake_packet_period);
	network_tick_context->timers[SERVER_KEEP_ALIVE]		.period = pdMS_TO_TICKS(network_tick_config->keep_alive_period);
	network_tick_context->timers[PACKET_SENSOR_INFO]	.period	= pdMS_TO_TICKS(network_tick_config->info_packet_period);
	network_tick_context->timers[PACKET_FEATURE_FLAG]	.period	= pdMS_TO_TICKS(network_tick_config->info_packet_period);
	network_tick_context->timers[PACKET_SIGNAL_STRENGTH].period	= pdMS_TO_TICKS(network_tick_config->info_packet_period);

	// Log the progress.
	SLIME_NETWORK_LOG("Initializing time periods of network ticking loop.");

	// Calculate the delay times in ticks.
	const TickType_t loop_period = pdMS_TO_TICKS(			network_tick_config->loop_period);
	const TickType_t tick_period = pdMS_TO_TICKS(1000U /	network_tick_config->tick_rate);

	// Initialize the tick times.
	TickType_t last_wake_time = xTaskGetTickCount();
	TickType_t last_tick_time = xTaskGetTickCount();

	// Initialize the tick context states.
	uint8_t create_context = true;

	// Tick until stop event received.
	while ((xEventGroupGetBits(network_context->network_tick_events) & SLIME_NETWORK_STOP) == 0U) {
		// Wait until next tick.
		xTaskDelayUntil(&last_wake_time, tick_period);

		// Log the operation.
		SLIME_NETWORK_LOG("Network context is ticking.");
		SLIME_NETWORK_LOG("Checking if Wi-Fi context is still connected.");

		// Check if the Wi-Fi context is still connected.
		ESP_GOTO_ON_ERROR(slime_wifi_wait_connected(network_context->wifi_context, 0U), error, TAG, "Wi-Fi context disconnected.");

		// Get current time.
		const TickType_t this_tick_time = xTaskGetTickCount();

		// Create the network tick context if needed.
		if (create_context) {
			create_context = false;

			// Log the progress.
			SLIME_NETWORK_LOG("Creating network tick context.");
			SLIME_NETWORK_LOG("Creating UDP context and buffer context.");

			// Create the contexts.
			ESP_GOTO_ON_ERROR(slime_udp_context_new		(&socket_context, &network_tick_config->socket_context_config), error, TAG, "Failed to create UDP context.");
			ESP_GOTO_ON_ERROR(slime_buffer_context_new	(&buffer_context, &network_tick_config->buffer_context_config), error, TAG, "Failed to create buffer context.");

			// Log the progress.
			SLIME_NETWORK_LOG("Initializing timers.");

			// Initialize passed time of all timers.
			network_tick_context->timers[PACKET_HANDSHAKE]		.count = 0U;
			network_tick_context->timers[SERVER_KEEP_ALIVE]		.count = 0U;
			network_tick_context->timers[PACKET_SENSOR_INFO]	.count = 0U;
			network_tick_context->timers[PACKET_FEATURE_FLAG]	.count = 0U;
			network_tick_context->timers[PACKET_SIGNAL_STRENGTH].count = 0U;

			// Enable the server keep alive timer and the handshake packet timer, disable all packet timers.
			network_tick_context->timers[PACKET_HANDSHAKE]		.enabled = true;
			network_tick_context->timers[SERVER_KEEP_ALIVE]		.enabled = true;
			network_tick_context->timers[PACKET_SENSOR_INFO]	.enabled = false;
			network_tick_context->timers[PACKET_FEATURE_FLAG]	.enabled = false;
			network_tick_context->timers[PACKET_SIGNAL_STRENGTH].enabled = false;

			// Log the progress.
			SLIME_NETWORK_LOG("Finalizing network tick context.");

			// Fill the tick context.
			network_tick_context->state				= HANDSHAKE;
			network_tick_context->buffer_context	= buffer_context;
			network_tick_context->socket_context	= socket_context;
			network_tick_context->packet_number		= 0U;

			// Fill the mac address from the Wi-Fi context.
			ESP_GOTO_ON_ERROR(slime_wifi_get_mac_address(network_context->wifi_context, network_tick_context->mac_address), error, TAG, "Failed to get mac address.");
		} else {
			// Calculate the time passed since last tick.
			const TickType_t passed = this_tick_time - last_tick_time;

			// Update the tick count passed of timers.
			for (uint32_t timer_type = 0U; timer_type < TIMER_COUNT; timer_type ++) {

				// Update the time passed of the timer if enabled.
				if (network_tick_context->timers[timer_type].enabled) {
					network_tick_context->timers[timer_type].count += passed;
				}
			}
		}

		// Update the tick time.
		last_tick_time = this_tick_time;

		// Tick based on the current state.
		switch (network_tick_context->state) {
			case HANDSHAKE: slime_network_handshake_tick(network_context, network_tick_context); break;
			case CONNECTED: slime_network_connected_tick(network_context, network_tick_context); break;
		}

		// Proceed to next tick.
		continue;

		// Resource cleanup when error occurred.
		error:

		// Reserve space for error string.
		char error_string[64U];

		// Log the error.
		SLIME_NETWORK_LOG("Error occurred when ticking network: %s", esp_err_to_name_r(ret, error_string, 64U));
		SLIME_NETWORK_LOG("Resetting network tick context.");

		if (buffer_context) ESP_GOTO_ON_ERROR(slime_buffer_context_del	(buffer_context), stop, TAG, "Failed to release buffer context.");	// Cleanup the buffer context.
		if (socket_context) ESP_GOTO_ON_ERROR(slime_udp_context_del		(socket_context), stop, TAG, "Failed to release UDP context.");		// Cleanup the UDP context.

		// Restart cooldown.
		vTaskDelay(loop_period);

		// Detach context handles.
		socket_context = NULL;
		buffer_context = NULL;
		create_context = true;

		// Reset tick time.
		last_wake_time = xTaskGetTickCount();
	}

	stop:

	// Cleanup the network tick context.
	if (network_tick_context) free(network_tick_context);

	// Mark the network task stopped.
	xEventGroupSetBits(network_context->network_tick_events, SLIME_NETWORK_STOPPED);

	// Log the stop.
	SLIME_NETWORK_LOG("Network tick task has stopped.");
}

esp_err_t slime_network_update_quaternion(const slime_network_context_t* network_context, const ceigen_quaternion_handle_t quaternion_enu) {
	// We cannot proceed without a context and a quaternion to update.
	ESP_RETURN_ON_FALSE(network_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_t handle provided when updating quaternion.");
	ESP_RETURN_ON_FALSE(quaternion_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_quaternion_handle_t handle provided when updating quaternion.");

	// Log the operation.
	SLIME_NETWORK_LOG("Network context is updating quaternion.");

	// Check if the transmission task of the screen context is still running.
	ESP_RETURN_ON_FALSE((xEventGroupGetBits(network_context->network_tick_events) & SLIME_NETWORK_STOPPED) == 0U, ESP_ERR_INVALID_STATE, TAG, "Network tick task of the network context has already stopped.");

	// Get the coefficients of the quaternion to construct the network quaternion data struct.
	const slime_network_quaternion_t network_quaternion_enu = {
		.w = ceigen_get_quaternion_w(quaternion_enu),
		.x = ceigen_get_quaternion_x(quaternion_enu),
		.y = ceigen_get_quaternion_y(quaternion_enu),
		.z = ceigen_get_quaternion_z(quaternion_enu)
	};

	// Log the operation.
	SLIME_NETWORK_LOG("Updating network quaternion: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
		/* .2f	*/ network_quaternion_enu.w,
		/* .2f	*/ network_quaternion_enu.x,
		/* .2f	*/ network_quaternion_enu.y,
		/* .2f	*/ network_quaternion_enu.z
	);

	// Update the network quaternion in the quaternion data mailbox of the network context.
	ESP_RETURN_ON_FALSE(xQueueOverwrite(network_context->quaternion_data_mailbox, &network_quaternion_enu) == pdPASS, ESP_ERR_INVALID_STATE, TAG, "Failed to update network quaternion data.");

	return ESP_OK;
}

esp_err_t slime_network_update_acceleration(const slime_network_context_t* network_context, const ceigen_matrix_handle_t acceleration_enu) {
	// We cannot proceed without a context and an acceleration to update.
	ESP_RETURN_ON_FALSE(network_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_t handle provided when updating acceleration.");
	ESP_RETURN_ON_FALSE(acceleration_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when updating acceleration.");

	// Log the operation.
	SLIME_NETWORK_LOG("Network context is updating acceleration.");

	// Check if the transmission task of the screen context is still running.
	ESP_RETURN_ON_FALSE((xEventGroupGetBits(network_context->network_tick_events) & SLIME_NETWORK_STOPPED) == 0U, ESP_ERR_INVALID_STATE, TAG, "Network tick task of the network context has already stopped.");

	// Get the coefficients of the acceleration to construct the network acceleration data struct.
	const slime_network_acceleration_t network_acceleration_enu = {
		.x = ceigen_get_matrix_coefficient(acceleration_enu, 0U, 0U),
		.y = ceigen_get_matrix_coefficient(acceleration_enu, 1U, 0U),
		.z = ceigen_get_matrix_coefficient(acceleration_enu, 2U, 0U)
	};

	// Log the operation.
	SLIME_NETWORK_LOG("Updating network acceleration: x=%.2f, y=%.2f, z=%.2f.",
		/* .2f	*/ network_acceleration_enu.x,
		/* .2f	*/ network_acceleration_enu.y,
		/* .2f	*/ network_acceleration_enu.z
	);

	// Update the network quaternion in the acceleration data mailbox of the network context.
	ESP_RETURN_ON_FALSE(xQueueOverwrite(network_context->acceleration_data_mailbox, &network_acceleration_enu) == pdPASS, ESP_ERR_INVALID_STATE, TAG, "Failed to update network acceleration data.");

	return ESP_OK;
}

esp_err_t slime_network_update_battery_level(
	const slime_network_context_t*	network_context,
	const float_t					battery_voltage,
	const float_t					battery_percent
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(network_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_t handle provided when updating battery level.");

	// Log the operation.
	SLIME_NETWORK_LOG("Network context is updating battery level.");

	// Check if the transmission task of the screen context is still running.
	ESP_RETURN_ON_FALSE((xEventGroupGetBits(network_context->network_tick_events) & SLIME_NETWORK_STOPPED) == 0U, ESP_ERR_INVALID_STATE, TAG, "Network tick task of the network context has already stopped.");

	// Construct the network battery level data struct.
	const slime_network_battery_level_t network_battery_level = {
		.battery_voltage = battery_voltage,
		.battery_percent = battery_percent
	};

	// Log the operation.
	SLIME_NETWORK_LOG("Updating network acceleration: voltage=%.2f V, percentage=%.2f %%.",
		/* .2f	*/ network_battery_level.battery_voltage,
		/* .2f	*/ network_battery_level.battery_percent * 100.0f
	);

	// Update the network quaternion in the battery level data mailbox of the network context.
	ESP_RETURN_ON_FALSE(xQueueOverwrite(network_context->battery_level_data_mailbox, &network_battery_level) == pdPASS, ESP_ERR_INVALID_STATE, TAG, "Failed to update network battery level data.");

	return ESP_OK;
}

esp_err_t slime_network_context_new(
			slime_network_context_t**		network_context_out,
	const	slime_network_context_config_t*	network_context_config,
			slime_wifi_context_t*			wifi_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration, a Wi-Fi context, and a handle to receive the created network context.
	ESP_RETURN_ON_FALSE(network_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_t handle provided when creating network context.");
	ESP_RETURN_ON_FALSE(network_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_config_t handle provided when creating network context.");
	ESP_RETURN_ON_FALSE(wifi_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_wifi_context_t handle provided when creating network context.");

	// Log the progress.
	SLIME_NETWORK_LOG("Creating network context.");
	SLIME_NETWORK_LOG("Reserving handles of network context.");

	// Reserve handles of network context.
	slime_network_context_t*	network_context				= NULL;
	EventGroupHandle_t			network_tick_events			= NULL;
	QueueHandle_t				quaternion_data_mailbox		= NULL;
	QueueHandle_t				acceleration_data_mailbox	= NULL;
	QueueHandle_t				battery_level_data_mailbox	= NULL;

	// Log the progress.
	SLIME_NETWORK_LOG("Creating event group.");

	// Create the event group.
	network_tick_events = xEventGroupCreate();

	// Check the allocation.
	ESP_GOTO_ON_FALSE(network_tick_events != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the network tick event group.");

	// Clear all bits of the network tick event group.
	xEventGroupClearBits(network_tick_events,
			SLIME_NETWORK_STOP
		|	SLIME_NETWORK_STOPPED
	);

	// Log the progress.
	SLIME_NETWORK_LOG("Creating data mailboxes.");

	// Create the data mailboxes.
	quaternion_data_mailbox		= xQueueCreate(1U, sizeof(slime_network_quaternion_t));
	acceleration_data_mailbox	= xQueueCreate(1U, sizeof(slime_network_acceleration_t));
	battery_level_data_mailbox	= xQueueCreate(1U, sizeof(slime_network_battery_level_t));

	// Check the allocations.
	ESP_GOTO_ON_FALSE(quaternion_data_mailbox		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the quaternion data mailbox.");
	ESP_GOTO_ON_FALSE(acceleration_data_mailbox		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the acceleration data mailbox.");
	ESP_GOTO_ON_FALSE(battery_level_data_mailbox	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create the battery level data mailbox.");

	// Log the progress.
	SLIME_NETWORK_LOG("Creating network context struct.");

	// Create the network context struct.
	network_context = calloc(1U, sizeof(slime_network_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(network_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create network context struct.");

	// Log the progress.
	SLIME_NETWORK_LOG("Filling network context.");

	// Fill the network context.
	network_context->wifi_context				=	wifi_context;
	network_context->network_tick_config		= &	network_context_config->network_tick_config;
	network_context->network_tick_events		=	network_tick_events;
	network_context->quaternion_data_mailbox	=	quaternion_data_mailbox;
	network_context->acceleration_data_mailbox	=	acceleration_data_mailbox;
	network_context->battery_level_data_mailbox	=	battery_level_data_mailbox;

	// Log the progress.
	SLIME_NETWORK_LOG("Creating network tick task.");

	// Create the network tick task.
	ESP_GOTO_ON_FALSE(xTaskCreatePinnedToCore(
		/* pxTaskCode		= */	slime_network_tick_task,
		/* pcName			= */	network_context_config	->network_tick_task_config.task_name,
		/* usStackDepth		= */	network_context_config	->network_tick_task_config.task_stack_depth,
		/* pvParameters		= */	network_context,
		/* uxPriority		= */	network_context_config	->network_tick_task_config.task_priority,
		/* pxCreatedTask	= */ &	network_context			->network_tick_task_handle,
		/* xCoreID			= */	network_context_config	->network_tick_task_config.task_core_id
	) == pdPASS, ESP_ERR_INVALID_STATE, error, TAG, "Failed to create network tick task.");

	// Return the created network context.
	*network_context_out = network_context;

	// Log the progress.
	SLIME_NETWORK_LOG("Network context has been created.");

	return ret;

	error:

	// Log the error.
	SLIME_NETWORK_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_NETWORK_LOG("Cleaning up resources.");

	if (network_context)			free				(network_context);				// Cleanup the network context struct.
	if (battery_level_data_mailbox)	vQueueDelete		(battery_level_data_mailbox);	// Cleanup the battery level data mailbox.
	if (acceleration_data_mailbox)	vQueueDelete		(acceleration_data_mailbox);	// Cleanup the acceleration data mailbox.
	if (quaternion_data_mailbox)	vQueueDelete		(quaternion_data_mailbox);		// Cleanup the quaternion data mailbox.
	if (network_tick_events)		vEventGroupDelete	(network_tick_events);			// Cleanup the network tick events.

	return ret;
}

esp_err_t slime_network_context_del(slime_network_context_t* network_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(network_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_network_context_t handle provided when releasing network context.");

	// Log the progress.
	SLIME_NETWORK_LOG("Releasing network context.");
	SLIME_NETWORK_LOG("Stopping network tick task.");

	// Mark that network tick task should stop.
	xEventGroupSetBits(network_context_in->network_tick_events, SLIME_NETWORK_STOP);

	// Wait until the transmission task is stopped.
	xEventGroupWaitBits(
		/* xEventGroup		= */ network_context_in->network_tick_events,
		/* uxBitsToWaitFor	= */ SLIME_NETWORK_STOPPED,
		/* xClearOnExit		= */ pdFALSE,
		/* xWaitForAllBits	= */ pdFALSE,
		/* xTicksToWait		= */ portMAX_DELAY
	);

	// Log the progress.
	SLIME_NETWORK_LOG("Deleting network tick task.");

	// Delete the network tick task and its event group.
	vTaskDelete			(network_context_in->network_tick_task_handle);
	vEventGroupDelete	(network_context_in->network_tick_events);

	// Log the progress.
	SLIME_NETWORK_LOG("Releasing data mailboxes.");

	// Delete all data mailboxes.
	vQueueDelete(network_context_in->quaternion_data_mailbox);
	vQueueDelete(network_context_in->acceleration_data_mailbox);
	vQueueDelete(network_context_in->battery_level_data_mailbox);

	// Log the progress.
	SLIME_NETWORK_LOG("Detaching all fields of network context.");

	// Detach all fields.
	network_context_in->wifi_context				= NULL;
	network_context_in->network_tick_config			= NULL;
	network_context_in->network_tick_events			= NULL;
	network_context_in->network_tick_task_handle	= NULL;
	network_context_in->quaternion_data_mailbox		= NULL;
	network_context_in->acceleration_data_mailbox	= NULL;
	network_context_in->battery_level_data_mailbox	= NULL;

	// Log the progress.
	SLIME_NETWORK_LOG("Releasing network context struct.");

	// Free the buffer context struct.
	free(network_context_in);

	// Log the progress.
	SLIME_NETWORK_LOG("network context has been released.");

	return ESP_OK;
}