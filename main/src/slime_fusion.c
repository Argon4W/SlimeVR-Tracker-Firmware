#include <esp_log.h>
#include <slime_fusion.h>

/**
 * @brief The log tag of the Slime Fusion.
 */
static const char* TAG = "slime_fusion";

/**
 * @brief The definition of the debug logging function of the Slime Fusion.
 */
#ifdef CONFIG_SLIME_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_FUSION_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_FUSION_LOG(format, ...)
#endif // CONFIG_SLIME_DEBUG_LOGGING

esp_err_t slime_fusion_update_gyroscope(slime_fusion_context_t* fusion_context, const ceigen_matrix_handle_t gyroscope_mdps_enu) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing gyroscope update.");
	ESP_RETURN_ON_FALSE(gyroscope_mdps_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing gyroscope update.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying doing gyroscope update: x=%.2f mdps, y=%.2f mdps, z=%.2f mdps.",
		/* s	*/ fusion_context->name,
		/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_mdps_enu, 0U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_mdps_enu, 1U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_mdps_enu, 2U, 0U)
	);

	// Do the gyroscope update using the function handle in the fusion context if it exists.
	if (fusion_context->update_gyroscope) {
		// Log the operation.
		SLIME_FUSION_LOG("Transforming ENU frame gyroscope data to fusion frame and unit.");

		// Rotate the ENU frame gyroscope data to fusion frame gyroscope data.
		ceigen_multiply_matrix(
			/* left_matrix			= */ fusion_context->enu_mdps_to_fusion_gyroscope_matrix,
			/* right_matrix			= */ gyroscope_mdps_enu,
			/* destination_matrix	= */ fusion_context->enu_to_fusion_vector
		);

		// Log the operation.
		SLIME_FUSION_LOG("Transformed fusion frame gyroscope data in fusion unit: x=%.2f, y=%.2f, z=%.2f.",
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 2U, 0U)
		);

		// Do the gyroscope update.
		ESP_RETURN_ON_ERROR(fusion_context->update_gyroscope(
			/* fusion_context	= */ fusion_context,
			/* gyroscope_fusion	= */ fusion_context->enu_to_fusion_vector
		), TAG, "Failed to do gyroscope update.");
	}

	return ESP_OK;
}

esp_err_t slime_fusion_update_accelerometer(slime_fusion_context_t* fusion_context, const ceigen_matrix_handle_t accelerometer_mg_enu) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing accelerometer update.");
	ESP_RETURN_ON_FALSE(accelerometer_mg_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing accelerometer update.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying doing accelerometer update: x=%.2f mg, y=%.2f mg, z=%.2f mg.",
		/* s	*/ fusion_context->name,
		/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_mg_enu, 0U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_mg_enu, 1U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_mg_enu, 2U, 0U)
	);

	// Do the accelerometer update using the function handle in the fusion context if it exists.
	if (fusion_context->update_accelerometer) {
		// Log the operation.
		SLIME_FUSION_LOG("Transforming enu body frame accelerometer data to fusion frame and unit.");

		// Rotate the ENU frame accelerometer data to fusion frame accelerometer data.
		ceigen_multiply_matrix(
			/* left_matrix			= */ fusion_context->enu_mg_to_fusion_accelerometer_matrix,
			/* right_matrix			= */ accelerometer_mg_enu,
			/* destination_matrix	= */ fusion_context->enu_to_fusion_vector
		);

		// Log the operation.
		SLIME_FUSION_LOG("Transformed fusion frame accelerometer data in fusion unit: x=%.2f, y=%.2f, z=%.2f.",
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 2U, 0U)
		);

		// Do the accelerometer update.
		ESP_RETURN_ON_ERROR(fusion_context->update_accelerometer(
			/* fusion_context		= */ fusion_context,
			/* accelerometer_fusion	= */ fusion_context->enu_to_fusion_vector
		), TAG, "Failed to do accelerometer update.");
	}

	return ESP_OK;
}

