#include "stdlib.h"
#include "slime_fusion_vqf.h"

/**
 * @brief The log tag of the Slime VQF fusion context.
 */
static const char* TAG = "slime_fusion_vqf";

/**
 * @brief The VQF fusion context struct.
 */
typedef struct {
	/**
	 * @brief The bases fusion context struct of the fusion context.
	 */
	slime_fusion_context_t base;

	/**
	 * @brief The VQF context of the fusion context.
	 */
	vqf_context_t* vqf_context;
} slime_vqf_fusion_context_t;

/**
 * @brief The linear algebra functions of the VQF context.
 */
static const vqf_linear_algebra_t slime_vqf_linear_algebra = {
	.new_matrix						= ceigen_new_matrix,
	.delete_matrix					= ceigen_delete_matrix,
	.is_matrix_zeros				= ceigen_is_matrix_zeros,
	.get_matrix_coefficient			= ceigen_get_matrix_coefficient,
	.set_matrix_coefficient			= ceigen_set_matrix_coefficient,
	.add_matrix_coefficient			= ceigen_add_matrix_coefficient,
	.copy_matrix					= ceigen_copy_matrix,
	.add_matrix						= ceigen_add_matrix,
	.subtract_matrix				= ceigen_subtract_matrix,
	.multiply_matrix				= ceigen_multiply_matrix,
	.clip_matrix					= ceigen_clip_matrix,
	.abs_matrix						= ceigen_abs_matrix,
	.invert_matrix					= ceigen_invert_matrix,
	.transpose_matrix				= ceigen_transpose_matrix,
	.normalize_matrix				= ceigen_normalize_matrix,
	.multiply_matrix_scalar			= ceigen_multiply_matrix_scalar,
	.set_matrix_zeros				= ceigen_set_matrix_zeros,
	.set_matrix_scaled_identity		= ceigen_set_matrix_scaled_identity,
	.get_vector_norm				= ceigen_get_vector_norm,
	.get_vector_squared_norm		= ceigen_get_vector_squared_norm,
	.sum_matrix_column_vectors		= ceigen_sum_matrix_column_vectors,
	.new_matrix_double				= ceigen_new_matrix_double,
	.delete_matrix_double			= ceigen_delete_matrix_double,
	.get_matrix_double_coefficient	= ceigen_get_matrix_double_coefficient,
	.set_matrix_double_coefficient	= ceigen_set_matrix_double_coefficient,
	.add_matrix_double_coefficient	= ceigen_add_matrix_double_coefficient,
	.copy_matrix_double				= ceigen_copy_matrix_double,
	.add_matrix_double				= ceigen_add_matrix_double,
	.accumulate_matrix_double		= ceigen_accumulate_matrix_double,
	.multiply_matrix_double_scalar	= ceigen_multiply_matrix_double_scalar,
	.set_matrix_double_zeros		= ceigen_set_matrix_double_zeros,
	.set_matrix_double_constants	= ceigen_set_matrix_double_constants,
	.copy_matrix_to_matrix_double	= ceigen_copy_matrix_to_matrix_double,
	.copy_matrix_double_to_matrix	= ceigen_copy_matrix_double_to_matrix,
	.new_quaternion					= ceigen_new_quaternion,
	.delete_quaternion				= ceigen_delete_quaternion,
	.set_quaternion_w				= ceigen_set_quaternion_w,
	.set_quaternion_x				= ceigen_set_quaternion_x,
	.set_quaternion_y				= ceigen_set_quaternion_y,
	.set_quaternion_z				= ceigen_set_quaternion_z,
	.copy_quaternion				= ceigen_copy_quaternion,
	.multiply_quaternion			= ceigen_multiply_quaternion,
	.rotate_quaternion_around_z		= ceigen_rotate_quaternion_around_z,
	.set_quaternion_rotation		= ceigen_set_quaternion_rotation,
	.quaternion_rotate_vector		= ceigen_quaternion_rotate_vector,
	.quaternion_to_rotation_matrix	= ceigen_quaternion_to_rotation_matrix,
	.normalize_quaternion			= ceigen_normalize_quaternion,
	.set_quaternion_identity		= ceigen_set_quaternion_identity
};

