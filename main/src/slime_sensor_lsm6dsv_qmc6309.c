#include "stdlib.h"
#include "stdint.h"
#include "sys/cdefs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_bit_defs.h"
#include "esp_log.h"
#include "slime_sensor_lsm6dsv_qmc6309.h"

/**
 * @brief The flag of gyroscope data presence in the FIFO data buffer of LSM6DSV+QMC6309 sensor context.
 */
#define SENSOR_FIFO_GYROSCOPE BIT0

/**
 * @brief The flag of timestamp data presence in the FIFO data buffer of LSM6DSV+QMC6309 sensor context.
 */
#define SENSOR_FIFO_TIMESTAMP BIT1

/**
 * @brief The flag of accelerometer data presence in the FIFO data buffer of LSM6DSV+QMC6309 sensor context.
 */
#define SENSOR_FIFO_ACCELEROMETER BIT2

/**
 * @brief The flag of magnetometer data presence in the FIFO data buffer of LSM6DSV+QMC6309 sensor context.
 */
#define SENSOR_FIFO_MAGNETOMETER BIT3

/**
 * @brief		Pack 2 uint8_t into an int16_t.
 * @param msb	The higher 8-bit (MSByte) of the int16_t.
 * @param lsb	The lower 8-bit (LSByte) of the int16_t.
 */
#define SENSOR_PACK_INT16(msb, lsb) ((int16_t) ((((uint16_t) (msb)) << 8U) | (lsb)))

/**
 * @brief		Pack 4 uint8_t into an uint32_t.
 * @param b24	The [31:24] of the uint32_t.
 * @param b16	The [23:16] of the uint32_t.
 * @param b08	The [15:8] of the uint32_t.
 * @param b00	The [7:0] of the uint32_t.
 */
#define SENSOR_PACK_UINT32(b24, b16, b08, b00) ((uint32_t) ((((uint32_t) (b24)) << 24U) | (((uint32_t) (b16)) << 16U) | (((uint32_t) (b08)) << 8U) | (b00)))

/**
 * @brief The log tag of the Slime LSM6DSV+QMC6309 Sensor.
 */
static const char* TAG = "slime_sensor_lsm6dsv_qmc6309";

/**
 * @brief The definition of the debug logging function of the Slime LSM6DSV+QMC6309 Sensor
 */
#ifdef CONFIG_SLIME_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_LSM6DSV_QMC6309_SENSOR_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_LSM6DSV_QMC6309_SENSOR_LOG(format, ...)
#endif // CONFIG_SLIME_DEBUG_LOGGING

/**
 * @brief The FIFO data buffer struct of the LSM6DSV+QMC6309 sensor context.
 */
typedef struct {
	int8_t	flags;						/*!< The presence flags of data in the FIFO data buffer. */
	int16_t	count;						/*!< The FIFO count of data in this FIFO data buffer. */
	int32_t	timestamp_buffer;			/*!< The raw timestamp LSB buffer of the FIFO data buffer. */
	int16_t	gyroscope_buffer	[3];	/*!< The raw gyroscope LSB buffer of the FIFO data buffer. */
	int16_t	accelerometer_buffer[3];	/*!< The raw accelerometer LSB buffer of the FIFO data buffer. */
	int16_t	magnetometer_buffer	[3];	/*!< The raw magnetometer LSB buffer of the FIFO data buffer. */
} slime_lsm6dsv_qmc6309_sensor_fifo_data_buffer_t;

/**
 * @brief The LSM6DSV+QMC6309 sensor context struct.
 */
typedef struct {
	slime_sensor_context_t								base;									/*!< The bases sensor context struct of the sensor context. */
	stmdev_ctx_t										lsm6dsv_context;						/*!< The LSM6DSV device context of the sensor context. */
	qmc_context_t										qmc6309_context;						/*!< The QMC6309 device context of the sensor context. */
	float_t												lsm6dsv_timestamp_sensitivity;			/*!< The timestamp sensitivity of the LSM6DSV of the sensor context. */
	float_t												lsm6dsv_gyroscope_sensitivity;			/*!< The gyroscope sensitivity of the LSM6DSV of the sensor context. */
	float_t												lsm6dsv_accelerometer_sensitivity;		/*!< The accelerometer sensitivity of the LSM6DSV of the sensor context. */
	float_t												qmc6309_magnetometer_sensitivity;		/*!< The magnetometer sensitivity of the QMC6309 of the sensor context. */
	float_t												lsm6dsv_gyroscope_sample_time;			/*!< The sample time of the gyroscope of LSM6DSV in milliseconds of the sensor context. */
	float_t												lsm6dsv_accelerometer_sample_time;		/*!< The sample time of the accelerometer of LSM6DSV in milliseconds of the sensor context. */
	float_t												qmc6309_magnetometer_sample_time;		/*!< The sample time of the magnetometer of QMC6309 in milliseconds of the sensor context. */
	slime_lsm6dsv_qmc6309_sensor_fifo_data_buffer_t*	fifo_data_buffer;						/*!< The FIFO data buffer for holding data of incoming FIFO words of same FIFO count in order to reorder them into fixed callback order when flushing. */
	uint8_t*											fifo_word_buffer;						/*!< The FIFO word buffer for fast batched FIFO word polling in order to avoid I2C transition costs. */
	uint32_t											fifo_timestamp_last_value;				/*!< The value of last recorded timestamp from FIFO in LSBs. */
	uint8_t												fifo_timestamp_last_valid;				/*!< The state of last recorded timestamp from FIFO, true if the value is valid for calculating delta time. */
	uint8_t												fifo_magnetometer_ready;				/*!< The data-ready state of the magnetometer FIFO data. */
	slime_sensor_gyroscope_callback_t					gyroscope_callback;						/*!< The user-defined gyroscope callback function handle. */
	slime_sensor_accelerometer_callback_t				accelerometer_callback;					/*!< The user-defined accelerometer callback function handle. */
	slime_sensor_magnetometer_callback_t				magnetometer_callback;					/*!< The user-defined magnetometer callback function handle. */
	slime_sensor_timestamp_callback_t					timestamp_callback;						/*!< The user-defined timestamp callback function handle. */
	void*												callbacks_user_context;					/*!< The custom user-defined context handle of the callbacks. */
} slime_lsm6dsv_qmc6309_sensor_context_t;

/**
 * @brief		The magnetometer status register read configuration of sensor hub slave 0 of LSM6DSV.
 * @attention	Set the status register read before the output register read to ensure the output data is ready before
 *				reading.
 */
static lsm6dsv_sh_cfg_read_t sensor_hub_slave_0_read_config = {
	.slv_add	= QMC6309_I2C_ADDRESS << 1,	/*!< The 8-bit read address format of the I2C device to read. */
	.slv_subadd	= QMC6309_STATUS_1,			/*!< The address of the STATUS1 register to be read. */
	.slv_len	= 1							/*!< We need only the STATUS1 register, so 1. */
};

/**
 * @brief The magnetometer output registers batch read of sensor hub slave 1 of LSM6DSV.
 */
static lsm6dsv_sh_cfg_read_t sensor_hub_slave_1_read_config = {
	.slv_add	= QMC6309_I2C_ADDRESS << 1,	/*!< The 8-bit read address format of the I2C device to read. */
	.slv_subadd	= QMC6309_OUT_X_L,			/*!< The start address of the 6 output registers to be read. */
	.slv_len	= 6							/*!< We need to read ALL 6 output registers. */
};

/**
 * @brief				Delay in milliseconds.
 * @param milliseconds	The time to delay in milliseconds.
 */