esp_err_t slime_fusion_update_magnetometer( slime_fusion_context_t* fusion_context, const ceigen_matrix_handle_t magnetometer_gauss_enu) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing magnetometer update.");
	ESP_RETURN_ON_FALSE(magnetometer_gauss_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing magnetometer update.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying doing magnetometer update: x=%.2f gauss, y=%.2f gauss, z=%.2f gauss.",
		/* s	*/ fusion_context->name,
		/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 0U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 1U, 0U),
		/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 2U, 0U)
	);

	// Do the magnetometer update using the function handle in the fusion context if it exists.
	if (fusion_context->update_magnetometer) {
		// Log the operation.
		SLIME_FUSION_LOG("Transforming enu body frame magnetometer data to fusion frame and unit.");

		// Rotate the ENU frame magnetometer data to fusion frame magnetometer data.
		ceigen_multiply_matrix(
			/* left_matrix			= */ fusion_context->enu_gauss_to_fusion_magnetometer_matrix,
			/* right_matrix			= */ magnetometer_gauss_enu,
			/* destination_matrix	= */ fusion_context->enu_to_fusion_vector
		);

		// Log the operation.
		SLIME_FUSION_LOG("Transformed fusion frame magnetometer data in fusion unit: x=%.2f, y=%.2f, z=%.2f.",
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(fusion_context->enu_to_fusion_vector, 2U, 0U)
		);

		// Do the magnetometer update.
		ESP_RETURN_ON_ERROR(fusion_context->update_magnetometer(
			/* fusion_context		= */ fusion_context,
			/* magnetometer_fusion	= */ fusion_context->enu_to_fusion_vector
		), TAG, "Failed to do magnetometer update.");
	}

	return ESP_OK;
}

esp_err_t slime_fusion_update_timestamp(slime_fusion_context_t* fusion_context, const float_t delta_timestamp_seconds) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing timestamp update.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying doing timestamp update: delta=%.2f seconds.",
		/* s	*/ fusion_context->name,
		/* .2f	*/ delta_timestamp_seconds
	);

	// Do the timestamp update using the function handle in the fusion context if it exists.
	if (fusion_context->update_timestamp) {
		// Log the operation.
		SLIME_FUSION_LOG("Scaling timestamp data in seconds to fusion unit.");

		const float_t delta_timestamp_fusion = fusion_context->timestamp_seconds_to_fusion_scale * delta_timestamp_seconds;

		// Log the operation.
		SLIME_FUSION_LOG("Transformed fusion frame timestamp data in fusion unit: delta=%.2f.", delta_timestamp_fusion);

		// Do the timestamp update.
		ESP_RETURN_ON_ERROR(fusion_context->update_timestamp(
			/* fusion_context			= */ fusion_context,
			/* delta_timestamp_fusion	= */ delta_timestamp_fusion
		), TAG, "Failed to do timestamp update.");
	}

	return ESP_OK;
}

