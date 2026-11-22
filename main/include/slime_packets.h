#ifndef SLIME_PACKETS_H
#define SLIME_PACKETS_H

#include "math.h"
#include "stddef.h"
#include "stdint.h"



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#pragma pack(push, 1)

/**
 * @brief The client-broadcasted handshake packet payload header struct.
 */
typedef struct {
	uint32_t	board;						/*!< The board of the tracker. */
	uint32_t	sensor_type;				/*!< The sensor type ID of the tracker for backward-compatibility, should always be "0" (Unknown). */
	uint32_t	mcu;						/*!< The MCU of the tracker. */
	uint32_t	unused_0;					/*!< Backwards compatibility, unused IMU data in Big-Endian uint32_t form. */
	uint32_t	unused_1;					/*!< Backwards compatibility, unused IMU data in Big-Endian uint32_t form. */
	uint32_t	unused_2;					/*!< Backwards compatibility, unused IMU data in Big-Endian uint32_t form. */
	uint32_t	protocol_version;			/*!< The protocol version of the firmware in Big-Endian uint32_t form. */
	uint8_t		firmware_version_length;	/*!< The length of the firmware version string. */
	uint8_t		firmware_version[5U];		/*!< The firmware version string of the tracker. */
	uint8_t		mac_address		[6U];		/*!< The MAC address of the tracker. */
	uint8_t		tracker_type;				/*!< The type ID of the tracker, should always be "0" (TRACKER_TYPE_SVR_ROTATION). */
} slime_packet_bc_handshake_header_t;

/**
 * @brief The bidirectional sensor info packet payload struct.
 */
typedef struct {
	uint8_t sensor_id;		/*!< The sensor ID of the sensor info. */
	uint8_t sensor_status;	/*!< The sensor status of the sensor, should always be "1" (SENSOR_OK). */
	uint8_t sensor_type;	/*!< The sensor type ID of the sensor, should always be "0" (Unknown). */
	struct {
		uint8_t										: 8; /*!< Padding. */
		uint8_t magnetometer_enabled				: 1; /*!< True if the magnetometer is enabled. */
		uint8_t magnetometer_supported				: 1; /*!< True if the magnetometer is supported. */
		uint8_t calibration_enabled					: 1; /*!< True if the calibration is enabled, should always be "true". */
		uint8_t calibration_supported				: 1; /*!< True if the calibration is supported, should always be "true". */
		uint8_t temp_gradient_calibration_enabled	: 1; /*!< True if the temperature gradient gyroscope calibration is enabled, should always be "false". */
		uint8_t temp_gradient_calibration_supported	: 1; /*!< True if the temperature gradient gyroscope calibration is supported, should always be "false". */
		uint8_t										: 2; /*!< Padding. */
	} sensor_config_data;
	uint8_t		has_completed_rest_calibration;	/*!< True if rest detection is completed. */
	uint8_t		sensor_position;				/*!< The position of the sensor. */
	uint8_t		sensor_data_type;				/*!< The data type of the sensor, should always be "0" (SENSOR_DATATYPE_ROTATION). */
	uint32_t	tps_counter_averaged_tps;		/*!< The averaged TPS of the sensor loop in Hz in Big-Endian float form. */
	uint32_t	data_counter_averaged_tps;		/*!< The averaged TPS of the rotation data in Hz in Big-Endian float form. */
} slime_packet_bid_sensor_info_t;

/**
 * @brief The client-to-server rotation data packet payload struct.
 */
typedef struct {
	uint8_t		sensor_id;		/*!< The sensor ID of the rotation data. */
	uint8_t		data_type;		/*!< The type of the rotation data, should always be "1" (NORMAL). */
	uint32_t	quaternion_x;	/*!< The X of the quaternion of the rotation data in Big-Endian float form. */
	uint32_t	quaternion_y;	/*!< The Y of the quaternion of the rotation data in Big-Endian float form. */
	uint32_t	quaternion_z;	/*!< The Z of the quaternion of the rotation data in Big-Endian float form. */
	uint32_t	quaternion_w;	/*!< The W of the quaternion of the rotation data in Big-Endian float form. */
	uint8_t		accuracy_info;	/*!< The accuracy info of the rotation data, should always be "0". */
} slime_packet_c2s_rotation_data_t;

/**
 * @brief The client-to-server acceleration packet payload struct.
 */
typedef struct {
	uint32_t	acceleration_x;	/*!< The X of the acceleration data in Big-Endian float form. */
	uint32_t	acceleration_y;	/*!< The Y of the acceleration data in Big-Endian float form. */
	uint32_t	acceleration_z;	/*!< The Z of the acceleration data in Big-Endian float form. */
	uint8_t		sensor_id;		/*!< The sensor ID of the acceleration data. */
} slime_packet_c2s_acceleration_data_t;

/**
 * @brief The client-to-server battery level packet payload struct.
 */
typedef struct {
	uint32_t battery_voltage;		/*!< Current battery voltage in Big-Endian float form. */
	uint32_t battery_percentage;	/*!< Current battery percentage in Big-Endian float form. */
} slime_packet_c2s_battery_level_t;

/**
 * @brief The client-to-server signal strength packet payload struct.
 */
typedef struct {
	uint8_t sensor_id;			/*!< The sensor ID of the signal strength. */
	uint8_t signal_strength;	/*!< The signal strength. */
} slime_packet_c2s_signal_strength_t;

/**
 * @brief The client-to-server configuration change acknowledgement packet payload struct.
 */
typedef struct {
	uint8_t		sensor_id;			/*!< The sensor ID that acknowledge the change. */
	uint16_t	configuration_type;	/*!< The type of the configuration that changed in Big-Endian uint16_t form. */
} slime_packet_c2s_acknowledge_config_change_t;

/**
 * @brief The server-to-client set configuration state packet payload struct.
 */
typedef struct {
	uint8_t		sensor_id;				/*!< The sensor ID that set the configuration. */
	uint16_t	configuration_type;		/*!< The type of the configuration to set in Big-Endian uint16_t form. */
	uint8_t		configuration_state;	/*!< The state to set to the configuration. */
} slime_packet_s2c_set_config_t;

/**
 * @brief The client-to-server firmware feature packet payload struct.
 */
typedef struct {
	uint8_t	remote_command_supported	: 1; /*!< Copied from server, what is it?? */
	uint8_t	example_feature_supported	: 1; /*!< Example feature flag. */
	uint8_t	sensor_config_supported		: 1; /*!< True if configuring sensors is supported. */
	uint8_t								: 5; /*!< Padding. */
} slime_packet_c2s_firmware_feature_t;

/**
 * @brief The server-to-client server feature packet payload struct.
 */
typedef struct {
	uint8_t protocol_bundle_supported			: 1; /*!< Server can parse bundle packets: `PACKET_BUNDLE` = 100 (0x64). */
	uint8_t protocol_bundle_compat_supported	: 1; /*!< Server can parse bundle packets with compact headers and packed IMU rotation/acceleration frames. */
	uint8_t										: 6; /*!< Padding. */
} slime_packet_s2c_server_feature_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_PACKETS_H