static void delay_milliseconds(const uint32_t milliseconds) {
	vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

/**
 * @brief					Register callbacks.
 * @param sensor_context	The LSM6DSV+QMC6309 sensor context to register callbacks into.
 * @param sensor_callbacks	The handle to the configuration of callbacks.
 * @param user_context		The custom user-defined handle, passed directly to callback's user_context。
 * @return					The status of registering callback.
 */
static esp_err_t slime_lsm6dsv_qmc6309_sensor_register_callbacks(
			slime_sensor_context_t*				sensor_context,
	const	slime_sensor_callbacks_config_t*	sensor_callbacks,
			void*								user_context
) {
	// We cannot proceed without a context and a callback configuration.
	ESP_RETURN_ON_FALSE(sensor_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when performing registering callbacks.");
	ESP_RETURN_ON_FALSE(sensor_callbacks	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_callbacks_config_t handle provided when performing registering callbacks.");

	// Get the container LSM6DSV+QMC6309 sensor context handle of the base sensor context handle.
	slime_lsm6dsv_qmc6309_sensor_context_t* lsm6dsv_qmc6309_sensor_context = __containerof(
		/* value		= */ sensor_context,
		/* container	= */ slime_lsm6dsv_qmc6309_sensor_context_t,
		/* field_offset	= */ base
	);

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" is registering callbacks.", sensor_context->name);

	// Log the registered callback function handles.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Callback function handle of timestamp: 0x%"		PRIXPTR ".", (uintptr_t) sensor_callbacks->timestamp_callback);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Callback function handle of gyroscope: 0x%"		PRIXPTR ".", (uintptr_t) sensor_callbacks->gyroscope_callback);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Callback function handle of accelerometer: 0x%"	PRIXPTR ".", (uintptr_t) sensor_callbacks->accelerometer_callback);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Callback function handle of magnetometer: 0x%"	PRIXPTR ".", (uintptr_t) sensor_callbacks->magnetometer_callback);

	// Log the user-defined context handle.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Custom user-defined context handle: 0x%" PRIXPTR ".", (uintptr_t) user_context);

	// Register the callbacks and the user context into the LSM6DSV+QMC6309 sensor context.
	lsm6dsv_qmc6309_sensor_context->timestamp_callback		= sensor_callbacks->timestamp_callback;
	lsm6dsv_qmc6309_sensor_context->gyroscope_callback		= sensor_callbacks->gyroscope_callback;
	lsm6dsv_qmc6309_sensor_context->accelerometer_callback	= sensor_callbacks->accelerometer_callback;
	lsm6dsv_qmc6309_sensor_context->magnetometer_callback	= sensor_callbacks->magnetometer_callback;
	lsm6dsv_qmc6309_sensor_context->callbacks_user_context	= user_context;

	return ESP_OK;
}

/**
 * @brief								Get the sample times of the LSM6DSV and QMC6309.
 * @param sensor_context				The LSM6DSV+QMC6309 sensor context to get the sample times.
 * @param gyroscope_sample_time_ms		The handle to receive the gyroscope sample time in milliseconds.
 * @param accelerometer_sample_time_ms	The handle to receive the accelerometer sample time in milliseconds.
 * @param magnetometer_sample_time_ms	The handle to receive the magnetometer sample time in milliseconds.
 * @return								The status of getting sample times.
 */
esp_err_t slime_lsm6dsv_qmc6309_sensor_get_sample_time(
	const	slime_sensor_context_t*	sensor_context,
			float_t*				gyroscope_sample_time_ms,
			float_t*				accelerometer_sample_time_ms,
			float_t*				magnetometer_sample_time_ms
) {
	// We cannot proceed without a context and handles to receive the sample times.
	ESP_RETURN_ON_FALSE(sensor_context					!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when performing getting sample times.");
	ESP_RETURN_ON_FALSE(gyroscope_sample_time_ms		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the gyroscope sample time when performing getting sample times.");
	ESP_RETURN_ON_FALSE(accelerometer_sample_time_ms	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the accelerometer sample time when performing getting sample times.");
	ESP_RETURN_ON_FALSE(magnetometer_sample_time_ms		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No handle provided to received the magnetometer sample time when performing getting sample times.");

	// Get the container LSM6DSV+QMC6309 sensor context handle of the base sensor context handle.
	const slime_lsm6dsv_qmc6309_sensor_context_t* lsm6dsv_qmc6309_sensor_context = __containerof(
		/* value		= */ sensor_context,
		/* container	= */ slime_lsm6dsv_qmc6309_sensor_context_t,
		/* field_offset	= */ base
	);

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" is getting sample times.", sensor_context->name);

	// Get and return the sample times.
	*gyroscope_sample_time_ms		= lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sample_time;
	*accelerometer_sample_time_ms	= lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sample_time;
	*magnetometer_sample_time_ms	= lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sample_time;

	return ESP_OK;
}

/**
 * @brief					Poll sensor FIFO data.
 * @param sensor_context	The LSM6DSV+QMC6309 sensor context to poll the FIFO data.
 * @return					The status of polling FIFO data.
 */
static slime_sensor_error_t slime_lsm6dsv_qmc6309_sensor_poll_fifo(slime_sensor_context_t* sensor_context) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a context.
	ESP_GOTO_ON_FALSE(sensor_context != NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_sensor_context_t handle provided when performing polling FIFO data.");

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" is polling data from FIFO.", sensor_context->name);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Getting container LSM6DSV+QMC6309 sensor context handle from the base sensor context.");

	// Get the container LSM6DSV+QMC6309 sensor context handle of the base sensor context handle.
	slime_lsm6dsv_qmc6309_sensor_context_t* lsm6dsv_qmc6309_sensor_context = __containerof(
		/* value		= */ sensor_context,
		/* container	= */ slime_lsm6dsv_qmc6309_sensor_context_t,
		/* field_offset	= */ base
	);

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Extracting frequently modifiable states from LSM6DSV+QMC6309 sensor context.");

	// Extract the modifiable FIFO states from the LSM6DSV+QMC6309 sensor context.

	// The frequently modifiable FIFO timestamp delta states of the LSM6DSV+QM6309 sensor context.
	uint32_t	fifo_timestamp_last_value = lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_value;
	uint8_t		fifo_timestamp_last_valid = lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_valid;

	// The frequently modifiable FIFO magnetometer data ready of the LSM6DSV+QM6309 sensor context.
	uint8_t fifo_magnetometer_ready = lsm6dsv_qmc6309_sensor_context->fifo_magnetometer_ready;

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Getting FIFO status from LSM6DSV.");

	// Reserve spaces for FIFO status.
	lsm6dsv_fifo_status_t fifo_status = {0};

	// Get the status of the FIFO from LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_status_get(&lsm6dsv_qmc6309_sensor_context->lsm6dsv_context, &fifo_status), error_imu, TAG, "Failed to get FIFO status from LSM6DSV.");

	// Cache the count of FIFO words from the FIFO status to stack to avoid unaligned 9-bit integer operations.
	uint16_t fifo_word_count = fifo_status.fifo_level;

	// Log the FIFO status.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("%" PRIu16 " pending FIFO words to be read, %s, %s.",
		/* PRIu16	*/ fifo_word_count,
		/* s		*/ fifo_status.fifo_ovr	? "FIFO overflow occurred" : "no FIFO overflow occurs",
		/* s		*/ fifo_status.fifo_th	? "FIFO watermark reached" : "FIFO filling is below watermark"
	);

	// Poll all FIFO words from the FIFO of the LSM6DSV.
	while (fifo_word_count > 0U) {
		// Clamp the count of FIFO words to read.
		const uint32_t fifo_word_count_read = fifo_word_count < 256U ? fifo_word_count : 256U;

		// Log the operation.
		SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Getting %" PRIu32 " FIFO words from LSM6DSV.", fifo_word_count_read);

		// Batched poll the FIFO words from LSM6DSV.
		ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_read_reg(
			/* ctx	= */ &	lsm6dsv_qmc6309_sensor_context->lsm6dsv_context,
			/* reg	= */	LSM6DSV_FIFO_DATA_OUT_TAG,
			/* data	= */	lsm6dsv_qmc6309_sensor_context->fifo_word_buffer,
			/* len	= */	fifo_word_count_read * 7U
		), error_imu, TAG, "Failed to get FIFO words from LSM6DSV.");

		// Decrease the count of pending FIFO words to be read.
		fifo_word_count -= fifo_word_count_read;

		// Log the operation.
		SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Processing all FIFO words polled from LSM6DSV.");

		// Process all polled FIFO words.
		for (uint32_t index = 0; index < fifo_word_count_read; index ++) {
			// Get the index of the FIFO word in the FIFO word buffer.
			const uint32_t fifo_word_index = index * 7U;

			// Reinterpret the first byte of the FIFO word into the FIFO tag struct handle.
			const lsm6dsv_fifo_data_out_tag_t* fifo_word_tag = (lsm6dsv_fifo_data_out_tag_t*) &lsm6dsv_qmc6309_sensor_context->fifo_word_buffer[fifo_word_index];

			// Flush the FIFO count data slot if there is a new incoming FIFO count.
			if (lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->count != fifo_word_tag->tag_cnt) {
				lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->count = fifo_word_tag->tag_cnt; // Update the current FIFO count.

				// Log the operation.
				SLIME_LSM6DSV_QMC6309_SENSOR_LOG("New FIFO count incoming, flushing FIFO words of the old FIFO count in the FIFO buffer.");

				// Get the flags from the FIFO data buffer.
				const uint8_t flags = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags;

				// Flush the gyroscope data if the gyroscope data is in the buffer.
				if (flags & SENSOR_FIFO_GYROSCOPE) {
					// Get the gyroscope buffer from the FIFO data buffer.
					const int16_t* gyroscope_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->gyroscope_buffer;

					// Get the gyroscope sensitivity from the context.
					const float_t lsm6dsv_gyroscope_sensitivity = lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity;

					// Convert the raw gyroscope LSB data in FIFO data buffer into mdps.
					const float_t gyroscope_mdps_enu_x = gyroscope_buffer[0U] * lsm6dsv_gyroscope_sensitivity;
					const float_t gyroscope_mdps_enu_y = gyroscope_buffer[1U] * lsm6dsv_gyroscope_sensitivity;
					const float_t gyroscope_mdps_enu_z = gyroscope_buffer[2U] * lsm6dsv_gyroscope_sensitivity;

					// Log the flushing.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Trying flushing gyroscope data to gyroscope callback function: x=%.2f mdps, y=%.2f mdps, z=%.2f mdps.",
						/* .2f */ gyroscope_mdps_enu_x,
						/* .2f */ gyroscope_mdps_enu_y,
						/* .2f */ gyroscope_mdps_enu_z
					);

					// Flush the data to callback function if it exists.
					if (lsm6dsv_qmc6309_sensor_context->gyroscope_callback) {
						lsm6dsv_qmc6309_sensor_context->gyroscope_callback(
							/* sensor_context		= */ sensor_context,
							/* gyroscope_mdps_enu_x	= */ gyroscope_mdps_enu_x,
							/* gyroscope_mdps_enu_y	= */ gyroscope_mdps_enu_y,
							/* gyroscope_mdps_enu_z	= */ gyroscope_mdps_enu_z,
							/* user_context			= */ lsm6dsv_qmc6309_sensor_context->callbacks_user_context
						);
					} else {
						// Log the skip.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("No gyroscope callback function registered, skip flushing gyroscope data.");
					}
				}

				// Flush the timestamp data if the timestamp data is in the buffer.
				if (flags & SENSOR_FIFO_TIMESTAMP) {
					// Get the raw timestamp LSB data from the timestamp buffer of the FIFO data buffer.
					const uint32_t timestamp_lsb = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->timestamp_buffer;

					// Set the last timestamp to the current timestamp if the value of last timestamp is not valid.
					if (fifo_timestamp_last_valid == false) {
						fifo_timestamp_last_valid = true;
						fifo_timestamp_last_value = timestamp_lsb;
					}

					// Calculate the delta time in seconds.
					const float_t delta_timestamp_seconds = (timestamp_lsb - fifo_timestamp_last_value) * lsm6dsv_qmc6309_sensor_context->lsm6dsv_timestamp_sensitivity;

					// Update the last timestamp
					fifo_timestamp_last_value = timestamp_lsb;

					// Log the flushing.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Trying flushing timestamp data to timestamp callback function: delta=%.6f seconds.", delta_timestamp_seconds);

					// Flush the data to callback function if it exists.
					if (lsm6dsv_qmc6309_sensor_context->timestamp_callback) {
						lsm6dsv_qmc6309_sensor_context->timestamp_callback(
							/* sensor_context			= */ sensor_context,
							/* delta_timestamp_seconds	= */ delta_timestamp_seconds,
							/* user_context				= */ lsm6dsv_qmc6309_sensor_context->callbacks_user_context
						);
					} else {
						// Log the skip.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("No timestamp callback function registered, skip flushing timestamp data.");
					}
				}

				// Flush the accelerometer data if the accelerometer data is in the buffer.
				if (flags & SENSOR_FIFO_ACCELEROMETER) {
					// Get the accelerometer buffer from the FIFO data buffer.
					const int16_t* accelerometer_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->accelerometer_buffer;

					// Get the magnetometer sensitivity from the context.
					const float_t lsm6dsv_accelerometer_sensitivity = lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sensitivity;

					// Convert raw accelerometer LSB data into mg.
					const float_t accelerometer_mg_enu_x = accelerometer_buffer[0U] * lsm6dsv_accelerometer_sensitivity;
					const float_t accelerometer_mg_enu_y = accelerometer_buffer[1U] * lsm6dsv_accelerometer_sensitivity;
					const float_t accelerometer_mg_enu_z = accelerometer_buffer[2U] * lsm6dsv_accelerometer_sensitivity;

					// Log the flushing.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Trying flushing accelerometer data to accelerometer callback function: x=%.2f mg, y=%.2f mg, z=%.2f mg.",
						/* .2f */ accelerometer_mg_enu_x,
						/* .2f */ accelerometer_mg_enu_y,
						/* .2f */ accelerometer_mg_enu_z
					);

					// Flush the data to callback function if it exists.
					if (lsm6dsv_qmc6309_sensor_context->accelerometer_callback) {
						lsm6dsv_qmc6309_sensor_context->accelerometer_callback(
							/* sensor_context			= */ sensor_context,
							/* accelerometer_mg_enu_x	= */ accelerometer_mg_enu_x,
							/* accelerometer_mg_enu_y	= */ accelerometer_mg_enu_y,
							/* accelerometer_mg_enu_z	= */ accelerometer_mg_enu_z,
							/* user_context				= */ lsm6dsv_qmc6309_sensor_context->callbacks_user_context
						);
					} else {
						// Log the skip.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("No accelerometer callback function registered, skip flushing accelerometer data.");
					}
				}

				// Flush the magnetometer data if the magnetometer data is in the buffer.
				if (flags & SENSOR_FIFO_MAGNETOMETER) {
					// Get the magnetometer buffer from the FIFO data buffer.
					const int16_t* magnetometer_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->magnetometer_buffer;

					// Get the magnetometer sensitivity from the context.
					const float_t qmc6309_magnetometer_sensitivity = lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sensitivity;

					// Convert the raw magnetometer LSB data into gauss.
					const float_t magnetometer_gauss_enu_x = magnetometer_buffer[0U] * qmc6309_magnetometer_sensitivity;
					const float_t magnetometer_gauss_enu_y = magnetometer_buffer[1U] * qmc6309_magnetometer_sensitivity;
					const float_t magnetometer_gauss_enu_z = magnetometer_buffer[2U] * qmc6309_magnetometer_sensitivity;

					// Log the flushing.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Trying flushing magnetometer data to magnetometer callback function: x=%.2f gauss, y=%.2f gauss, z=%.2f gauss.",
						/* .2f */ magnetometer_gauss_enu_x,
						/* .2f */ magnetometer_gauss_enu_y,
						/* .2f */ magnetometer_gauss_enu_z
					);

					// Flush the data to callback function if it exists.
					if (lsm6dsv_qmc6309_sensor_context->magnetometer_callback) {
						lsm6dsv_qmc6309_sensor_context->magnetometer_callback(
							/* sensor_context			= */ sensor_context,
							/* magnetometer_gauss_enu_x	= */ magnetometer_gauss_enu_x,
							/* magnetometer_gauss_enu_y	= */ magnetometer_gauss_enu_y,
							/* magnetometer_gauss_enu_z	= */ magnetometer_gauss_enu_z,
							/* user_context				= */ lsm6dsv_qmc6309_sensor_context->callbacks_user_context
						);
					} else {
						// Log the skip.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("No magnetometer callback function registered, skip flushing magnetometer data.");
					}
				}

				// Clear the flag for the new incoming FIFO count.
				lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags = 0U;

				// Log the operation.
				SLIME_LSM6DSV_QMC6309_SENSOR_LOG("FIFO buffer has been flushed.");
			}

			// Alias of the lsm6dsv_qmc6309_sensor_context->fifo_word_buffer
			uint8_t* fifo_word_buffer = lsm6dsv_qmc6309_sensor_context->fifo_word_buffer;

			// Classify the data of the FIFO word by the tag.
			switch (fifo_word_tag->tag_sensor) {
				// Gyroscope data.
				case LSM6DSV_GY_NC_TAG:
				case LSM6DSV_GY_NC_T_1_TAG:
				case LSM6DSV_GY_NC_T_2_TAG:
					// Get the gyroscope buffer from the FIFO data buffer.
					int16_t* gyroscope_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->gyroscope_buffer;

					// Copy the raw gyroscope data to the gyroscope buffer.
					gyroscope_buffer[0U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 2U], fifo_word_buffer[fifo_word_index + 1U]);
					gyroscope_buffer[1U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 4U], fifo_word_buffer[fifo_word_index + 3U]);
					gyroscope_buffer[2U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 6U], fifo_word_buffer[fifo_word_index + 5U]);

					// Mark the gyroscope data present in the FIFO buffer.
					lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags |= SENSOR_FIFO_GYROSCOPE;

					// Log the data.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Got gyroscope FIFO word from LSM6DSV: x=0x%04" PRIX16 ", y=0x%04" PRIX16 ", z=0x%04" PRIX16 ".",
						/* PRIX16 */ gyroscope_buffer[0U],
						/* PRIX16 */ gyroscope_buffer[1U],
						/* PRIX16 */ gyroscope_buffer[2U]
					);
					break;
				// Timestamp data.
				case LSM6DSV_TIMESTAMP_TAG:
					// Copy the raw timestamp data to the timestamp buffer of FIFO data buffer.
					lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->timestamp_buffer = SENSOR_PACK_UINT32(
						/* b24 = */ fifo_word_buffer[fifo_word_index + 4U],
						/* b16 = */ fifo_word_buffer[fifo_word_index + 3U],
						/* b08 = */ fifo_word_buffer[fifo_word_index + 2U],
						/* b00 = */ fifo_word_buffer[fifo_word_index + 1U]
					);

					// Mark the timestamp data present in the FIFO buffer.
					lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags |= SENSOR_FIFO_TIMESTAMP;

					// Log the data.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Got timestamp FIFO word from LSM6DSV: t=0x%08" PRIX32 ".", lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->timestamp_buffer);
					break;
				// Accelerometer data.
				case LSM6DSV_XL_NC_TAG:
				case LSM6DSV_XL_NC_T_1_TAG:
				case LSM6DSV_XL_NC_T_2_TAG:
					// Get the accelerometer buffer from the FIFO data buffer.
					int16_t* accelerometer_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->accelerometer_buffer;

					// Copy the raw accelerometer data into the FIFO count buffer.
					accelerometer_buffer[0U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 2U], fifo_word_buffer[fifo_word_index + 1U]);
					accelerometer_buffer[1U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 4U], fifo_word_buffer[fifo_word_index + 3U]);
					accelerometer_buffer[2U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 6U], fifo_word_buffer[fifo_word_index + 5U]);

					// Mark the accelerometer data present in the FIFO count buffer.
					lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags |= SENSOR_FIFO_ACCELEROMETER;

					// Log the data.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Got accelerometer FIFO word from LSM6DSV: x=0x%04" PRIX16 ", y=0x%04" PRIX16 ", z=0x%04" PRIX16 ".",
						/* PRIX16 */ accelerometer_buffer[0U],
						/* PRIX16 */ accelerometer_buffer[1U],
						/* PRIX16 */ accelerometer_buffer[2U]
					);
					break;
				// Magnetometer STATUS1 data.
				case LSM6DSV_SENSORHUB_SLAVE0_TAG:
					// Get the STATUS1 register from the FIFO word data.
					const qmc6309_status_1_t* qmc6309_status_1 = (qmc6309_status_1_t *) &fifo_word_buffer[fifo_word_index + 1U];

					// Get the data ready bit and data overflow bit from the STATUS1 register value.
					const uint8_t qmc6309_drdy_bit = qmc6309_status_1->drdy_bit;
					const uint8_t qmc6309_ovfl_bit = qmc6309_status_1->ovfl_bit;

					// Log the data.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Got magnetometer STATUS1 FIFO word from LSM6DSV: drdy=%s, ovfl=%s.",
						/* s */ qmc6309_drdy_bit ? "true" : "false",
						/* s */ qmc6309_ovfl_bit ? "true" : "false"
					);

					// Check if the data is ready and no overflow occurred.
					if (	qmc6309_drdy_bit != 0
						&&	qmc6309_ovfl_bit == 0
					) {
						// Log the operation.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Magnetometer FIFO data ready.");

						// Mark the magnetometer data ready.
						fifo_magnetometer_ready = true;
					}
					break;
				// Magnetometer output data.
				case LSM6DSV_SENSORHUB_SLAVE1_TAG:
					// Only add the magnetometer data into the FIFO count buffer when the data is ready.
					if (fifo_magnetometer_ready) {
						fifo_magnetometer_ready = false; // Reset the data ready state.

						// Get the magnetometer buffer from the FIFO data buffer.
						int16_t* magnetometer_buffer = lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->magnetometer_buffer;

						// Add the raw magnetometer data to the FIFO count buffer.
						magnetometer_buffer[0U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 2U], fifo_word_buffer[fifo_word_index + 1U]);
						magnetometer_buffer[1U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 4U], fifo_word_buffer[fifo_word_index + 3U]);
						magnetometer_buffer[2U] = SENSOR_PACK_INT16(fifo_word_buffer[fifo_word_index + 6U], fifo_word_buffer[fifo_word_index + 5U]);

						// Mark the magnetometer data present in the FIFO count buffer.
						lsm6dsv_qmc6309_sensor_context->fifo_data_buffer->flags |= SENSOR_FIFO_MAGNETOMETER;

						// Log the data.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Got magnetometer data FIFO word from LSM6DSV: x=0x%04" PRIX16 ", y=0x%04" PRIX16 ", z=0x%04" PRIX16 ".",
							/* PRIX16 */ magnetometer_buffer[0U],
							/* PRIX16 */ magnetometer_buffer[1U],
							/* PRIX16 */ magnetometer_buffer[2U]
						);
					} else {
						// Log the skip.
						SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Magnetometer FIFO data is not ready, skip.");
					}
					break;
				// No data in FIFO.
				case LSM6DSV_FIFO_EMPTY:
					// Log the empty tag.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("No data in FIFO, should not be here.");
					break;
				// Invalid.
				default:
					// Log the invalid tag.
					SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Invalid FIFO tag: 0x%02" PRIX8 ".", fifo_word_tag->tag_sensor);
					break;
			}
		}
	}

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Updating frequently modifiable FIFO states of the LSM6DSV+QMC6309 sensor context.");

	// Update the (possibly) modified FIFO states of the LSM6DSSV+QMC6309 sensor context.
	lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_value	= fifo_timestamp_last_value;
	lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_valid	= fifo_timestamp_last_valid;
	lsm6dsv_qmc6309_sensor_context->fifo_magnetometer_ready		= fifo_magnetometer_ready;

	// Log the operation.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" has finished polling data from FIFO.", sensor_context->name);

	return SLIME_SENSOR_OK();

	// Wrap the ESP error to slime sensor error;
	error_imu:	return SLIME_IMU_ERROR	(ret);
	error_host:	return SLIME_HOST_ERROR	(ret);
}

