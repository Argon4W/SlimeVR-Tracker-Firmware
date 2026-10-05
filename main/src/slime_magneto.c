#include "slime_magneto.h"

/**
 * @brief The log tag of the Slime Magneto.
 */
static const char* TAG = "slime_magneto";

/**
 * @brief The linear algebra context of the Magneto.
 */
static const magneto_linear_algebra_context_t slime_magneto_linear_algebra_context = {
	.new_matrix						= ceigen_new_matrix,
	.delete_matrix					= ceigen_delete_matrix,
	.get_matrix_coefficient			= ceigen_get_matrix_coefficient,
	.set_matrix_coefficient			= ceigen_set_matrix_coefficient,
	.add_matrix_coefficient			= ceigen_add_matrix_coefficient,
	.multiply_matrix_coefficient	= ceigen_multiply_matrix_coefficient,
	.copy_matrix					= ceigen_copy_matrix,
	.copy_matrix_block				= ceigen_copy_matrix_block,
	.multiply_matrix				= ceigen_multiply_matrix,
	.subtract_matrix				= ceigen_subtract_matrix,
	.invert_matrix					= ceigen_invert_matrix,
	.transpose_matrix				= ceigen_transpose_matrix,
	.normalize_matrix				= ceigen_normalize_matrix,
	.multiply_matrix_scalar			= ceigen_multiply_matrix_scalar,
	.set_matrix_zeros				= ceigen_set_matrix_zeros,
	.solve_matrix_eigen				= ceigen_solve_matrix_eigen
};

