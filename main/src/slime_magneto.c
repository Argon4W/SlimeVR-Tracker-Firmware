#include "slime_magneto.h"

/**
 * @brief The log tag of the Slime Magneto.
 */
static const char* TAG = "slime_magneto";

/**
 * @brief The definition of the debug logging function of the Slime Magneto.
 */
#ifdef CONFIG_SLIME_DEBUG_LOGGING
/**
 * @brief			Log the format string.
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_MAGNETO_LOG(format, ...) ESP_LOGD(TAG, format, ##__VA_ARGS__)
#else
/**
 * @brief			Log the format string (Inactive).
 * @param format	The format string to be logged.
 * @param ...		Optional arguments to be formatted according to the format string.
 */
#define SLIME_MAGNETO_LOG(format, ...)
#endif // CONFIG_SLIME_DEBUG_LOGGING

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

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto context is calculating magneto calibration coefficients.");
	SLIME_MAGNETO_LOG("Calculating coefficients.");

	// Calculate the calibration coefficients then save the calibration coefficients to the magneto context.
	ESP_RETURN_ON_FALSE(magneto_calculate(
		/* context			= */	magneto_context->linear_algebra_context,
		/* sample_container	= */	magneto_context->sample_container,
		/* soft_iron_matrix	= */	magneto_context->soft_iron_matrix,
		/* hard_iron_vector	= */	magneto_context->hard_iron_vector,
		/* reference_length	= */ &	magneto_context->reference_length
	) == 0, ESP_ERR_INVALID_STATE, TAG, "Failed to calculate calibration coefficients.");

	// Log the operation.
	SLIME_MAGNETO_LOG("Marking the magneto context valid.");

	// Mark the calibration coefficients valid.
	magneto_context->valid = true;

	// Caching the soft iron matrix and hard iron vector of the magneto context to stack.
	const ceigen_matrix_handle_t soft_iron_matrix = magneto_context->soft_iron_matrix;
	const ceigen_matrix_handle_t hard_iron_vector = magneto_context->hard_iron_vector;

	// Log the calibrated coefficients.
	SLIME_MAGNETO_LOG("Calibrated soft iron matrix: ");
	SLIME_MAGNETO_LOG("[");

	// Log the rows of the calibrated soft iron matrix.
	for (uint32_t row = 0; row < 3; row ++) {
		SLIME_MAGNETO_LOG("\t%.2f, %.2f, %.2f",
			ceigen_get_matrix_coefficient(soft_iron_matrix, row, 0U),
			ceigen_get_matrix_coefficient(soft_iron_matrix, row, 1U),
			ceigen_get_matrix_coefficient(soft_iron_matrix, row, 2U)
		);
	}

	SLIME_MAGNETO_LOG("]");

	// Log the calibrated hard iron vector.
	SLIME_MAGNETO_LOG("Calibrated hard iron vector: ");
	SLIME_MAGNETO_LOG("[");
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 0U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 1U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(hard_iron_vector, 2U, 0U));
	SLIME_MAGNETO_LOG("]");

	// Log the operation.
	SLIME_MAGNETO_LOG("Serializing the calculated coefficients.");

	// Reserve space for serialized calibration coefficients.
	slime_magneto_calibration_coefficients_t magneto_calibration_coefficients;

	// Log the operation.
	SLIME_MAGNETO_LOG("Serializing soft iron matrix.");

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

	// Log the operation.
	SLIME_MAGNETO_LOG("Serializing hard iron vector.");

	// Fill the hard iron vector array with coefficients from the hard iron vector of the calculated coefficients.
	magneto_calibration_coefficients.hard_iron_vector[0U] = ceigen_get_matrix_coefficient(hard_iron_vector, 0U, 0U);
	magneto_calibration_coefficients.hard_iron_vector[1U] = ceigen_get_matrix_coefficient(hard_iron_vector, 1U, 0U);
	magneto_calibration_coefficients.hard_iron_vector[2U] = ceigen_get_matrix_coefficient(hard_iron_vector, 2U, 0U);

	// Log the operation.
	SLIME_MAGNETO_LOG("Serializing reference length and valid state.");

	// Copy the valid state and reference length of the calibrated magnetometer output to the serialized calibration coefficients.
	magneto_calibration_coefficients.reference_length	= magneto_context->reference_length;
	magneto_calibration_coefficients.valid				= magneto_context->valid;

	// Log the operation.
	SLIME_MAGNETO_LOG("Saving the calculated coefficients to the NVS context of the magneto context.");

	// Save the serialized coefficients to the NVS context.
	ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
		/* nvs_context	= */	magneto_context->nvs_context,
		/* nvs_blob_key	= */	magneto_context->nvs_key,
		/* nvs_blob_in	= */ &	magneto_calibration_coefficients
	), TAG, "Failed to save serialized calibration offsets to the NVS context.");

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto calibration coefficients has been calculated and saved.");

	return ESP_OK;
}

