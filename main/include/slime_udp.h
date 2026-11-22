#ifndef SLIME_UDP_H
#define SLIME_UDP_H

#include "lwip/sockets.h"
#include "stdint.h"
#include "esp_check.h"
#include "slime_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The type definition alias of the socket handle.
 */
typedef uint32_t slime_udp_socket_handle_t;

/**
 * @brief The IPv4 socket address struct.
 */
typedef struct {
	in_addr_t addr; /*!< The IPv4 address part of the socket address. */
	in_port_t port; /*!< The port part of the socket address, in Big-Endian. */
} slime_udp_address_t;

typedef struct {
	slime_udp_address_t bind_address; /*!< The bind socket address of the UDP context. */
} slime_udp_context_config_t;

/**
 * @brief The UDP context struct.
 */
typedef struct {
	slime_udp_socket_handle_t	socket_handle;		/*!< The handle of the socket. */
	slime_udp_address_t			bind_address;		/*!< The bind socket address of the socket. */
	slime_udp_address_t			broadcast_address;	/*!< The broadcast socket address of the socket. */
} slime_udp_context_t;

/**
 * @brief						Broadcast a packet to the network.
 * @param udp_context			The UDP context to broadcast the packet.
 * @param send_buffer_context	The buffer context of the packet to be broadcasted.
 */
esp_err_t slime_udp_broadcast_packet(const slime_udp_context_t* udp_context, slime_buffer_context_t* send_buffer_context);

/**
 * @brief						Send a packet to given socket address.
 * @param udp_context			The UDP context to send the packet.
 * @param send_address			The socket address the packet to be sent to.
 * @param send_buffer_context	The buffer context of the packet to be sent.
 */
esp_err_t slime_udp_send_packet(
	const	slime_udp_context_t*	udp_context,
			slime_udp_address_t		send_address,
			slime_buffer_context_t*	send_buffer_context
);

/**
 * @brief							Try receiving a packet.
 * @param udp_context				The UDP context to receive the packet.
 * @param received_buffer_context	The buffer context to hold the received packet.
 * @param received_address			The handle to receive the socket address of the packet, can be NULL.
 */
esp_err_t slime_udp_receive_packet(
	const	slime_udp_context_t*		udp_context,
			slime_buffer_context_t*		received_buffer_context,
			slime_udp_address_t*		received_address
);

/**
 * @brief						Create the UDP context.
 * @param udp_context_out		The handle to receive the created UDP context.
 * @param udp_context_config	The configuration of the UDP context.
 * @return						The status of the creation.
 */
esp_err_t slime_udp_context_new(slime_udp_context_t** udp_context_out, const slime_udp_context_config_t* udp_context_config);

/**
 * @brief					Release the UDP context.
 * @param udp_context_in	The UDP context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_udp_context_del(slime_udp_context_t* udp_context_in);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_UDP_H