esp_err_t slime_magneto_calculate_calibration_coefficients(slime_magneto_context_t* magneto_context) {
	// We cannot proceed without a context and samples.
	ESP_RETURN_ON_FALSE(magneto_context											!= NULL,	ESP_ERR_INVALID_ARG,	TAG, "No slime_magneto_context_t handle provided when performing calculating calibration coefficients.");
	ESP_RETURN_ON_FALSE(magneto_context->sample_container->sample_norm_count	!= 0U,		ESP_ERR_INVALID_STATE,	TAG, "No sample recorded in the magneto context when performing calculating calibration coefficients.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context is calculating magneto calibration coefficients.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Calculating coefficients.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Calculate the calibration coefficients then save the calibration coefficients to the magneto context.
	ESP_RETURN_ON_FALSE(magneto_calculate(
		/* context			= */	magneto_context->linear_algebra_context,
		/* sample_container	= */	magneto_context->sample_container,
		/* soft_iron_matrix	= */	magneto_context->soft_iron_matrix,
		/* hard_iron_vector	= */	magneto_context->hard_iron_vector,
		/* reference_length	= */ &	magneto_context->reference_length
	) == 0, ESP_ERR_INVALID_STATE, TAG, "Failed to calculate calibration coefficients.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Marking the magneto context valid.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Mark the calibration coefficients valid.
	magneto_context->valid = true;

	// Caching the soft iron matrix and hard iron vector of the magneto context to stack.
	const ceigen_matrix_handle_t soft_iron_matrix = magneto_context->soft_iron_matrix;
	const ceigen_matrix_handle_t hard_iron_vector = magneto_context->hard_iron_vector;

	// Log the calibrated coefficients if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Calibrated soft iron matrix: ");
		ESP_LOGD(TAG, "[");

		// Log the rows of the calibrated soft iron matrix.
		for (uint32_t row = 0; row < 3; row ++) {
			ESP_LOGD(TAG, "\t%.2f, %.2f, %.2f",
				ceigen_get_matrix_coefficient(soft_iron_matrix, row, 0U),
				ceigen_get_matrix_coefficient(soft_iron_matrix, row, 1U),
				ceigen_get_matrix_coefficient(soft_iron_matrix, row, 2U)
			);
		}

		ESP_LOGD(TAG, "]");

		// Log the calibrated hard iron vector.
		ESP_LOGD(TAG, "Calibrated hard iron vector: ");
		ESP_LOGD(TAG, "[");
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 0U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 1U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 2U, 0U));
		ESP_LOGD(TAG, "]");
	#endif

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Serializing the calculated coefficients.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve space for serialized calibration coefficients.
	slime_magneto_calibration_coefficients_t magneto_calibration_coefficients;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Serializing soft iron matrix.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the soft iron matrix coefficients array with coefficients from the soft iron matrix of the calculated coefficients.
	for		(uint32_t row = 0U; row < 3U; row ++) {
		for	(uint32_t col = 0U; col < 3U; col ++) {
			// Store the coefficient at the given row and column.
			magneto_calibration_coefficients.soft_iron_matrix[
				/* index_y = */ row * 3U +
				/* index_x = */ col
			] = ceigen_get_matrix_coefficient(
				/* matrix	= */ soft_iron_matrix,
				/* row		= */ row,
				/* column	= */ col
			);
		}
	}

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Serializing hard iron vector.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the hard iron vector array with coefficients from the hard iron vector of the calculated coefficients.
	magneto_calibration_coefficients.hard_iron_vector[0] = ceigen_get_matrix_coefficient(hard_iron_vector, 0U, 0U);
	magneto_calibration_coefficients.hard_iron_vector[1] = ceigen_get_matrix_coefficient(hard_iron_vector, 1U, 0U);
	magneto_calibration_coefficients.hard_iron_vector[2] = ceigen_get_matrix_coefficient(hard_iron_vector, 2U, 0U);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Serializing reference length and valid state.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Copy the valid state and reference length of the calibrated magnetometer output to the serialized calibration coefficients.
	magneto_calibration_coefficients.reference_length	= magneto_context->reference_length;
	magneto_calibration_coefficients.valid				= magneto_context->valid;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Saving the calculated coefficients to the NVS context of the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Save the serialized coefficients to the NVS context.
	ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
		/* nvs_context	= */	magneto_context->nvs_context,
		/* nvs_blob_key	= */	magneto_context->nvs_key,
		/* nvs_blob_in	= */ &	magneto_calibration_coefficients
	), TAG, "Failed to save serialized calibration offsets to the NVS context.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto calibration coefficients has been calculated and saved.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t slime_magneto_reset_calibration_coefficients(slime_magneto_context_t* magneto_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(magneto_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when performing calculating calibration coefficients.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context is resetting magneto calibration coefficients.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Caching magneto context to stack.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Cache the magneto context to stack.
	const slime_nvs_blob_key_t*		calibration_coefficients_key = magneto_context->nvs_key;
	const ceigen_matrix_handle_t	calibration_soft_iron_matrix = magneto_context->soft_iron_matrix;
	const ceigen_matrix_handle_t	calibration_hard_iron_vector = magneto_context->hard_iron_vector;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Caching the handles of the default calibration coefficient from the NVS key of the magneto context to stack.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Cache the handles of coefficient arrays of the soft iron matrix and the hard iron vector from the default calibration coefficients to stack.
	const slime_magneto_calibration_coefficients_t*	calibration_coefficients_init	= calibration_coefficients_key	->init;
	const float_t*									init_soft_iron_matrix			= calibration_coefficients_init	->soft_iron_matrix;
	const float_t*									init_hard_iron_vector			= calibration_coefficients_init	->hard_iron_vector;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting calibration coefficients in the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting soft iron matrix in the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reset the soft iron matrix to the soft iron matrix coefficients from the default value of the serialized coefficients.
	for		(uint32_t row = 0U; row < 3U; row ++) {
		for	(uint32_t col = 0U; col < 3U; col ++) {
			// Reset the coefficient of the soft iron matrix at the given row and column.
			ceigen_set_matrix_coefficient(
				/* matrix	= */ calibration_soft_iron_matrix,
				/* row		= */ row,
				/* column	= */ col,
				/* value	= */ init_soft_iron_matrix[
					/* index_y = */ row * 3U +
					/* index_x = */ col
				]
			);
		}
	}

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting hard iron vector in the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reset the hard iron vector to the hard iron vector coefficients from the default value of the serialized coefficients.
	ceigen_set_matrix_coefficient(calibration_hard_iron_vector, 0U, 0U, init_hard_iron_vector[0]);
	ceigen_set_matrix_coefficient(calibration_hard_iron_vector, 1U, 0U, init_hard_iron_vector[1]);
	ceigen_set_matrix_coefficient(calibration_hard_iron_vector, 2U, 0U, init_hard_iron_vector[2]);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting reference length in the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reset the reference length of the calibrated magnetometer output to the value from the default value of the serialized coefficients.
	magneto_context->reference_length = calibration_coefficients_init->reference_length;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting valid state of the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reset the valid state to the default value of the serialized coefficients.
	magneto_context->valid = calibration_coefficients_init->valid;

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Resetting serialized calibration coefficients in the NVS context of the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Save the default value to the NVS context.
	ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
		/* nvs_context	= */ magneto_context->nvs_context,
		/* nvs_blob_key	= */ calibration_coefficients_key,
		/* nvs_blob_in	= */ calibration_coefficients_init
	), TAG, "Failed to reset the serialized calibration coefficients in the NVS context of the magneto context.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto calibration coefficients has been reset.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t slime_magneto_apply_calibration_coefficients(
	const slime_magneto_context_t*	magneto_context,
	const ceigen_matrix_handle_t	src_vector,
	const ceigen_matrix_handle_t	dst_vector
) {
	// We cannot proceed without the source vector of raw magnetometer output, the destination vector to receive the calibrated result, and a context.
	ESP_RETURN_ON_FALSE(magneto_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when performing applying calibration coefficients to the magnetometer output.");
	ESP_RETURN_ON_FALSE(src_vector		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No source raw magnetometer output provided when performing applying calibration coefficients to the magnetometer output.");
	ESP_RETURN_ON_FALSE(dst_vector		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No destination vector provided to receive the calibrated result when performing applying calibration coefficients to the magnetometer output.");

	// Abort if the coefficients is not valid for applying.
	ESP_RETURN_ON_FALSE(magneto_context->valid, ESP_ERR_INVALID_STATE, TAG, "Cannot apply invalid calibration coefficients to the raw magnetometer output.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context is applying calibration coefficients to the raw magnetometer output: ");
		ESP_LOGD(TAG, "[");
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 0U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 1U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 2U, 0U));
		ESP_LOGD(TAG, "]");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Centering the raw magnetometer output using the hard iron vector.");
	#endif

	// Stored the centered magnetometer output to the temporary vector of the magneto context.
	ceigen_subtract_matrix(
		/* left_matrix			= */ src_vector,
		/* right_matrix			= */ magneto_context->hard_iron_vector,
		/* destination_matrix	= */ magneto_context->temp_vector
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Canceling the soft iron interference using the soft iron matrix.");
	#endif

	// Calibrate the centered magnetometer output with the soft iron matrix,
	// then store the calibrated result to destination vector.
	ceigen_multiply_matrix(
		/* left_matrix			= */ magneto_context->soft_iron_matrix,
		/* right_matrix			= */ magneto_context->temp_vector,
		/* destination_matrix	= */ dst_vector
	);

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Raw magnetometer output has been calibrated.");
	#endif

	// Log the calibrated result if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "The calibrated magnetometer output: ");
		ESP_LOGD(TAG, "[");
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 0U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 1U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 2U, 0U));
		ESP_LOGD(TAG, "]");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}