/**
 * @brief		Get the value of the ODR from the ODR enum.
 * @param odr	the ODR enum to get the value of the ODR.
 * @return		the ODR value.
 */
static float_t slime_lsm6dsv_qmc6309_sensor_get_odr(lsm6dsv_data_rate_t odr);

/**
 * @brief					Release the sensor context.
 * @param sensor_context_in The LSM6DSV+QMC6309 sensor context to be released.
 * @return					The status of the releasing.
 */
static esp_err_t slime_lsm6dsv_qmc6309_sensor_context_del(slime_sensor_context_t* sensor_context_in);

slime_sensor_error_t slime_lsm6dsv_qmc6309_sensor_context_new(
			slime_sensor_context_t**	sensor_context_out,
	const	char*						sensor_context_name,
	const	void*						sensor_context_config,
	const	slime_i2c_context_t*		i2c_context
) {
	esp_err_t ret = ESP_OK;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Reserving handles of LSM6DSV+QMC6309 sensor context.");

	// Reserve the handle of the LSM6DSV+QMC6309 sensor context and the FIFO word buffer of the context.
	slime_lsm6dsv_qmc6309_sensor_context_t*				lsm6dsv_qmc6309_sensor_context	= NULL;
	slime_lsm6dsv_qmc6309_sensor_fifo_data_buffer_t*	fifo_data_buffer				= NULL;
	uint8_t*											fifo_word_buffer				= NULL;

	// We cannot proceed without a name, a configuration, an I2C context of the devices, and a handle to receive the created empty sensor context.
	ESP_GOTO_ON_FALSE(sensor_context_out	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_sensor_context_t handle provided when creating LSM6DSV+QMC6309 sensor context.");
	ESP_GOTO_ON_FALSE(sensor_context_name	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No name provided when creating LSM6DSV+QMC6309 sensor context.");
	ESP_GOTO_ON_FALSE(sensor_context_config	!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No configuration handle provided when creating LSM6DSV+QMC6309 sensor context.");
	ESP_GOTO_ON_FALSE(i2c_context			!= NULL, ESP_ERR_INVALID_ARG, error_host, TAG, "No slime_i2c_context_t handle provided when creating LSM6DSV+QMC6309 sensor context.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Creating LSM6DSV+QMC6309 sensor context \"%s\".", sensor_context_name);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Reinterpreting opaque configuration handle.");

	// Reinterpret the opaque sensor_context_config handle to the slime_lsm6dsv_qmc6309_sensor_context_config_t handle.
	const slime_lsm6dsv_qmc6309_sensor_context_config_t* config = (slime_lsm6dsv_qmc6309_sensor_context_config_t*) sensor_context_config;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Allocating handles of LSM6DSV+QMC6309 sensor context.");

	// Create the LSM6DSV+QMC6309 context struct and the FIFO word buffer of the sensor context.
	lsm6dsv_qmc6309_sensor_context	= calloc(1U,		sizeof(slime_lsm6dsv_qmc6309_sensor_context_t));
	fifo_data_buffer				= calloc(1U,		sizeof(slime_lsm6dsv_qmc6309_sensor_fifo_data_buffer_t));
	fifo_word_buffer				= calloc(7U * 256U,	sizeof(uint8_t));

	// Check the allocation of the sensor context.
	ESP_GOTO_ON_FALSE(lsm6dsv_qmc6309_sensor_context != NULL, ESP_ERR_NO_MEM, error_host, TAG, "Failed to create sensor context struct for LSM6DSV+QMC6309 sensor context.");

	// Check the allocations of the FIFO buffers.
	ESP_GOTO_ON_FALSE(fifo_data_buffer != NULL, ESP_ERR_NO_MEM, error_host, TAG, "Failed to create FIFO data buffer of LSM6DSV+QMC6309 sensor context.");
	ESP_GOTO_ON_FALSE(fifo_word_buffer != NULL, ESP_ERR_NO_MEM, error_host, TAG, "Failed to create FIFO word buffer of LSM6DSV+QMC6309 sensor context.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Setting I2C device addresses of the I2C context.");

	// Set the I2C device addresses to LSM6DSV and QMC6309's addresses.
	ESP_GOTO_ON_ERROR(slime_i2c_set_device_address(
		/* i2c_context			= */ i2c_context,
		/* imu_device_address	= */ config->lsm6dsv_device_address,
		/* mag_device_address	= */ config->qmc6309_device_address
	), error_host, TAG, "Failed to set I2C device addresses.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Caching device driver context handles of the created sensor context struct to stack.");

	// Cache the handles of the device driver contexts of the sensor context.
	stmdev_ctx_t*	lsm6dsv_context = &lsm6dsv_qmc6309_sensor_context->lsm6dsv_context;
	qmc_context_t*	qmc6309_context = &lsm6dsv_qmc6309_sensor_context->qmc6309_context;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Linking I2C devices to the driver contexts.");

	lsm6dsv_context->handle			= i2c_context->imu_device_handle;
	qmc6309_context->user_handle	= i2c_context->mag_device_handle;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Initializing the driver context of LSM6DSV.");

	// Fill the device driver context of the LSM6DSV.
	lsm6dsv_context->mdelay		= delay_milliseconds;
	lsm6dsv_context->read_reg	= slime_i2c_read_register;
	lsm6dsv_context->write_reg	= slime_i2c_write_register;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Initializing the driver context of QMC6309.");

	// Fill the device driver context of the QMC6309.
	qmc6309_context->delay_milliseconds	= delay_milliseconds;
	qmc6309_context->read_register		= slime_i2c_read_register;
	qmc6309_context->write_register		= slime_i2c_write_register;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Checking LSM6DSV's device ID.");

	// Reserve space for the device ID of LSM6DSV to be checked.
	uint8_t lsm6dsv_device_id = 0xFFU;

	// Get the device ID of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_device_id_get(lsm6dsv_context, &lsm6dsv_device_id), error_imu, TAG, "Failed to get device ID of LSM6DSV.");
	// Compare the ID with the reference value.
	ESP_GOTO_ON_FALSE(lsm6dsv_device_id == LSM6DSV_ID, ESP_ERR_INVALID_RESPONSE, error_imu, TAG, "Failed to match device ID of LSM6DSV.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Resetting LSM6DSV.");

	// Soft reset LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sw_por(lsm6dsv_context), error_imu, TAG, "Failed to soft reset LSM6DSV.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Configuring LSM6DSV.");

	// LSM6DSV sensor board has external pull-up for sensor hub for better stability, but the sensor board has external pull-up resisters..
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_master_interface_pull_up_set(lsm6dsv_context, PROPERTY_DISABLE), error_imu, TAG, "Failed to disable sensor hub internal pull-up of LSM6DSV.");

	// Temporarily disable the sensor hub for not interfering the subsequent QMC6309 initialization.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_master_set(lsm6dsv_context, PROPERTY_DISABLE), error_imu, TAG, "Failed to temporarily disable sensor hub of LSM6DSV.");

	// Delay until the sensor hub is fully disabled.
	delay_milliseconds(300U);

	// Configure the gyroscope, accelerometer, and timestamp of LSM6DSV.

	// Configure the full scale range of the gyroscope and accelerometer of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_gy_full_scale_set(lsm6dsv_context, config->lsm6dsv_gyroscope_full_scale),		error_imu, TAG, "Failed to set gyroscope full scale range of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_xl_full_scale_set(lsm6dsv_context, config->lsm6dsv_accelerometer_full_scale),	error_imu, TAG, "Failed to set accelerometer full scale range of LSM6DSV.");

	// Configure the output data rate of the gyroscope and accelerometer of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_gy_data_rate_set(lsm6dsv_context, config->lsm6dsv_gyroscope_data_rate),		error_imu, TAG, "Failed to set gyroscope output data rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_xl_data_rate_set(lsm6dsv_context, config->lsm6dsv_accelerometer_data_rate),	error_imu, TAG, "Failed to set accelerometer output data rate of LSM6DSV.");

	// Enable the timestamp output of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_timestamp_set(lsm6dsv_context, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable timestamp counter of LSM6DSV.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Configuring Sensor Hub of LSM6DSV.");

	// Configure the sensor hub of LSM6DSV.

	// Enable the slave0 and slave1.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_slave_connected_set(lsm6dsv_context, LSM6DSV_SLV_0_1), error_imu, TAG, "Failed to set slave connectivity of LSM6DSV.");

	// Configure the slave0 for STATUS1 register read of the QMC6309, configure the slave1 for the output registers read of the QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_slv_cfg_read(lsm6dsv_context, 0, &sensor_hub_slave_0_read_config), error_imu, TAG, "Failed to setup sensor hub slave 0 read configuration of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_slv_cfg_read(lsm6dsv_context, 1, &sensor_hub_slave_1_read_config), error_imu, TAG, "Failed to setup sensor hub slave 1 read configuration of QMC6309.");

	// Configure the data rate of the sensor hub.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_data_rate_set(lsm6dsv_context, config->lsm6dsv_sensor_hub_data_rate), error_imu, TAG, "Failed to set sensor hub data rate of LSM6DSV.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Configuring FIFO of LSM6DSV.");

	// Configure the FIFO of the LSM6DSV.

	// Enable the block data output to avoid inconsistent data while reading.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_block_data_update_set(lsm6dsv_context, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable block data update of LSM6DSV.");

	// Set the timestamp FIFO batch rate.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_timestamp_batch_set(lsm6dsv_context, LSM6DSV_TMSTMP_DEC_1), error_imu, TAG, "Failed to set FIFO timestamp decimation of LSM6DSV.");

	// Set the FIFO batch rate of the gyroscope and accelerometer of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_gy_batch_set(lsm6dsv_context, config->lsm6dsv_fifo_gyroscope_batch_rate),		error_imu, TAG, "Failed to set FIFO gyroscope batch rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_xl_batch_set(lsm6dsv_context, config->lsm6dsv_fifo_accelerometer_batch_rate),	error_imu, TAG, "Failed to set FIFO accelerometer batch rate of LSM6DSV.");

	// Enable the FIFO batching of slave0 and slave1 of the sensor hub.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_sh_batch_slave_set(lsm6dsv_context, 0U, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable sensor hub slave 0 FIFO batch of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_sh_batch_slave_set(lsm6dsv_context, 1U, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable sensor hub slave 1 FIFO batch of LSM6DSV");

	// Set the FIFO watermark.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_watermark_set(lsm6dsv_context, 5U), error_imu, TAG, "Failed to set FIFO watermark of LSM6DSV.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Enabling I2C passthrough of LSM6DSV.");

	// Enable the I2C passthrough of LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_pass_through_set(lsm6dsv_context, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable I2C passthrough of LSM6DSV.");

	// Wait until the passthrough is fully enabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Checking QMC6309's chip ID.");

	// Reserve space for the chip ID of QMC6309 to be checked.
	uint8_t qmc6309_chip_id = 0xFFU;

	// Get the chip ID of the QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) qmc6309_raw_chip_id_get(qmc6309_context, &qmc6309_chip_id), error_mag, TAG, "Failed to read chip ID of QMC6309.");
	// Compare the ID with the reference value.
	ESP_GOTO_ON_FALSE(qmc6309_chip_id == QMC6309_CHIP_ID_REF, ESP_ERR_INVALID_RESPONSE, error_mag, TAG, "Failed to match chip ID of QMC6309.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Resetting QMC6309.");

	// Soft reset the QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) qmc6309_hl_soft_reset(qmc6309_context), error_mag, TAG, "Failed to soft reset QMC6309.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Configuring QMC6309.");

	// Configure the QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) qmc6309_hl_setup(qmc6309_context, config->qmc6309_setup), error_mag, TAG, "Failed to setup QMC6309.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Disabling I2C passthrough of LSM6DSV.");

	// Disable the I2C passthrough of LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_pass_through_set(lsm6dsv_context, PROPERTY_DISABLE), error_imu, TAG, "Failed to disable I2C passthrough of LSM6DSV.");

	// Wait until the passthrough is fully disabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calculating sensor sensitivity.");

	// Calculate the sensitivity of the sensors and save them in the sensor context.

	// Calculate the sensitivity of the gyroscope of LSM6DSV.
	switch (config->lsm6dsv_gyroscope_full_scale) {
		case LSM6DSV_125dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs125_to_mdps	(1); break;
		case LSM6DSV_250dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs250_to_mdps	(1); break;
		case LSM6DSV_500dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs500_to_mdps	(1); break;
		case LSM6DSV_1000dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs1000_to_mdps	(1); break;
		case LSM6DSV_2000dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs2000_to_mdps	(1); break;
		case LSM6DSV_4000dps:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sensitivity = lsm6dsv_from_fs4000_to_mdps	(1); break;
	}

	// Calculate the sensitivity of the accelerometer of LSM6DSV.
	switch (config->lsm6dsv_accelerometer_full_scale) {
		case LSM6DSV_2g:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sensitivity = lsm6dsv_from_fs2_to_mg	(1); break;
		case LSM6DSV_4g:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sensitivity = lsm6dsv_from_fs4_to_mg	(1); break;
		case LSM6DSV_8g:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sensitivity = lsm6dsv_from_fs8_to_mg	(1); break;
		case LSM6DSV_16g:	lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sensitivity = lsm6dsv_from_fs16_to_mg	(1); break;
	}

	// Calculate the sensitivity of magnetometer of QMC6309.
	switch (config->qmc6309_setup.rng) {
		case RNG_8G:	lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sensitivity = qmc6309_ll_from_rng8_to_gauss	(1); break;
		case RNG_16G:	lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sensitivity = qmc6309_ll_from_rng16_to_gauss	(1); break;
		case RNG_32G:	lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sensitivity = qmc6309_ll_from_rng32_to_gauss	(1); break;
	}

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrating timestamp and output data rate of LSM6DSV.");
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Getting internal frequency difference from LSM6DSV.");

	// Reserve space for internal frequency difference of the LSM6DSV.
	int8_t internal_frequency_difference = 0;

	// Get internal frequency of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_odr_cal_reg_get(lsm6dsv_context, &internal_frequency_difference), error_imu, TAG, "Failed to get internal frequency difference of LSM6DSV.");

	// Calculate the scale factor.
	const float_t scale_factor = (1.0f + 0.0013f * internal_frequency_difference);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrating timestamp sensitivity.");

	// Calculate and save the timestamp sensitivity in seconds.
	lsm6dsv_qmc6309_sensor_context->lsm6dsv_timestamp_sensitivity = 1.0f / (46080.0f * scale_factor);

	// Log the calibrated sensitivity of timestamp.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrated sensitivity of timestamp: %.6f.", lsm6dsv_qmc6309_sensor_context->lsm6dsv_timestamp_sensitivity);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrating sensor sample times.");

	float_t qmc6309_data_rate = 0.0f;

	switch (config->qmc6309_setup.odr) {
		case ODR_1HZ:	qmc6309_data_rate = 1.0f;	break;
		case ODR_10HZ:	qmc6309_data_rate = 10.0f;	break;
		case ODR_50HZ:	qmc6309_data_rate = 50.0f;	break;
		case ODR_100HZ:	qmc6309_data_rate = 100.0f;	break;
		case ODR_200HZ:	qmc6309_data_rate = 200.0f;	break;
	}

	// Calculate the calibrated sample times.
	const float_t gyroscope_sample_time		= 1000.0f / (slime_lsm6dsv_qmc6309_sensor_get_odr(config->lsm6dsv_gyroscope_data_rate)		* scale_factor);
	const float_t accelerometer_sample_time	= 1000.0f / (slime_lsm6dsv_qmc6309_sensor_get_odr(config->lsm6dsv_accelerometer_data_rate)	* scale_factor);
	const float_t magnetometer_sample_time	= 1000.0f / qmc6309_data_rate;

	// Store the calibrated sample times to sensor context.
	lsm6dsv_qmc6309_sensor_context->lsm6dsv_gyroscope_sample_time		= gyroscope_sample_time;
	lsm6dsv_qmc6309_sensor_context->lsm6dsv_accelerometer_sample_time	= accelerometer_sample_time;
	lsm6dsv_qmc6309_sensor_context->qmc6309_magnetometer_sample_time	= magnetometer_sample_time;

	// Log the calibrated sample times.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrated sample time of gyroscope: %.2f ms.",		gyroscope_sample_time);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrated sample time of accelerometer: %.2f ms.",	accelerometer_sample_time);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Calibrated sample time of magnetometer: %.2f ms.",	magnetometer_sample_time);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Starting sensors.");

	// Start FIFO data streaming of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_mode_set(lsm6dsv_context, LSM6DSV_STREAM_MODE), error_imu, TAG, "Failed to enable FIFO streaming of LSM6DSV.");

	// Start sensor hub slave data polling.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_master_set(lsm6dsv_context, PROPERTY_ENABLE), error_imu, TAG, "Failed to enable sensor hub of LSM6DSV.");

	// Delay until the sensor hub is fully enabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Finalizing LSM6DSV+QMC6309 sensor context.");

	// Get the base sensor context struct handle from the LSM6DSV+QMC6309 sensor context struct.
	slime_sensor_context_t* base = &lsm6dsv_qmc6309_sensor_context->base;

	// Fill the base sensor context struct.
	base->name					= sensor_context_name;
	base->register_callbacks	= slime_lsm6dsv_qmc6309_sensor_register_callbacks;
	base->get_sample_time		= slime_lsm6dsv_qmc6309_sensor_get_sample_time;
	base->poll_fifo				= slime_lsm6dsv_qmc6309_sensor_poll_fifo;
	base->delete				= slime_lsm6dsv_qmc6309_sensor_context_del;

	// Fill the sensor context.
	lsm6dsv_qmc6309_sensor_context->fifo_data_buffer			= fifo_data_buffer;
	lsm6dsv_qmc6309_sensor_context->fifo_word_buffer			= fifo_word_buffer;
	lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_value	= 0U;
	lsm6dsv_qmc6309_sensor_context->fifo_timestamp_last_valid	= false;
	lsm6dsv_qmc6309_sensor_context->fifo_magnetometer_ready		= false;
	lsm6dsv_qmc6309_sensor_context->gyroscope_callback			= NULL;
	lsm6dsv_qmc6309_sensor_context->accelerometer_callback		= NULL;
	lsm6dsv_qmc6309_sensor_context->magnetometer_callback		= NULL;
	lsm6dsv_qmc6309_sensor_context->timestamp_callback			= NULL;
	lsm6dsv_qmc6309_sensor_context->callbacks_user_context		= NULL;

	// Return the created LSM6DSV+QMC6309 sensor context.
	*sensor_context_out = base;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" has been created.", sensor_context_name);

	return SLIME_SENSOR_OK();

	// Reserve the sensor error.
	slime_sensor_error_t sensor_error;

	// Wrap the ESP error to slime sensor error;
	error_imu:	sensor_error = SLIME_IMU_ERROR	(ret);	goto error;
	error_mag:	sensor_error = SLIME_MAG_ERROR	(ret);	goto error;
	error_host:	sensor_error = SLIME_HOST_ERROR	(ret);	goto error;

	// Resource cleanup when error occurred.
	error:

	if (fifo_data_buffer)				free(fifo_data_buffer);					// Cleanup the FIFO data buffer of the LSM6DSV+QMC6309 sensor context.
	if (fifo_word_buffer)				free(fifo_word_buffer);					// Cleanup the FIFO word buffer of the LSM6DSV+QMC6309 sensor context.
	if (lsm6dsv_qmc6309_sensor_context)	free(lsm6dsv_qmc6309_sensor_context);	// Cleanup the LSM6DSV+QMC6309 sensor context struct.

	return sensor_error;
}

static esp_err_t slime_lsm6dsv_qmc6309_sensor_context_del(slime_sensor_context_t* sensor_context_in) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(sensor_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No sensor_context_in handle provided when releasing LSM6DSV+QMC6309 sensor context.");

	// Reserve the name of the sensor context.
	const char* name = sensor_context_in->name;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Releasing LSM6DSV+QMC6309 sensor context \"%s\".", name);
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Getting container LSM6DSV+QMC6309 sensor context handle from the base sensor context.");

	// Get the container LSM6DSV+QMC6309 sensor context handle of the base sensor context handle.
	slime_lsm6dsv_qmc6309_sensor_context_t* lsm6dsv_qmc6309_sensor_context = __containerof(
		/* value		= */ sensor_context_in,
		/* container	= */ slime_lsm6dsv_qmc6309_sensor_context_t,
		/* field_offset	= */ base
	);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Caching handles of device driver contexts from the sensor context struct to stack.");

	// Fetch the handles of the device driver contexts from the sensor context.
	stmdev_ctx_t*	lsm6dsv_context = &lsm6dsv_qmc6309_sensor_context->lsm6dsv_context;
	qmc_context_t*	qmc6309_context = &lsm6dsv_qmc6309_sensor_context->qmc6309_context;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Stopping sensors.");

	// Stop sensor hub slave data polling and FIFO data streaming of the LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_fifo_mode_set	(lsm6dsv_context, LSM6DSV_BYPASS_MODE),	skip_stop, TAG, "Failed to disable FIFO streaming of LSM6DSV.");
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_master_set	(lsm6dsv_context, PROPERTY_DISABLE),	skip_stop, TAG, "Failed to disable sensor hub of LSM6DSV.");

	// Delay until the sensor hub is fully disabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Enabling I2C passthrough of LSM6DSV.");

	// Enable the I2C passthrough of LSM6DSV for resetting QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_pass_through_set(lsm6dsv_context, PROPERTY_ENABLE), skip_stop, TAG, "Failed to enable I2C passthrough of LSM6DSV.");

	// Wait until the passthrough is fully enabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Resetting QMC6309.");

	// Soft reset the QMC6309.
	ESP_GOTO_ON_ERROR((esp_err_t) qmc6309_hl_soft_reset(qmc6309_context), skip_stop, TAG, "Failed to soft reset QMC6309.");

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Disabling I2C passthrough of LSM6DSV.");

	// Disable the I2C passthrough of LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sh_pass_through_set(lsm6dsv_context, PROPERTY_DISABLE), skip_stop, TAG, "Failed to disable I2C passthrough of LSM6DSV.");

	// Wait until the passthrough is fully disabled.
	delay_milliseconds(300U);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Resetting LSM6DSV.");

	// Soft reset LSM6DSV.
	ESP_GOTO_ON_ERROR((esp_err_t) lsm6dsv_sw_por(lsm6dsv_context), skip_stop, TAG, "Failed to soft reset LSM6DSV.");

	skip_stop:

	// Log if there is an error.
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Error occurred while stopping and resetting sensors: %s", esp_err_to_name(ret));
		ESP_LOGE(TAG, "Skip stopping sensors.");
	}

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Releasing FIFO buffers.");

	// Free the FIFO buffers.
	free(lsm6dsv_qmc6309_sensor_context->fifo_data_buffer);
	free(lsm6dsv_qmc6309_sensor_context->fifo_word_buffer);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Detaching all fields of the LSM6DSV+qmc6309 sensor context.");

	// Detach all fields.
	lsm6dsv_qmc6309_sensor_context->fifo_data_buffer		= NULL;
	lsm6dsv_qmc6309_sensor_context->fifo_word_buffer		= NULL;
	lsm6dsv_qmc6309_sensor_context->gyroscope_callback		= NULL;
	lsm6dsv_qmc6309_sensor_context->accelerometer_callback	= NULL;
	lsm6dsv_qmc6309_sensor_context->magnetometer_callback	= NULL;
	lsm6dsv_qmc6309_sensor_context->timestamp_callback		= NULL;
	lsm6dsv_qmc6309_sensor_context->callbacks_user_context	= NULL;

	// Detach all fields in the LSM6DSV device driver context.
	lsm6dsv_context->mdelay		= NULL;
	lsm6dsv_context->read_reg	= NULL;
	lsm6dsv_context->write_reg	= NULL;
	lsm6dsv_context->handle		= NULL;

	// Detach all fields in the QMC6309 device driver context.
	qmc6309_context->delay_milliseconds	= NULL;
	qmc6309_context->read_register		= NULL;
	qmc6309_context->write_register		= NULL;
	qmc6309_context->user_handle		= NULL;

	// Detach all fields in the base sensor context.
	sensor_context_in->name					= NULL;
	sensor_context_in->register_callbacks	= NULL;
	sensor_context_in->get_sample_time		= NULL;
	sensor_context_in->poll_fifo			= NULL;
	sensor_context_in->delete				= NULL;

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("Releasing LSM6DSV+QMC6309 sensor context struct.");

	// Release the LSM6DSV+QMC6309 sensor context struct.
	free(sensor_context_in);

	// Log the progress.
	SLIME_LSM6DSV_QMC6309_SENSOR_LOG("LSM6DSV+QMC6309 sensor context \"%s\" has been released.", name);

	return ret;
}