esp_err_t slime_vqf_fusion_update_gyroscope(
			slime_fusion_context_t*	fusion_context,
	const	ceigen_matrix_handle_t	gyroscope_rad_enu
) {
	// We cannot proceed without a context and gyroscope data.
	ESP_RETURN_ON_FALSE(fusion_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when performing gyroscope update.");
	ESP_RETURN_ON_FALSE(gyroscope_rad_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing gyroscope update.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF fusion context \"%s\" is trying doing gyroscope update. (%.2f rad/s, %.2f rad/s, %.2f rad/s)",
			/* s	*/ fusion_context->name,
			/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_rad_enu, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_rad_enu, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(gyroscope_rad_enu, 2U, 0U)
		);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Do the gyroscope update.
	vqf_update_gyr(
		/* vqf_context	= */ vqf_fusion_context->vqf_context,
		/* gyr			= */ gyroscope_rad_enu
	);

	return ESP_OK;
}

esp_err_t slime_vqf_fusion_update_accelerometer(
			slime_fusion_context_t*	fusion_context,
	const	ceigen_matrix_handle_t	accelerometer_ms2_enu
) {
	// We cannot proceed without a context and gyroscope data.
	ESP_RETURN_ON_FALSE(fusion_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when performing accelerometer update.");
	ESP_RETURN_ON_FALSE(accelerometer_ms2_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing accelerometer update.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF fusion context \"%s\" is trying doing accelerometer update. (%.2f m/s^2, %.2f m/s^2, %.2f m/s^2)",
			/* s	*/ fusion_context->name,
			/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_ms2_enu, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_ms2_enu, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(accelerometer_ms2_enu, 2U, 0U)
		);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Do the accelerometer update.
	vqf_update_acc(
		/* vqf_context	= */ vqf_fusion_context->vqf_context,
		/* acc			= */ accelerometer_ms2_enu
	);

	return ESP_OK;
}

esp_err_t slime_vqf_fusion_update_magnetometer(
			slime_fusion_context_t*	fusion_context,
	const	ceigen_matrix_handle_t	magnetometer_gauss_enu
) {
	// We cannot proceed without a context and gyroscope data.
	ESP_RETURN_ON_FALSE(fusion_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when performing magnetometer update.");
	ESP_RETURN_ON_FALSE(magnetometer_gauss_enu	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No ceigen_matrix_handle_t handle provided when performing magnetometer update.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF fusion context \"%s\" is trying doing magnetometer update. (%.2f gauss, %.2f gauss, %.2f gauss)",
			/* s	*/ fusion_context->name,
			/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 0U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 1U, 0U),
			/* .2f	*/ ceigen_get_matrix_coefficient(magnetometer_gauss_enu, 2U, 0U)
		);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Do the magnetometer update.
	vqf_update_mag(
		/* vqf_context	= */ vqf_fusion_context->vqf_context,
		/* mag			= */ magnetometer_gauss_enu
	);

	return ESP_OK;
}

esp_err_t slime_vqf_fusion_get_orientation(
	const	slime_fusion_context_t*		fusion_context,
			ceigen_quaternion_handle_t	quaternion_enu_6D_out,
			ceigen_quaternion_handle_t	quaternion_enu_9D_out
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing getting orientations.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF Fusion context \"%s\" is trying getting orientations:%s%s.",
			/* s */ fusion_context->name,
			/* s */ quaternion_enu_6D_out ? " 6D" : "",
			/* s */ quaternion_enu_9D_out ? " 9D" : ""
		);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the orientations.
	if (quaternion_enu_6D_out) vqf_get_quat_6D(vqf_fusion_context->vqf_context, quaternion_enu_6D_out);
	if (quaternion_enu_9D_out) vqf_get_quat_9D(vqf_fusion_context->vqf_context, quaternion_enu_9D_out);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		if (quaternion_enu_6D_out) {
			ESP_LOGD(TAG, "Got ENU frame 6D orientation: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
				/* .2f	*/ ceigen_get_quaternion_w(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_x(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_y(quaternion_enu_6D_out),
				/* .2f	*/ ceigen_get_quaternion_z(quaternion_enu_6D_out)
			);
		}
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		if (quaternion_enu_9D_out) {
			ESP_LOGD(TAG, "Got ENU frame 9D orientation: w=%.2f, x=%.2f, y=%.2f, z=%.2f.",
				/* .2f	*/ ceigen_get_quaternion_w(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_x(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_y(quaternion_enu_9D_out),
				/* .2f	*/ ceigen_get_quaternion_z(quaternion_enu_9D_out)
			);
		}
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t slime_vqf_fusion_get_state(
	const	slime_fusion_context_t*	fusion_context,
			uint8_t*				mag_disturbed_out,
			uint8_t*				rest_detected_out
) {
	// We cannot proceed without a context and handles to receive the states.
	ESP_RETURN_ON_FALSE(fusion_context		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing getting states.");
	ESP_RETURN_ON_FALSE(mag_disturbed_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No uint8_t handle provided to receive magnetometer disturbance state when performing getting states.");
	ESP_RETURN_ON_FALSE(rest_detected_out	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No uint8_t handle provided to receive rest detection state when performing getting states.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF Fusion context \"%s\" is trying getting states.", fusion_context->name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the states.
	*mag_disturbed_out = vqf_get_mag_dist_detected	(vqf_fusion_context->vqf_context);
	*rest_detected_out = vqf_get_rest_detected		(vqf_fusion_context->vqf_context);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Got magnetic disturbance state: %s.",	*mag_disturbed_out ? "true" : "false");
		ESP_LOGD(TAG, "Got rest detection state: %s.",			*rest_detected_out ? "true" : "false");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t slime_vqf_fusion_reset(slime_fusion_context_t* fusion_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No fusion_context handle provided when performing resetting fusion.");

	// Get the container VQF fusion context handle of the base fusion context handle.
	const slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF Fusion context \"%s\" is trying resetting fusion.", fusion_context->name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reset the VQF context.
	vqf_reset_state(vqf_fusion_context->vqf_context);

	return ESP_OK;
}

/**
 * @brief					Release the sensor context.
 * @param fusion_context_in	The VQF fusion context to be released.
 * @return					The status of the releasing.
 */
esp_err_t slime_vqf_fusion_context_del(slime_fusion_context_t* fusion_context_in);

esp_err_t slime_vqf_fusion_context_new(
			slime_fusion_context_t**	fusion_context_out,
	const	char*						fusion_context_name,
	const	void*						fusion_context_config,
	const	slime_sensor_context_t*		sensor_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a name, a configuration, a sensor context for getting sample time, and a handle to receive the created fusion context.
	ESP_RETURN_ON_FALSE(fusion_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when creating VQF fusion context.");
	ESP_RETURN_ON_FALSE(fusion_context_name		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No name provided when creating VQF fusion context.");
	ESP_RETURN_ON_FALSE(fusion_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No configuration handle provided when creating VQF fusion context.");
	ESP_RETURN_ON_FALSE(sensor_context			!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_sensor_context_t handle provided when creating VQF fusion context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating VQF fusion context \"%s\".", fusion_context_name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reinterpreting opaque configuration handle.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reinterpret the opaque fusion_context_config handle to the vqf_params_t handle.
	const vqf_params_t* vqf_params = (vqf_params_t*) fusion_context_config;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Getting sample times from sensor context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve spaces for sample times.
	float_t gyroscope_sample_time_ms;
	float_t accelerometer_sample_time_ms;
	float_t magnetometer_sample_time_ms;

	// Get the samples from sensor context.
	ESP_RETURN_ON_ERROR(slime_sensor_get_sample_time(
		/* sensor_context				= */	sensor_context,
		/* gyroscope_sample_time_ms		= */ &	gyroscope_sample_time_ms,
		/* accelerometer_sample_time_ms	= */ &	accelerometer_sample_time_ms,
		/* magnetometer_sample_time_ms	= */ &	magnetometer_sample_time_ms
	), TAG, "Failed to get sample times.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Converting sample times in milliseconds to sample times in seconds.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	const float_t gyroscope_sample_time_seconds		= gyroscope_sample_time_ms		/ 1000.0f;
	const float_t accelerometer_sample_time_seconds	= accelerometer_sample_time_ms	/ 1000.0f;
	const float_t magnetometer_sample_time_seconds	= magnetometer_sample_time_ms	/ 1000.0f;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving handles of VQF fusion context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve the handles of VQF fusion context.
	slime_vqf_fusion_context_t*		vqf_fusion_context;
	vqf_context_t*					context_vqf_context;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating VQF context of VQF fusion context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the VQF context.
	context_vqf_context = vqf_context_new(
		/* linear_algebra	= */ &	slime_vqf_linear_algebra,
		/* params			= */	vqf_params,
		/* gyr_ts			= */	gyroscope_sample_time_seconds,
		/* acc_ts			= */	accelerometer_sample_time_seconds,
		/* mag_ts			= */	magnetometer_sample_time_seconds
	);

	// Check the allocation.
	ESP_RETURN_ON_FALSE(context_vqf_context != NULL, ESP_ERR_NO_MEM, TAG, "Failed to create VQF context");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating VQF fusion context struct.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the VQF fusion context struct handle.
	vqf_fusion_context = calloc(1U, sizeof(slime_vqf_fusion_context_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(vqf_fusion_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create VQF fusion context struct.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing VQF fusion context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the base sensor context struct handle from the LSM6DSV+QMC6309 sensor context struct.
	struct slime_fusion_context* base = &vqf_fusion_context->base;

	// Fill the base fusion context struct.
	base->update_gyroscope		= slime_vqf_fusion_update_gyroscope;
	base->update_accelerometer	= slime_vqf_fusion_update_accelerometer;
	base->update_magnetometer	= slime_vqf_fusion_update_magnetometer;
	base->update_timestamp		= NULL; // VQF uses pre-defined sample times.
	base->get_orientation		= slime_vqf_fusion_get_orientation;
	base->get_state				= slime_vqf_fusion_get_state;
	base->reset					= slime_vqf_fusion_reset;
	base->delete				= slime_vqf_fusion_context_del;
	base->name					= fusion_context_name;

	// Fill the fusion context.
	vqf_fusion_context->vqf_context	= context_vqf_context;

	// Return the created VQF fusion context.
	*fusion_context_out = base;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF fusion context \"%s\" has been created.", fusion_context_name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Cleanup VQF context.
	vqf_context_del(context_vqf_context);

	return ret;
}

esp_err_t slime_vqf_fusion_context_del(slime_fusion_context_t* fusion_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(fusion_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_fusion_context_t handle provided when releasing VQF fusion context.");

	// Reserve the name of the fusion context.
	const char* name = fusion_context_in->name;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing VQF fusion context \"%s\".", name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Getting container VQF fusion context handle from the base fusion context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Get the container VQF fusion context handle of the base fusion context handle.
	slime_vqf_fusion_context_t* vqf_fusion_context = __containerof(
		/* value		= */ fusion_context_in,
		/* container	= */ slime_vqf_fusion_context_t,
		/* field_offset	= */ base
	);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing VQF context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the VQF context.
	vqf_context_del(vqf_fusion_context->vqf_context);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the VQF fusion context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Detach all fields in the base fusion context.
	fusion_context_in->update_gyroscope		= NULL;
	fusion_context_in->update_accelerometer	= NULL;
	fusion_context_in->update_magnetometer	= NULL;
	fusion_context_in->update_timestamp		= NULL;
	fusion_context_in->get_orientation		= NULL;
	fusion_context_in->get_state			= NULL;
	fusion_context_in->reset				= NULL;
	fusion_context_in->delete				= NULL;
	fusion_context_in->name					= NULL;

	// Detach fields in the VQF fusion context.
	vqf_fusion_context->vqf_context	= NULL;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing VQF fusion context struct.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the VQF fusion context struct.
	free(vqf_fusion_context);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "VQF fusion context \"%s\" has been released.", name);
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}