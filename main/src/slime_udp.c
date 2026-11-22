#include "esp_log.h"
#include "slime_udp.h"

/**
 * @brief The log tag of the Slime UDP.
 */
static const char* TAG = "slime_udp";

/**
 * @brief The definition of the debug logging function of the Slime UDP.
 */
#ifdef CONFIG_SLIME_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_UDP_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_UDP_LOG(format, ...)
#endif // CONFIG_SLIME_DEBUG_LOGGING

/**
 * @brief The type definition alias of the general socket address struct.
 */
typedef struct sockaddr address_t;

/**
 * @brief The type definition alias of the IPv4 socket address struct.
 */
typedef struct sockaddr_in address_ip4_t;

/**
 * @brief The type definition alias of the socket address length type.
 */
typedef socklen_t address_len_t;

/**
 * @brief The length of the native IPv4 socket address struct.
 */
static const size_t address_length = sizeof(address_ip4_t);

esp_err_t slime_udp_broadcast_packet(const slime_udp_context_t* udp_context, slime_buffer_context_t* send_buffer_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(udp_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_t handle provided when broadcasting packet.");

	// Broadcast the current packet.
	ESP_RETURN_ON_ERROR(slime_udp_send_packet(
		/* udp_context			= */ udp_context,
		/* send_address			= */ udp_context->broadcast_address,
		/* send_buffer_context	= */ send_buffer_context
	), TAG, "Failed to broadcast a packet.");

	return ESP_OK;
}

esp_err_t slime_udp_send_packet(
	const	slime_udp_context_t*		udp_context,
	const	slime_udp_address_t			send_address,
			slime_buffer_context_t*		send_buffer_context
) {
	// We cannot proceed without a context and a buffer to send.
	ESP_RETURN_ON_FALSE(udp_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_t handle provided when sending packet.");
	ESP_RETURN_ON_FALSE(send_buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when sending packet.");

	// Calculate the offset of the packet in the buffer context and the length of the packet.
	const uint32_t packet_offset = send_buffer_context->reader_index;
	const uint32_t packet_length = send_buffer_context->writer_index	- packet_offset;
	const uint8_t* packet_buffer = send_buffer_context->buffer			+ packet_offset;

	// Skip if the length of the packet is 0.
	if (packet_length == 0U) {
		return ESP_OK;
	}

	// Build the native broadcast socket address info.
	const address_ip4_t address_struct = {
		.sin_family			= AF_INET,
		.sin_addr.s_addr	= send_address.addr,
		.sin_port			= send_address.port,
	};

	// Send the current packet payload to the server, ignore the EAGAIN/EWOULDBLOCK.
	ESP_RETURN_ON_FALSE(sendto(
		/* socket_handle		= */ udp_context->socket_handle,
		/* packet_buffer		= */ packet_buffer,
		/* packet_length		= */ packet_length,
		/* packet_flags			= */ 0,
		/* send_address			= */ (address_t*) &	address_struct,
		/* send_address_length	= */				address_length
	) >= 0 || errno == EAGAIN, errno, TAG, "Failed to send a packet.");

	// Mark the packet read in the packet buffer context.
	ESP_RETURN_ON_ERROR(slime_buffer_read_skip(send_buffer_context, packet_length), TAG, "Failed to skip bytes.");

	return ESP_OK;
}

esp_err_t slime_udp_receive_packet(
	const	slime_udp_context_t*		udp_context,
			slime_buffer_context_t*		received_buffer_context,
			slime_udp_address_t*		received_address
) {
	// We cannot proceed without a context and a buffer to hold the received packet.
	ESP_RETURN_ON_FALSE(udp_context				!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_t handle provided when receiving packet.");
	ESP_RETURN_ON_FALSE(received_buffer_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_buffer_context_t handle provided when receiving packet.");

	// Calculate the offset of the packet in the buffer context and the length of the packet.
	const	uint32_t packet_offset = received_buffer_context->writer_index;
	const	uint32_t packet_length = received_buffer_context->length - packet_offset;
			uint8_t* packet_buffer = received_buffer_context->buffer + packet_offset;

	// We cannot proceed if the length of available space in buffer context is 0.
	ESP_RETURN_ON_FALSE(packet_length != 0U, ESP_ERR_NO_MEM, TAG, "No space in buffer context when receiving packet.");

	// Reserve space for the received address.
	address_ip4_t received_address_struct = {0};
	address_len_t received_address_length = address_length;

	// Reserve space for the length of the received packet.
	int32_t received_packet_length = 0;

	// Try receiving a packet from the UDP socket, ignore the EAGAIN/EWOULDBLOCK.
	ESP_RETURN_ON_FALSE((received_packet_length = recvfrom(
		/* socket			= */ udp_context->socket_handle,
		/* buffer			= */ packet_buffer,
		/* length			= */ packet_length,
		/* flags			= */ 0,
		/* address			= */ (address_t*)	&received_address_struct,
		/* address_length	= */				&received_address_length
	)) >= 0 || errno == EAGAIN, errno, TAG, "Failed to receive a packet.");

	// Skip if no packet received.
	if (received_packet_length <= 0) {
		return ESP_OK;
	}

	// Update received address.
	if (received_address) {
		received_address->addr = received_address_struct.sin_addr.s_addr;
		received_address->port = received_address_struct.sin_port;
	}

	// Mark the packet written in the packet buffer context.
	ESP_RETURN_ON_ERROR(slime_buffer_write_skip(received_buffer_context, (uint32_t) received_packet_length), TAG, "Failed to skip bytes.");

	return ESP_OK;
}

esp_err_t slime_udp_context_new(slime_udp_context_t** udp_context_out, const slime_udp_context_config_t* udp_context_config) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration and a handle to receive the created UDP context.
	ESP_RETURN_ON_FALSE(udp_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_t handle provided when creating UDP context.");
	ESP_RETURN_ON_FALSE(udp_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_config_t handle provided when creating UDP context.");

	// Log the progress.
	SLIME_UDP_LOG("Creating UDP context.");
	SLIME_UDP_LOG("Reserving handles of UDP context.");

	// Reserve handles of UDP context.
	slime_udp_context_t*		udp_context		= NULL;
	slime_udp_socket_handle_t	socket_handle	= 0U;

	// Log the progress.
	SLIME_UDP_LOG("Creating UDP socket handle.");

	// Create the IPv4 UDP socket handle.
	socket_handle = socket(
		/* domain	= */ AF_INET,
		/* type		= */ SOCK_DGRAM,
		/* protocol	= */ IPPROTO_IP
	);

	// Check the allocation
	ESP_RETURN_ON_FALSE(socket_handle != 0, errno, TAG, "Failed to create socket handle.");

	// Log the progress.
	SLIME_UDP_LOG("Configuring UDP socket.");

	// The option used in setsockopt.
	const uint32_t option = 1U;

	// Permit the socket to send and receive broadcast messages and allow the socket to reuse local address.
	ESP_GOTO_ON_FALSE(setsockopt(socket_handle, SOL_SOCKET, SO_BROADCAST, &option, sizeof(option)) == 0, errno, error, TAG, "Failed to permit socket to send and to receive broadcast messages");
	ESP_GOTO_ON_FALSE(setsockopt(socket_handle, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == 0, errno, error, TAG, "Failed to allow socket to reuse local address.");

	// Set the socket to non-blocking mode.
	ESP_GOTO_ON_FALSE(fcntl(
		/* handle	= */ socket_handle,
		/* command	= */ F_SETFL,
		/* ...		= */ O_NONBLOCK
	) >= 0, errno, error, TAG, "Failed to set the socket to non-blocking mode.");

	// Log the progress.
	SLIME_UDP_LOG("Starting UDP socket.");
	SLIME_UDP_LOG("Constructing bind socket address.");

	// Build the local bind address info.
	const address_ip4_t address_struct = {
		.sin_family			= AF_INET,
		.sin_addr.s_addr	= udp_context_config->bind_address.addr,
		.sin_port			= udp_context_config->bind_address.port,
	};

	// Log the progress.
	SLIME_UDP_LOG("Binding UDP socket to bind socket address.");

	// Bind the socket to local address.
	ESP_GOTO_ON_FALSE(bind(
		/* socket	= */				socket_handle,
		/* address	= */ (address_t*) &	address_struct,
		/* length	= */				address_length
	) == 0U, errno, error, TAG, "Failed to bind socket.");

	// Log the progress.
	SLIME_UDP_LOG("Creating UDP context struct.");

	// Create the UDP context struct.
	udp_context = calloc(1U, sizeof(slime_udp_context_t));

	// Check the allocations.
	ESP_GOTO_ON_FALSE(udp_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create UDP context struct.");

	// Log the progress.
	SLIME_UDP_LOG("Finalizing UDP context.");

	// Fill the UDP context.
	udp_context->socket_handle			= socket_handle;
	udp_context->bind_address			= udp_context_config->bind_address;
	udp_context->broadcast_address.port	= udp_context_config->bind_address.port;
	udp_context->broadcast_address.addr	= INADDR_BROADCAST;

	// Return the created UDP context.
	*udp_context_out = udp_context;

	// Log the progress.
	SLIME_UDP_LOG("UDP context has been created.");

	return ret;

	// Error occurred.
	error:

	// Reserve space for error string.
	char error_string[64U];

	// Log the error.
	SLIME_UDP_LOG("Error occurred: %s", esp_err_to_name_r(ret, error_string, 64U));
	SLIME_UDP_LOG("Cleaning up resources.");

	// Cleanup the UDP context struct.
	if (udp_context) {
		free(udp_context);
	}

	// Cleanup the UDP socket handle.
	close(socket_handle);

	return ret;
}

esp_err_t slime_udp_context_del(slime_udp_context_t* udp_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(udp_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_udp_context_t handle provided when releasing UDP context.");

	// Log the progress.
	SLIME_UDP_LOG("Releasing UDP context.");
	SLIME_UDP_LOG("Releasing UDP socket handle.");

	// Close the UDP socket handle.
	close(udp_context_in->socket_handle);

	SLIME_UDP_LOG("Detaching all fields of UDP context.");

	// Detach all fields.
	udp_context_in->socket_handle			= 0xFFFFFFFFU;
	udp_context_in->bind_address.addr		= 0xFFFFFFFFU;
	udp_context_in->bind_address.port		= 0xFFFFU;
	udp_context_in->broadcast_address.addr	= 0xFFFFFFFFU;
	udp_context_in->broadcast_address.port	= 0xFFFFU;

	// Log the progress.
	SLIME_UDP_LOG("Releasing UDP context struct.");

	// Free the UDP context struct.
	free(udp_context_in);

	return ESP_OK;
}