esp_err_t slime_fusion_get_orientation(
	const slime_fusion_context_t*		fusion_context,
	const ceigen_quaternion_handle_t	quaternion_enu_6D_out,
	const ceigen_quaternion_handle_t	quaternion_enu_9D_out
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing getting orientations.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying getting orientations:%s%s.",
		/* s */ fusion_context->name,
		/* s */ quaternion_enu_6D_out ? " 6D" : "",
		/* s */ quaternion_enu_9D_out ? " 9D" : ""
	);

	// Get the orientations using the function handle in the fusion context if it exists.
	if (fusion_context->get_orientation) {
		// Get the fusion frame orientations.
		ESP_RETURN_ON_ERROR(fusion_context->get_orientation(
			/* fusion_context			= */ fusion_context,
			/* quaternion_fusion_6D_out	= */ quaternion_enu_6D_out,
			/* quaternion_fusion_9D_out	= */ quaternion_enu_9D_out
		), TAG, "Failed to get orientations.");

		// Log the operation.
		SLIME_FUSION_LOG("Transforming fusion frame orientations to ENU earth frame.");

		// 1. Transform the ENU frame vector to fusion frame (q_enu_to_fusion).
		// 2. Rotate the fusion frame vector (q_fusion).
		// 3. Transform the rotated fusion frame vector back to ENU frame (q_fusion_to_enu).

		// q_fusion * q_enu_to_fusion
		if (quaternion_enu_6D_out) ceigen_multiply_quaternion(quaternion_enu_6D_out, fusion_context->enu_to_fusion_quaternion, quaternion_enu_6D_out);
		if (quaternion_enu_9D_out) ceigen_multiply_quaternion(quaternion_enu_9D_out, fusion_context->enu_to_fusion_quaternion, quaternion_enu_9D_out);

		// q_fusion_to_enu * q_fusion * q_enu_to_fusion
		if (quaternion_enu_6D_out) ceigen_multiply_quaternion(fusion_context->fusion_to_enu_quaternion, quaternion_enu_6D_out, quaternion_enu_6D_out);
		if (quaternion_enu_9D_out) ceigen_multiply_quaternion(fusion_context->fusion_to_enu_quaternion, quaternion_enu_9D_out, quaternion_enu_9D_out);

		// Log the operation.
		if (quaternion_enu_6D_out) {
			SLIME_FUSION_LOG("Transformed ENU frame 6D orientation: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
				/* .2f	*/ ceigen_get_quaternion_w(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_x(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_y(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_z(quaternion_enu_6D_out)
			);
		}

		// Log the operation.
		if (quaternion_enu_9D_out) {
			SLIME_FUSION_LOG("Transformed ENU frame 9D orientation: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
				/* .2f	*/ ceigen_get_quaternion_w(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_x(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_y(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_z(quaternion_enu_9D_out)
			);
		}
	} else {
		// Log the operation.
		SLIME_FUSION_LOG("Fusion context \"%s\" does not support getting orientations, defaulting to identity quaternions.", fusion_context->name);

		// Set to identity quaternions.
		if (quaternion_enu_6D_out) ceigen_set_quaternion_identity(quaternion_enu_6D_out);
		if (quaternion_enu_9D_out) ceigen_set_quaternion_identity(quaternion_enu_9D_out);
	}

	return ESP_OK;
}

esp_err_t slime_fusion_get_state(
	const	slime_fusion_context_t*	fusion_context,
			uint8_t*				mag_disturbed_out,
			uint8_t*				rest_detected_out
) {
	// We cannot proceed without a context and handles to receive the states.
	ESP_RETURN_ON_FALSE(fusion_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing getting states.");
	ESP_RETURN_ON_FALSE(mag_disturbed_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No uint8_t handle provided to receive magnetometer disturbance state when performing getting states.");
	ESP_RETURN_ON_FALSE(rest_detected_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No uint8_t handle provided to receive rest detection state when performing getting states.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying getting states.", fusion_context->name);

	// Get the states using the function handle in the fusion context if it exists.
	if (fusion_context->get_state) {
		// Get the fusion frame orientations.
		ESP_RETURN_ON_ERROR(fusion_context->get_state(
			/* fusion_context		= */ fusion_context,
			/* mag_disturbed_out	= */ mag_disturbed_out,
			/* rest_detected_out	= */ rest_detected_out
		), TAG, "Failed to get states.");
	} else {
		// Log the operation.
		SLIME_FUSION_LOG("Fusion context \"%s\" does not support getting states, defaulting to false.", fusion_context->name);

		// Set to false.
		*mag_disturbed_out = false;
		*rest_detected_out = false;
	}

	return ESP_OK;
}

esp_err_t slime_fusion_reset(slime_fusion_context_t* fusion_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing resetting fusion.");

	// Log the operation.
	SLIME_FUSION_LOG("Fusion context \"%s\" is trying resetting fusion.", fusion_context->name);

	// Get the states using the function handle in the fusion context if it exists.
	if (fusion_context->reset) {
		// Get the fusion frame orientations.
		ESP_RETURN_ON_ERROR(fusion_context->reset(fusion_context), TAG, "Failed to reset fusion.");
	}

	return ESP_OK;
}

esp_err_t slime_fusion_context_new(
			slime_fusion_context_t**		fusion_context_out,
	const	slime_fusion_context_type_t*	fusion_context_type_table,
	const	slime_sensor_context_t*			sensor_context,
	const	uint32_t						fusion_context_type_id
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a fusion type table, a sensor context, and a handle to receive the created fusion context.
	ESP_RETURN_ON_FALSE(fusion_context_out			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when creating fusion context.");
	ESP_RETURN_ON_FALSE(fusion_context_type_table	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_type_t table handle provided when creating fusion context.");
	ESP_RETURN_ON_FALSE(sensor_context				!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when creating fusion context.");

	// Log the progress.
	SLIME_FUSION_LOG("Creating fusion context.");
	SLIME_FUSION_LOG("Mapping fusion context type ID to fusion context type.");

	// Get the fusion context type struct handle of the corresponding fusion context type ID.
	const slime_fusion_context_type_t* fusion_context_type = &fusion_context_type_table[fusion_context_type_id];

	// Check the fusion context type.
	ESP_RETURN_ON_FALSE(fusion_context_type != NULL, ESP_ERR_NOT_FOUND, TAG, "Failed to get a fusion context type from the fusion context type table.");

	// Log the progress.
	SLIME_FUSION_LOG("Reserving handles of fusion context.");

	// Reserve the handles of fusion context.
	ceigen_matrix_handle_t		scale_matrix_gyroscope;
	ceigen_matrix_handle_t		scale_matrix_accelerometer;
	ceigen_matrix_handle_t		scale_matrix_magnetometer;
	ceigen_matrix_handle_t		enu_to_fusion_vector;
	ceigen_quaternion_handle_t	enu_to_fusion_quaternion;
	ceigen_quaternion_handle_t	fusion_to_enu_quaternion;
	ceigen_matrix_handle_t		enu_mdps_to_fusion_gyroscope_matrix;
	ceigen_matrix_handle_t		enu_mg_to_fusion_accelerometer_matrix;
	ceigen_matrix_handle_t		enu_gauss_to_fusion_magnetometer_matrix;

	// Log the progress.
	SLIME_FUSION_LOG("Creating transformation quaternions and matrices.");

	// Create the transformation quaternions and matrices.
	scale_matrix_gyroscope					= ceigen_new_matrix		(3U, 3U);
	scale_matrix_accelerometer				= ceigen_new_matrix		(3U, 3U);
	scale_matrix_magnetometer				= ceigen_new_matrix		(3U, 3U);
	enu_to_fusion_vector					= ceigen_new_matrix		(3U, 1U);
	enu_to_fusion_quaternion				= ceigen_new_quaternion	();
	fusion_to_enu_quaternion				= ceigen_new_quaternion	();
	enu_mdps_to_fusion_gyroscope_matrix		= ceigen_new_matrix		(3U, 3U);
	enu_mg_to_fusion_accelerometer_matrix	= ceigen_new_matrix		(3U, 3U);
	enu_gauss_to_fusion_magnetometer_matrix	= ceigen_new_matrix		(3U, 3U);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(scale_matrix_gyroscope					!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary scale matrix of gyroscope data.");
	ESP_GOTO_ON_FALSE(scale_matrix_accelerometer				!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary scale matrix of accelerometer data.");
	ESP_GOTO_ON_FALSE(scale_matrix_magnetometer					!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary scale matrix of magnetometer data.");
	ESP_GOTO_ON_FALSE(enu_to_fusion_vector						!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary transformed vector.");
	ESP_GOTO_ON_FALSE(enu_to_fusion_quaternion					!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create transformation quaternion from ENU frame to fusion frame.");
	ESP_GOTO_ON_FALSE(fusion_to_enu_quaternion					!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create transformation quaternion from fusion frame to ENU frame.");
	ESP_GOTO_ON_FALSE(enu_mdps_to_fusion_gyroscope_matrix		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create scaled rotation matrix for gyroscope data.");
	ESP_GOTO_ON_FALSE(enu_mg_to_fusion_accelerometer_matrix		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create scaled rotation matrix for accelerometer data.");
	ESP_GOTO_ON_FALSE(enu_gauss_to_fusion_magnetometer_matrix	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create scaled rotation matrix for magnetometer data.");

	// Log the progress.
	SLIME_FUSION_LOG("Building transformation quaternions.");
	SLIME_FUSION_LOG("Filling coefficients of the ENU-to-fusion quaternion.");

	// Fill the coefficients to ENU-to-fusion quaternion.
	ceigen_set_quaternion_w(enu_to_fusion_quaternion, fusion_context_type->enu_to_fusion_quaternion[0U]);
	ceigen_set_quaternion_x(enu_to_fusion_quaternion, fusion_context_type->enu_to_fusion_quaternion[1U]);
	ceigen_set_quaternion_y(enu_to_fusion_quaternion, fusion_context_type->enu_to_fusion_quaternion[2U]);
	ceigen_set_quaternion_z(enu_to_fusion_quaternion, fusion_context_type->enu_to_fusion_quaternion[3U]);

	// Log the progress.
	SLIME_FUSION_LOG("Building fusion-to-ENU quaternion from ENU-to-fusion quaternion.");

	// Conjugate the ENU-to-fusion quaternion to create the fusion-to-ENU quaternions.
	ceigen_conjugate_quaternion(
		/* source_quaternion		= */ enu_to_fusion_quaternion,
		/* destination_quaternion	= */ fusion_to_enu_quaternion
	);

	// Log the quaternion.
	SLIME_FUSION_LOG("Built ENU-to-fusion quaternion: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
		/* .2f	*/ ceigen_get_quaternion_w(enu_to_fusion_quaternion),
		/* .2f	*/ ceigen_get_quaternion_x(enu_to_fusion_quaternion),
		/* .2f	*/ ceigen_get_quaternion_y(enu_to_fusion_quaternion),
		/* .2f	*/ ceigen_get_quaternion_z(enu_to_fusion_quaternion)
	);

	// Log the quaternion.
	SLIME_FUSION_LOG("Built fusion-to-ENU quaternion: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
		/* .2f	*/ ceigen_get_quaternion_w(fusion_to_enu_quaternion),
		/* .2f	*/ ceigen_get_quaternion_x(fusion_to_enu_quaternion),
		/* .2f	*/ ceigen_get_quaternion_y(fusion_to_enu_quaternion),
		/* .2f	*/ ceigen_get_quaternion_z(fusion_to_enu_quaternion)
	);

	// Log the progress.
	SLIME_FUSION_LOG("Building scaled rotation matrices.");
	SLIME_FUSION_LOG("Converting ENU-to-fusion quaternion to rotation matrices.");

	// Convert the ENU-to-fusion quaternion to rotation matrices.
	ceigen_quaternion_to_rotation_matrix(enu_to_fusion_quaternion, enu_mdps_to_fusion_gyroscope_matrix);
	ceigen_quaternion_to_rotation_matrix(enu_to_fusion_quaternion, enu_mg_to_fusion_accelerometer_matrix);
	ceigen_quaternion_to_rotation_matrix(enu_to_fusion_quaternion, enu_gauss_to_fusion_magnetometer_matrix);

	// Log the progress.
	SLIME_FUSION_LOG("Building scale matrices.");

	// Build scale matrices based on scale factors.
	ceigen_set_matrix_scaled_identity(fusion_context_type->gyroscope_mdps_to_fusion_scale,		scale_matrix_gyroscope);
	ceigen_set_matrix_scaled_identity(fusion_context_type->accelerometer_mg_to_fusion_scale,	scale_matrix_accelerometer);
	ceigen_set_matrix_scaled_identity(fusion_context_type->magnetometer_gauss_to_fusion_scale,	scale_matrix_magnetometer);

	// Log the progress.
	SLIME_FUSION_LOG("Applying scale matrices to rotation matrices.");

	// Apply the scale matrices to the rotation matrices.
	ceigen_multiply_matrix(scale_matrix_gyroscope,		enu_mdps_to_fusion_gyroscope_matrix,		enu_mdps_to_fusion_gyroscope_matrix);
	ceigen_multiply_matrix(scale_matrix_accelerometer,	enu_mg_to_fusion_accelerometer_matrix,		enu_mg_to_fusion_accelerometer_matrix);
	ceigen_multiply_matrix(scale_matrix_magnetometer,	enu_gauss_to_fusion_magnetometer_matrix,	enu_gauss_to_fusion_magnetometer_matrix);

	// Log the matrix.
	SLIME_FUSION_LOG("Scaled rotation matrix of gyroscope data: ");
	SLIME_FUSION_LOG("[");

	// Log the scaled rotation matrix of gyroscope data.
	for (uint32_t row = 0; row < 3; row ++) {
		SLIME_FUSION_LOG("\t%.2f, %.2f, %.2f",
			/* .2f */ ceigen_get_matrix_coefficient(enu_mdps_to_fusion_gyroscope_matrix, row, 0U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_mdps_to_fusion_gyroscope_matrix, row, 1U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_mdps_to_fusion_gyroscope_matrix, row, 2U)
		);
	}

	SLIME_FUSION_LOG("]");

	// Log the matrix.
	SLIME_FUSION_LOG("Scaled rotation matrix of accelerometer data: ");
	SLIME_FUSION_LOG("[");

	// Log the scaled rotation matrix of accelerometer data.
	for (uint32_t row = 0; row < 3; row ++) {
		SLIME_FUSION_LOG("\t%.2f, %.2f, %.2f",
			/* .2f */ ceigen_get_matrix_coefficient(enu_mg_to_fusion_accelerometer_matrix, row, 0U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_mg_to_fusion_accelerometer_matrix, row, 1U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_mg_to_fusion_accelerometer_matrix, row, 2U)
		);
	}

	SLIME_FUSION_LOG("]");

	// Log the matrix.
	SLIME_FUSION_LOG("Scaled rotation matrix of magnetometer data: ");
	SLIME_FUSION_LOG("[");

	// Log the scaled rotation matrix of magnetometer data.
	for (uint32_t row = 0; row < 3; row ++) {
		SLIME_FUSION_LOG("\t%.2f, %.2f, %.2f",
			/* .2f */ ceigen_get_matrix_coefficient(enu_gauss_to_fusion_magnetometer_matrix, row, 0U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_gauss_to_fusion_magnetometer_matrix, row, 1U),
			/* .2f */ ceigen_get_matrix_coefficient(enu_gauss_to_fusion_magnetometer_matrix, row, 2U)
		);
	}

	SLIME_FUSION_LOG("]");

	// Log the progress.
	SLIME_FUSION_LOG("Creating fusion context of the fusion context type.");

	// Create the fusion context of the fusion context type.
	ESP_GOTO_ON_ERROR(fusion_context_type->fusion_context_new(
		/* fusion_context_out		= */ fusion_context_out,
		/* fusion_context_name		= */ fusion_context_type->name,
		/* fusion_context_config	= */ fusion_context_type->config,
		/* sensor_context			= */ sensor_context
	), error, TAG, "Failed to create fusion context.");

	// Log the progress.
	SLIME_FUSION_LOG("Releasing temporary scale matrices.");

	// Release the scale matrices.
	ceigen_delete_matrix(scale_matrix_gyroscope);
	ceigen_delete_matrix(scale_matrix_accelerometer);
	ceigen_delete_matrix(scale_matrix_magnetometer);

	// Log the progress.
	SLIME_FUSION_LOG("Finalizing fusion context.");

	// Fill the fusion context.
	(*fusion_context_out)->enu_to_fusion_vector						= enu_to_fusion_vector;
	(*fusion_context_out)->enu_to_fusion_quaternion					= enu_to_fusion_quaternion;
	(*fusion_context_out)->fusion_to_enu_quaternion					= fusion_to_enu_quaternion;
	(*fusion_context_out)->enu_mdps_to_fusion_gyroscope_matrix		= enu_mdps_to_fusion_gyroscope_matrix;
	(*fusion_context_out)->enu_mg_to_fusion_accelerometer_matrix	= enu_mg_to_fusion_accelerometer_matrix;
	(*fusion_context_out)->enu_gauss_to_fusion_magnetometer_matrix	= enu_gauss_to_fusion_magnetometer_matrix;
	(*fusion_context_out)->timestamp_seconds_to_fusion_scale		= fusion_context_type->timestamp_seconds_to_fusion_scale;

	// Log the progress.
	SLIME_FUSION_LOG("Fusion context has been created.");

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error.
	SLIME_FUSION_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_FUSION_LOG("Cleaning up resources.");

	if (enu_gauss_to_fusion_magnetometer_matrix)	ceigen_delete_matrix	(enu_gauss_to_fusion_magnetometer_matrix);	// Cleanup the scaled rotation matrix for magnetometer data.
	if (enu_mg_to_fusion_accelerometer_matrix)		ceigen_delete_matrix	(enu_mg_to_fusion_accelerometer_matrix);	// Cleanup the scaled rotation matrix for accelerometer data.
	if (enu_mdps_to_fusion_gyroscope_matrix)		ceigen_delete_matrix	(enu_mdps_to_fusion_gyroscope_matrix);		// Cleanup the scaled rotation matrix for gyroscope data.
	if (fusion_to_enu_quaternion)					ceigen_delete_quaternion(fusion_to_enu_quaternion);					// Cleanup the transformation quaternion from fusion frame to ENU frame.
	if (enu_to_fusion_quaternion)					ceigen_delete_quaternion(enu_to_fusion_quaternion);					// Cleanup the transformation quaternion from ENU frame to fusion frame.
	if (enu_to_fusion_vector)						ceigen_delete_matrix	(enu_to_fusion_vector);						// Cleanup the temporary transformed vector.
	if (scale_matrix_magnetometer)					ceigen_delete_matrix	(scale_matrix_magnetometer);				// Cleanup the temporary scale matrix of magnetometer data.
	if (scale_matrix_accelerometer)					ceigen_delete_matrix	(scale_matrix_accelerometer);				// Cleanup the temporary scale matrix of accelerometer data.
	if (scale_matrix_gyroscope)						ceigen_delete_matrix	(scale_matrix_gyroscope);					// Cleanup the temporary scale matrix of gyroscope data.

	return ret;
}

esp_err_t slime_fusion_context_del(slime_fusion_context_t* fusion_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when releasing fusion context.");

	// Log the progress.
	SLIME_FUSION_LOG("Releasing fusion context.");
	SLIME_FUSION_LOG("Releasing transformation quaternions and matrices.");

	// Release the transformation quaternions and matrices.
	ceigen_delete_matrix	(fusion_context_in->enu_to_fusion_vector);
	ceigen_delete_quaternion(fusion_context_in->enu_to_fusion_quaternion);
	ceigen_delete_quaternion(fusion_context_in->fusion_to_enu_quaternion);
	ceigen_delete_matrix	(fusion_context_in->enu_mdps_to_fusion_gyroscope_matrix);
	ceigen_delete_matrix	(fusion_context_in->enu_mg_to_fusion_accelerometer_matrix);
	ceigen_delete_matrix	(fusion_context_in->enu_gauss_to_fusion_magnetometer_matrix);

	// Log the progress.
	SLIME_FUSION_LOG("Detaching all fields of transformation quaternions and matrices in fusion context.");

	// Detach all fields.
	fusion_context_in->enu_to_fusion_vector						= NULL;
	fusion_context_in->enu_to_fusion_quaternion					= NULL;
	fusion_context_in->fusion_to_enu_quaternion					= NULL;
	fusion_context_in->enu_mdps_to_fusion_gyroscope_matrix		= NULL;
	fusion_context_in->enu_mg_to_fusion_accelerometer_matrix	= NULL;
	fusion_context_in->enu_gauss_to_fusion_magnetometer_matrix	= NULL;

	// Log the progress.
	SLIME_FUSION_LOG("Releasing fusion context of the fusion context type.");

	// Release the fusion context using its own delete function handle.
	// A fusion context must have a delete function, so no null-check here.
	ESP_RETURN_ON_ERROR(fusion_context_in->delete(fusion_context_in), TAG, "Failed to release fusion context");

	// The has-been-released is logged in the impl function.
	return ESP_OK;
}