esp_err_t slime_magneto_collect_sample(
	const	slime_magneto_context_t*	magneto_context,
			float_t						sample_x,
			float_t						sample_y,
			float_t						sample_z
) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(magneto_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when performing collecting sample.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context is collecting raw magnetometer output sample to the sample container: ");
		ESP_LOGD(TAG, "[");
		ESP_LOGD(TAG, "\t%.2f,", sample_x);
		ESP_LOGD(TAG, "\t%.2f,", sample_y);
		ESP_LOGD(TAG, "\t%.2f,", sample_z);
		ESP_LOGD(TAG, "]");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Collect the sample into the sample container of the magneto context.
	magneto_sample(
		/* context			= */ magneto_context->linear_algebra_context,
		/* sample_container	= */ magneto_context->sample_container,
		/* sample_x			= */ sample_x,
		/* sample_y			= */ sample_y,
		/* sample_z			= */ sample_z
	);

	return ESP_OK;
}

esp_err_t slime_magneto_clear_samples(const slime_magneto_context_t* magneto_context) {
	// We cannot proceed without a handle.
	ESP_RETURN_ON_FALSE(magneto_context	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when performing collecting sample.");

	// Log the operation if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context is clearing samples of the sample container.");
	#endif

	// Clear the samples in the sample container by resetting the sample container.
	magneto_reset_sample_container(
		/* context			= */ magneto_context->linear_algebra_context,
		/* sample_container	= */ magneto_context->sample_container
	);

	return ESP_OK;
}

esp_err_t slime_magneto_context_new(
			slime_magneto_context_t**		magneto_context_out,
	const	slime_magneto_context_config_t*	magneto_context_config,
	const	slime_nvs_context_t*			nvs_context
) {
	esp_err_t ret = ESP_OK;

	// We cannot proceed without a configuration, a NVS context, and a handle to receive the created magneto context.
	ESP_RETURN_ON_FALSE(magneto_context_out		!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when creating magneto context.");
	ESP_RETURN_ON_FALSE(magneto_context_config	!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_config_t handle provided when creating magneto context.");
	ESP_RETURN_ON_FALSE(nvs_context				!= NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_nvs_context_t handle provided when creating magneto context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Reserving handles of magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve handles of magneto context.
	slime_magneto_context_t*	magneto_context				= NULL;
	ceigen_matrix_handle_t		context_soft_iron_matrix	= NULL;
	ceigen_matrix_handle_t		context_hard_iron_vector	= NULL;
	ceigen_matrix_handle_t		context_temp_vector			= NULL;
	magneto_sample_container_t*	context_sample_container	= NULL;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating sample container.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the sample container using the linear algebra context.
	context_sample_container = magneto_new_sample_container(&slime_magneto_linear_algebra_context);

	// Check the allocation.
	ESP_GOTO_ON_FALSE(context_sample_container != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create sample container");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating calibration coefficient matrices and vectors.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the CEigen matrices of the calibration coefficients.
	context_soft_iron_matrix	= ceigen_new_matrix(3U, 3U);
	context_hard_iron_vector	= ceigen_new_matrix(3U, 1U);
	context_temp_vector			= ceigen_new_matrix(3U, 1U);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(context_soft_iron_matrix	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create soft iron matrix.");
	ESP_GOTO_ON_FALSE(context_hard_iron_vector	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create hard iron vector.");
	ESP_GOTO_ON_FALSE(context_temp_vector		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary vector.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Creating magneto context struct.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Create the magneto context handle.
	magneto_context = calloc(1, sizeof(slime_magneto_context_config_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(magneto_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create magneto context struct.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Loading serialized calibration coefficients from NVS context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Reserve space for loaded serialized calibration coefficients.
	slime_magneto_calibration_coefficients_t magneto_calibration_coefficients;

	// Fetch the NVS blob key of the calibration coefficients from the configuration of the magneto context.
	const slime_nvs_blob_key_t* magneto_calibration_coefficients_key = &magneto_context_config->calibration_coefficients_key;

	// Load the serialized calibration coefficients from the given NVS context.
	ESP_GOTO_ON_ERROR(slime_nvs_load_blob(
		/* nvs_context	= */	nvs_context,
		/* nvs_blob_key	= */	magneto_calibration_coefficients_key,
		/* nvs_blob_out	= */ &	magneto_calibration_coefficients
	), error, TAG, "Failed to load serialized calibration coefficients from NVS context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deserializing calibration coefficients to magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deserializing soft iron matrix to magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the soft iron matrix and with coefficients from the soft iron matrix array of the serialized calibration coefficients.
	for		(uint32_t row = 0U; row < 3U; row ++) {
		for	(uint32_t col = 0U; col < 3U; col ++) {
			// Set the coefficient of the soft iron matrix at given row and column.
			ceigen_set_matrix_coefficient(
				/* matrix	= */ context_soft_iron_matrix,
				/* row		= */ row,
				/* column	= */ col,
				/* value	= */ magneto_calibration_coefficients.soft_iron_matrix[
					/* index_y = */ row * 3U +
					/* index_x = */ col
				]
			);
		}
	}

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deserializing hard iron vector to magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the hard iron vector with coefficients from the hard iron vector array of the serialized calibration coefficients.
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 0U, 0U, magneto_calibration_coefficients.hard_iron_vector[0]);
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 1U, 0U, magneto_calibration_coefficients.hard_iron_vector[1]);
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 2U, 0U, magneto_calibration_coefficients.hard_iron_vector[2]);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deserializing reference length to the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Deserialize the reference length in the serialized calibration coefficients to the magneto context.
	magneto_context->reference_length = magneto_calibration_coefficients.reference_length;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Deserializing valid state to the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Deserialize the valid state in the serialized calibration coefficients to the magneto context.
	magneto_context->valid = magneto_calibration_coefficients.valid;

	// Log the loaded coefficients if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Loaded soft iron matrix: ");
		ESP_LOGD(TAG, "[");

		// Log the rows of the loaded soft iron matrix.
		for (uint32_t row = 0; row < 3; row ++) {
			ESP_LOGD(TAG, "\t%.2f, %.2f, %.2f",
				ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 0U),
				ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 1U),
				ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 2U)
			);
		}

		ESP_LOGD(TAG, "]");

		// Log the loaded hard iron vector.
		ESP_LOGD(TAG, "Loaded hard iron vector: ");
		ESP_LOGD(TAG, "[");
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 0U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 1U, 0U));
		ESP_LOGD(TAG, "\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 2U, 0U));
		ESP_LOGD(TAG, "]");

		// Log the loaded reference length.
		ESP_LOGD(TAG, "Loaded reference length of the magneto context: %.2f.", magneto_context->reference_length);

		// Log the loaded valid state.
		ESP_LOGD(TAG, "Loaded valid state of the magneto context: %s.", magneto_context->valid ? "valid" : "invalid");
	#endif

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Finalizing magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Fill the magneto context.
	magneto_context->valid					= magneto_calibration_coefficients.valid;
	magneto_context->temp_vector			= context_temp_vector;
	magneto_context->soft_iron_matrix		= context_soft_iron_matrix;
	magneto_context->hard_iron_vector		= context_hard_iron_vector;
	magneto_context->sample_container		= context_sample_container;
	magneto_context->linear_algebra_context	= &slime_magneto_linear_algebra_context;
	magneto_context->nvs_context			= nvs_context;
	magneto_context->nvs_key				= magneto_calibration_coefficients_key;

	// Return the created magneto context.
	*magneto_context_out = magneto_context;

	return ret;

	// Resource cleanup when error occurred.
	error:

	// Log the error if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Error occurred: %s", esp_err_to_name(ret));
		ESP_LOGD(TAG, "Cleaning up resources.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	if (magneto_context)			free							(magneto_context);													// Cleanup the magneto context struct.
	if (context_temp_vector)		ceigen_delete_matrix			(context_temp_vector);												// Cleanup the temporary CEigen vector.
	if (context_hard_iron_vector)	ceigen_delete_matrix			(context_hard_iron_vector);											// Cleanup the hard iron CEigen vector.
	if (context_soft_iron_matrix)	ceigen_delete_matrix			(context_soft_iron_matrix);											// Cleanup the soft iron CEigen matrix.
	if (context_sample_container)	magneto_delete_sample_container	(&slime_magneto_linear_algebra_context, context_sample_container);	// Cleanup the sample container.

	return ret;
}

esp_err_t slime_magneto_context_del(slime_magneto_context_t* magneto_context_in) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(magneto_context_in != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when releasing magneto context.");

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing sample container.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the sample container.
	magneto_delete_sample_container(
		/* context			= */ magneto_context_in->linear_algebra_context,
		/* sample_container	= */ magneto_context_in->sample_container
	);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing Releasing calibration matrices and vectors.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Release the CEigen matrices of calibration coefficients.
	ceigen_delete_matrix(magneto_context_in->temp_vector);
	ceigen_delete_matrix(magneto_context_in->soft_iron_matrix);
	ceigen_delete_matrix(magneto_context_in->hard_iron_vector);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Detaching all fields of the magneto context.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Detach all fields
	magneto_context_in->temp_vector				= NULL;
	magneto_context_in->soft_iron_matrix		= NULL;
	magneto_context_in->hard_iron_vector		= NULL;
	magneto_context_in->sample_container		= NULL;
	magneto_context_in->linear_algebra_context	= NULL;
	magneto_context_in->nvs_context				= NULL;
	magneto_context_in->nvs_key					= NULL;

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Releasing magneto context struct.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	// Free the magneto context struct.
	free(magneto_context_in);

	// Log the progress if debug logging is enabled.
	#ifdef CONFIG_SLIME_DEBUG_LOGGING
		ESP_LOGD(TAG, "Magneto context has been released.");
	#endif // CONFIG_SLIME_DEBUG_LOGGING

	return ESP_OK;
}