static float_t slime_lsm6dsv_qmc6309_sensor_get_odr(const lsm6dsv_data_rate_t odr) {
	// Get the ODR value from enum.
	switch (odr) {
		case LSM6DSV_ODR_OFF:				return 0.0f;
		case LSM6DSV_ODR_AT_1Hz875:			return 1.875f;
		case LSM6DSV_ODR_AT_7Hz5:			return 7.5f;
		case LSM6DSV_ODR_AT_15Hz:			return 15.0f;
		case LSM6DSV_ODR_AT_30Hz:			return 30.0f;
		case LSM6DSV_ODR_AT_60Hz:			return 60.0f;
		case LSM6DSV_ODR_AT_120Hz:			return 120.0f;
		case LSM6DSV_ODR_AT_240Hz:			return 240.0f;
		case LSM6DSV_ODR_AT_480Hz:			return 480.0f;
		case LSM6DSV_ODR_AT_960Hz:			return 960.0f;
		case LSM6DSV_ODR_AT_1920Hz:			return 1920.0f;
		case LSM6DSV_ODR_AT_3840Hz:			return 3840.0f;
		case LSM6DSV_ODR_AT_7680Hz:			return 7680.0f;
		case LSM6DSV_ODR_HA01_AT_15Hz625:	return 15.625f;
		case LSM6DSV_ODR_HA01_AT_31Hz25:	return 31.25f;
		case LSM6DSV_ODR_HA01_AT_62Hz5:		return 62.5f;
		case LSM6DSV_ODR_HA01_AT_125Hz:		return 125.0f;
		case LSM6DSV_ODR_HA01_AT_250Hz:		return 250.0f;
		case LSM6DSV_ODR_HA01_AT_500Hz:		return 500.0f;
		case LSM6DSV_ODR_HA01_AT_1000Hz:	return 1000.0f;
		case LSM6DSV_ODR_HA01_AT_2000Hz:	return 2000.0f;
		case LSM6DSV_ODR_HA01_AT_4000Hz:	return 4000.0f;
		case LSM6DSV_ODR_HA01_AT_8000Hz:	return 8000.0f;
		case LSM6DSV_ODR_HA02_AT_12Hz5:		return 12.5f;
		case LSM6DSV_ODR_HA02_AT_25Hz:		return 25.0f;
		case LSM6DSV_ODR_HA02_AT_50Hz:		return 50.0f;
		case LSM6DSV_ODR_HA02_AT_100Hz:		return 100.0f;
		case LSM6DSV_ODR_HA02_AT_200Hz:		return 200.0f;
		case LSM6DSV_ODR_HA02_AT_400Hz:		return 400.0f;
		case LSM6DSV_ODR_HA02_AT_800Hz:		return 800.0f;
		case LSM6DSV_ODR_HA02_AT_1600Hz:	return 1600.0f;
		case LSM6DSV_ODR_HA02_AT_3200Hz:	return 3200.0f;
		case LSM6DSV_ODR_HA02_AT_6400Hz:	return 6400.0f;
	}

	// Should never be here.
	return 0.0f;
}