esp_err_t slime_magneto_reset_calibration_coefficients(slime_magneto_context_t* magneto_context) {
	// We cannot proceed without a context.
	ESP_RETURN_ON_FALSE(magneto_context != NULL, ESP_ERR_INVALID_ARG, TAG, "No slime_magneto_context_t handle provided when performing calculating calibration coefficients.");

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto context is resetting magneto calibration coefficients.");
	SLIME_MAGNETO_LOG("Resetting calibration coefficients in the magneto context.");

	// Cast the opaque default value handle to serialized calibration coefficients handle.
	const slime_magneto_calibration_coefficients_t* calibration_coefficients_init = magneto_context->nvs_key->init;

	// Log the operation.
	SLIME_MAGNETO_LOG("Resetting soft iron matrix in the magneto context.");

	// Reset the soft iron matrix to the soft iron matrix coefficients from the default value of the serialized coefficients.
	for		(uint32_t row = 0U; row < 3U; row ++) {
		for	(uint32_t col = 0U; col < 3U; col ++) {
			// Reset the coefficient of the soft iron matrix at the given row and column.
			ceigen_set_matrix_coefficient(
				/* matrix	= */ magneto_context->soft_iron_matrix,
				/* row		= */ row,
				/* column	= */ col,
				/* value	= */ calibration_coefficients_init->soft_iron_matrix[
					/* index_y = */ row * 3U +
					/* index_x = */ col
				]
			);
		}
	}

	// Log the operation.
	SLIME_MAGNETO_LOG("Resetting hard iron vector in the magneto context.");

	// Reset the hard iron vector to the hard iron vector coefficients from the default value of the serialized coefficients.
	ceigen_set_matrix_coefficient(magneto_context->hard_iron_vector, 0U, 0U, calibration_coefficients_init->hard_iron_vector[0U]);
	ceigen_set_matrix_coefficient(magneto_context->hard_iron_vector, 1U, 0U, calibration_coefficients_init->hard_iron_vector[1U]);
	ceigen_set_matrix_coefficient(magneto_context->hard_iron_vector, 2U, 0U, calibration_coefficients_init->hard_iron_vector[2U]);

	// Log the operation.
	SLIME_MAGNETO_LOG("Resetting reference length in the magneto context.");

	// Reset the reference length of the calibrated magnetometer output to the value from the default value of the serialized coefficients.
	magneto_context->reference_length = calibration_coefficients_init->reference_length;

	// Log the operation.
	SLIME_MAGNETO_LOG("Resetting valid state of the magneto context.");

	// Reset the valid state to the default value of the serialized coefficients.
	magneto_context->valid = calibration_coefficients_init->valid;

	// Log the operation.
	SLIME_MAGNETO_LOG("Resetting serialized calibration coefficients in the NVS context of the magneto context.");

	// Save the default value to the NVS context.
	ESP_RETURN_ON_ERROR(slime_nvs_save_blob(
		/* nvs_context	= */ magneto_context->nvs_context,
		/* nvs_blob_key	= */ magneto_context->nvs_key,
		/* nvs_blob_in	= */ calibration_coefficients_init
	), TAG, "Failed to reset the serialized calibration coefficients in the NVS context of the magneto context.");

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto calibration coefficients has been reset.");

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

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto context is applying calibration coefficients to the raw magnetometer output: ");
	SLIME_MAGNETO_LOG("[");
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 0U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 1U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(src_vector, 2U, 0U));
	SLIME_MAGNETO_LOG("]");

	// Log the operation.
	SLIME_MAGNETO_LOG("Centering the raw magnetometer output using the hard iron vector.");

	// Stored the centered magnetometer output to the temporary vector of the magneto context.
	ceigen_subtract_matrix(
		/* left_matrix			= */ src_vector,
		/* right_matrix			= */ magneto_context->hard_iron_vector,
		/* destination_matrix	= */ magneto_context->temp_vector
	);

	// Log the operation.
	SLIME_MAGNETO_LOG("Canceling the soft iron interference using the soft iron matrix.");

	// Calibrate the centered magnetometer output with the soft iron matrix,
	// then store the calibrated result to destination vector.
	ceigen_multiply_matrix(
		/* left_matrix			= */ magneto_context->soft_iron_matrix,
		/* right_matrix			= */ magneto_context->temp_vector,
		/* destination_matrix	= */ dst_vector
	);

	// Log the operation.
	SLIME_MAGNETO_LOG("Raw magnetometer output has been calibrated.");

	// Log the calibrated result.
	SLIME_MAGNETO_LOG("The calibrated magnetometer output: ");
	SLIME_MAGNETO_LOG("[");
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 0U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 1U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(dst_vector, 2U, 0U));
	SLIME_MAGNETO_LOG("]");

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

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto context is collecting raw magnetometer output sample to the sample container: ");
	SLIME_MAGNETO_LOG("[");
	SLIME_MAGNETO_LOG("\t%.2f,", sample_x);
	SLIME_MAGNETO_LOG("\t%.2f,", sample_y);
	SLIME_MAGNETO_LOG("\t%.2f,", sample_z);
	SLIME_MAGNETO_LOG("]");

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

	// Log the operation.
	SLIME_MAGNETO_LOG("Magneto context is clearing samples of the sample container.");

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

	// Log the progress.
	SLIME_MAGNETO_LOG("Creating magneto context.");
	SLIME_MAGNETO_LOG("Reserving handles of magneto context.");

	// Reserve handles of magneto context.
	slime_magneto_context_t*	magneto_context				= NULL;
	ceigen_matrix_handle_t		context_soft_iron_matrix	= NULL;
	ceigen_matrix_handle_t		context_hard_iron_vector	= NULL;
	ceigen_matrix_handle_t		context_temp_vector			= NULL;
	magneto_sample_container_t*	context_sample_container	= NULL;

	// Log the progress.
	SLIME_MAGNETO_LOG("Creating sample container.");

	// Create the sample container using the linear algebra context.
	context_sample_container = magneto_new_sample_container(&slime_magneto_linear_algebra_context);

	// Check the allocation.
	ESP_GOTO_ON_FALSE(context_sample_container != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create sample container");

	// Log the progress.
	SLIME_MAGNETO_LOG("Creating calibration coefficient matrices and vectors.");

	// Create the CEigen matrices of the calibration coefficients.
	context_soft_iron_matrix	= ceigen_new_matrix(3U, 3U);
	context_hard_iron_vector	= ceigen_new_matrix(3U, 1U);
	context_temp_vector			= ceigen_new_matrix(3U, 1U);

	// Check the allocations.
	ESP_GOTO_ON_FALSE(context_soft_iron_matrix	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create soft iron matrix.");
	ESP_GOTO_ON_FALSE(context_hard_iron_vector	!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create hard iron vector.");
	ESP_GOTO_ON_FALSE(context_temp_vector		!= NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create temporary vector.");

	// Log the progress.
	SLIME_MAGNETO_LOG("Creating magneto context struct.");

	// Create the magneto context handle.
	magneto_context = calloc(1U, sizeof(slime_magneto_context_config_t));

	// Check the allocation.
	ESP_GOTO_ON_FALSE(magneto_context != NULL, ESP_ERR_NO_MEM, error, TAG, "Failed to create magneto context struct.");

	// Log the progress.
	SLIME_MAGNETO_LOG("Loading serialized calibration coefficients from NVS context.");

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

	// Log the progress.
	SLIME_MAGNETO_LOG("Deserializing calibration coefficients to magneto context.");
	SLIME_MAGNETO_LOG("Deserializing soft iron matrix to magneto context.");

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

	// Log the progress.
	SLIME_MAGNETO_LOG("Deserializing hard iron vector to magneto context.");

	// Fill the hard iron vector with coefficients from the hard iron vector array of the serialized calibration coefficients.
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 0U, 0U, magneto_calibration_coefficients.hard_iron_vector[0U]);
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 1U, 0U, magneto_calibration_coefficients.hard_iron_vector[1U]);
	ceigen_set_matrix_coefficient(context_hard_iron_vector, 2U, 0U, magneto_calibration_coefficients.hard_iron_vector[2U]);

	// Log the progress.
	SLIME_MAGNETO_LOG("Deserializing reference length to the magneto context.");

	// Deserialize the reference length in the serialized calibration coefficients to the magneto context.
	magneto_context->reference_length = magneto_calibration_coefficients.reference_length;

	// Log the progress.
	SLIME_MAGNETO_LOG("Deserializing valid state to the magneto context.");

	// Deserialize the valid state in the serialized calibration coefficients to the magneto context.
	magneto_context->valid = magneto_calibration_coefficients.valid;

	// Log the loaded coefficients.
	SLIME_MAGNETO_LOG("Loaded soft iron matrix: ");
	SLIME_MAGNETO_LOG("[");

	// Log the rows of the loaded soft iron matrix.
	for (uint32_t row = 0; row < 3; row ++) {
		SLIME_MAGNETO_LOG("\t%.2f, %.2f, %.2f",
			/* .2f */ ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 0U),
			/* .2f */ ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 1U),
			/* .2f */ ceigen_get_matrix_coefficient(context_soft_iron_matrix, row, 2U)
		);
	}

	SLIME_MAGNETO_LOG("]");

	// Log the loaded hard iron vector.
	SLIME_MAGNETO_LOG("Loaded hard iron vector: ");
	SLIME_MAGNETO_LOG("[");
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 0U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 1U, 0U));
	SLIME_MAGNETO_LOG("\t%.2f,", ceigen_get_matrix_coefficient(context_hard_iron_vector, 2U, 0U));
	SLIME_MAGNETO_LOG("]");

	// Log the loaded reference length.
	SLIME_MAGNETO_LOG("Loaded reference length of the magneto context: %.2f.", magneto_context->reference_length);

	// Log the loaded valid state.
	SLIME_MAGNETO_LOG("Loaded valid state of the magneto context: %s.", magneto_context->valid ? "valid" : "invalid");

	// Log the progress.
	SLIME_MAGNETO_LOG("Finalizing magneto context.");

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

	// Log the error.
	SLIME_MAGNETO_LOG("Error occurred: %s", esp_err_to_name(ret));
	SLIME_MAGNETO_LOG("Cleaning up resources.");

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

	SLIME_MAGNETO_LOG("Releasing magneto context.");
	SLIME_MAGNETO_LOG("Releasing sample container.");

	// Release the sample container.
	magneto_delete_sample_container(
		/* context			= */ magneto_context_in->linear_algebra_context,
		/* sample_container	= */ magneto_context_in->sample_container
	);

	// Log the progress.
	SLIME_MAGNETO_LOG("Releasing Releasing calibration matrices and vectors.");

	// Release the CEigen matrices of calibration coefficients.
	ceigen_delete_matrix(magneto_context_in->soft_iron_matrix);
	ceigen_delete_matrix(magneto_context_in->hard_iron_vector);
	ceigen_delete_matrix(magneto_context_in->temp_vector);

	// Log the progress.
	SLIME_MAGNETO_LOG("Detaching all fields of the magneto context.");

	// Detach all fields
	magneto_context_in->soft_iron_matrix		= NULL;
	magneto_context_in->hard_iron_vector		= NULL;
	magneto_context_in->temp_vector				= NULL;
	magneto_context_in->sample_container		= NULL;
	magneto_context_in->linear_algebra_context	= NULL;
	magneto_context_in->nvs_context				= NULL;
	magneto_context_in->nvs_key					= NULL;

	// Log the progress.
	SLIME_MAGNETO_LOG("Releasing magneto context struct.");

	// Free the magneto context struct.
	free(magneto_context_in);

	// Log the progress.
	SLIME_MAGNETO_LOG("Magneto context has been released.");

	return ESP_OK;
}