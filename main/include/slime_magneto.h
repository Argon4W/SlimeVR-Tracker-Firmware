#ifndef SLIME_MAGNETO_H
#define SLIME_MAGNETO_H

#include "ceigen.h"

/**
 * @brief Define the magneto matrix handle type to avoid casting void* before calling CEigen functions.
 */
#define MAGNETO_MATRIX_HANDLE_TYPE ceigen_matrix_handle_t

#include "magneto.h"
#include "slime_nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The serialized calibration coefficients struct of the magneto context.
 */
typedef struct {
	float_t soft_iron_matrix[3U * 3U];	/*!< The coefficients of the soft iron matrix of the serialized calibration coefficients. */
	float_t hard_iron_vector[3U * 1U];	/*!< The coefficients of the hard iron vector of the serialized calibration coefficients. */
	float_t reference_length;			/*!< The reference length of the calibrated magnetometer output vector. */
	uint8_t valid;						/*!< True if the coefficients are valid for applying. */
} slime_magneto_calibration_coefficients_t;

/**
 * @brief The size of the serialized calibration coefficients struct.
 */
static const size_t slime_magneto_calibration_coefficients_size = sizeof(slime_magneto_calibration_coefficients_t);

/**
 * @brief The configuration struct of the magneto context.
 */
typedef struct {
	slime_nvs_blob_key_t				calibration_coefficients_key;	/*!< The NVS blob key of the serialized calibration coefficients of the magneto context. */
	magneto_linear_algebra_context_t	linear_algebra_context;			/*!< The linear algebra context of the magneto context. */
} slime_magneto_context_config_t;

/**
 * @brief The magneto context struct.
 */
typedef struct {
			uint8_t								valid;					/*!< True if the coefficients are valid for applying. */
			float_t								reference_length;		/*!< The reference length of the calibrated magnetometer output vector. */
			ceigen_matrix_handle_t				soft_iron_matrix;		/*!< The soft iron CEigen matrix of the calibration coefficients. */
			ceigen_matrix_handle_t				hard_iron_vector;		/*!< The hard iron CEigen vector of the calibration coefficients. */
			ceigen_matrix_handle_t				temp_vector;			/*!< The temporary CEigen vector when applying calibration coefficients to raw magnetometer output. */
			magneto_sample_container_t*			sample_container;		/*!< The sampler container for collecting magnetometer samples for calculating calibration coefficients. */
	const	magneto_linear_algebra_context_t*	linear_algebra_context;	/*!< The linear algebra context of the magneto context. */
	const	slime_nvs_context_t*				nvs_context;			/*!< The NVS context to load and store the calibration coefficients. */
	const	slime_nvs_blob_key_t*				nvs_key;				/*!< The NVS blob key of the calibration coefficients. */
} slime_magneto_context_t;

/**
 * @brief					Calculate the calibration coefficients using the recorded samples in the given magneto context
 *							then store the coefficients to the NVS context of the magneto context.
 * @param magneto_context	The magneto context to calculate and store the calibration coefficients.
 * @return					The status of the operation.
 */
esp_err_t slime_magneto_calculate_calibration_coefficients(slime_magneto_context_t* magneto_context);

/**
 * @brief					Reset the calibration coefficients in the given magneto context then store the reset coefficients
 *							to the NVS context of the magneto context.
 * @param magneto_context	The magneto context to reset the calibration coefficients.
 * @return					The status of the reset.
 */
esp_err_t slime_magneto_reset_calibration_coefficients(slime_magneto_context_t* magneto_context);

/**
 * @brief					Apply calibration coefficients of the given magneto context to the given magnetometer output
 *							vector.
 * @param magneto_context	The magneto context to apply the calibration coefficients.
 * @param src_vector		The source CEigen magnetometer vector to be calibrated.
 * @param dst_vector		The destination CEigen magnetometer vector to store the calibrated magnetometer output, can
 *							be the src_vector.
 * @return					The status of applying calibration coefficients.
 */
esp_err_t slime_magneto_apply_calibration_coefficients(
	const	slime_magneto_context_t*	magneto_context,
			ceigen_matrix_handle_t			src_vector,
			ceigen_matrix_handle_t			dst_vector
);

/**
 * @brief					Record a raw magnetometer output sample to the magneto context for calculating calibration
 *							coefficients.
 * @param magneto_context	The magneto context to record the sample.
 * @param sample_x			The X-axis value of the sample.
 * @param sample_y			The Y-axis value of the sample.
 * @param sample_z			The Z-axis value of the sample.
 * @return					The status of recording sample.
 */
esp_err_t slime_magneto_collect_sample(
	const	slime_magneto_context_t*	magneto_context,
			float_t						sample_x,
			float_t						sample_y,
			float_t						sample_z
);

/**
 * @brief					Clear all recorded samples in the given magneto context.
 * @param magneto_context	The magneto context to clear recorded samples.
 * @return					The status of clearing.
 */
esp_err_t slime_magneto_clear_samples(const slime_magneto_context_t* magneto_context);

/**
 * @brief							Create the magneto context and try loading calibration coefficients to the magneto context
 *									from the given NVS context.
 * @param magneto_context_out		The handle to receive the created magneto context.
 * @param magneto_context_config	The configuration of the magneto context.
 * @param nvs_context				The NVS context that loads and stores the calibration coefficients.
 * @return							The status of the creation.
 */
esp_err_t slime_magneto_context_new(
			slime_magneto_context_t**		magneto_context_out,
	const	slime_magneto_context_config_t*	magneto_context_config,
	const	slime_nvs_context_t*			nvs_context
);

/**
 * @brief						Release the magneto context.
 * @param magneto_context_in	The magneto context to be released.
 * @return						The status of the releasing.
 */
esp_err_t slime_magneto_context_del(slime_magneto_context_t* magneto_context_in);

#ifdef __cplusplus
}
#endif

#endif // SLIME_MAGNETO_H
