#include "slime_magneto.h"

/**
 * @brief The default value of the calibration data blob of magneto.
 */
static const slime_magneto_blob_t slime_magneto_blob_default = {
	.magneto_soft_iron_matrix = { /*!< The identity soft iron matrix. */
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	},
	.magneto_hard_iron_vector = { /*!< The zero hard iron vector. */
		0.0f,
		0.0f,
		0.0f
	},
	.magneto_calibrated = false /*!< Mark this data invalid. */
};

const slime_nvs_blob_key_t slime_magneto_blob_key = {
	.blob_name		= "magneto",					/*!< The name of the blob. */
	.blob_default	= &slime_magneto_blob_default,	/*!< The default value of the blob. */
	.blob_size		= sizeof(slime_magneto_blob_t)	/*!< The size of the blob. */
};

const magneto_linear_algebra_context_t slime_magneto_linear_algebra_context = {
	.new_matrix							= slime_magneto_new_matrix,
	.delete_matrix						= slime_magneto_del_matrix,
	.get_matrix_coefficient				= slime_magneto_get_matrix_coefficient,
	.set_matrix_coefficient				= slime_magneto_set_matrix_coefficient,
	.add_matrix_coefficient				= slime_magneto_add_matrix_coefficient,
	.multiply_matrix_coefficient		= slime_magneto_multiply_matrix_coefficient,
	.copy_matrix						= slime_magneto_copy_matrix,
	.copy_matrix_block					= slime_magneto_copy_matrix_block,
	.multiply_matrix					= slime_magneto_multiply_matrix,
	.subtract_matrix					= slime_magneto_subtract_matrix,
	.invert_matrix_in_place				= slime_magneto_invert_matrix_in_place,
	.transpose_matrix_in_place			= slime_magneto_transpose_matrix_in_place,
	.normalize_matrix_in_place			= slime_magneto_normalize_matrix_in_place,
	.multiply_matrix_scalar_in_place	= slime_magneto_multiply_scalar_in_place,
	.solve_matrix_eigen					= slime_magneto_solve_matrix_eigen
};

ceigen_matrix_t* slime_magneto_get_ceigen_matrix(
	magneto_matrix_t* matrix
) {
	return (ceigen_matrix_t*) matrix->implementation_handle;
}

magneto_matrix_t* slime_magneto_new_matrix(
	const uint32_t rows,
	const uint32_t columns
) {
	// Allocate the magneto matrix struct.
	magneto_matrix_t* matrix = calloc(1, sizeof(magneto_matrix_t));

	// Fill the handle with the ceigen matrix.
	matrix->implementation_handle = (void*) ceigen_new_matrix(rows, columns);

	return matrix;
}

void slime_magneto_del_matrix(
	magneto_matrix_t* matrix
) {
	// Free the CEigen matrix in the magneto matrix.
	ceigen_delete_matrix((ceigen_matrix_t*) matrix->implementation_handle);
	// Then free the magneto matrix struct.
	free(matrix);
}

float_t slime_magneto_get_matrix_coefficient(
			magneto_matrix_t*	matrix,
	const	uint32_t			row,
	const	uint32_t			column
) {
	// Set the value of the element.
	return ceigen_get_matrix_coefficient(
		/* matrix	= */ (ceigen_matrix_t*) matrix->implementation_handle,
		/* row		= */ row,
		/* column	= */ column
	);
}

void slime_magneto_set_matrix_coefficient(
			magneto_matrix_t*	matrix,
	const	uint32_t			row,
	const	uint32_t			column,
	const	float_t				value
) {
	// Set the value of the element.
	ceigen_set_matrix_coefficient(
		/* matrix	= */ (ceigen_matrix_t*) matrix->implementation_handle,
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void slime_magneto_add_matrix_coefficient(
			magneto_matrix_t*	matrix,
	const	uint32_t			row,
	const	uint32_t			column,
	const	float_t				value
) {
	// Add a value to the value of the element.
	ceigen_add_matrix_coefficient(
		/* matrix	= */ (ceigen_matrix_t*) matrix->implementation_handle,
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void slime_magneto_multiply_matrix_coefficient(
			magneto_matrix_t*	matrix,
	const	uint32_t			row,
	const	uint32_t			column,
	const	float_t				value
) {
	// Multiply a value to the value of the element.
	ceigen_multiply_matrix_coefficient(
		/* matrix	= */ (ceigen_matrix_t*) matrix->implementation_handle,
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void slime_magneto_copy_matrix(
	magneto_matrix_t* src_matrix,
	magneto_matrix_t* dst_matrix
) {
	// Copy the values from the source matrix to the destination matrix.
	ceigen_copy_matrix(
		/* source_matrix		= */ (ceigen_matrix_t*) src_matrix->implementation_handle,
		/* destination_matrix	= */ (ceigen_matrix_t*) dst_matrix->implementation_handle
	);
}

void slime_magneto_copy_matrix_block(
			magneto_matrix_t*	src_matrix,
	const	uint32_t			from_row,
	const	uint32_t			from_column,
	const	uint32_t			to_row,
	const	uint32_t			to_column,
	const	uint32_t			rows,
	const	uint32_t			columns,
			magneto_matrix_t*	dst_matrix
) {
	// Copy a block of the source matrix to a block the destination matrix.
	ceigen_copy_matrix_block(
		/* source_matrix		= */ (ceigen_matrix_t*) src_matrix->implementation_handle,
		/* from_row				= */ from_row,
		/* from_column			= */ from_column,
		/* to_row				= */ to_row,
		/* to_column			= */ to_column,
		/* rows					= */ rows,
		/* columns				= */ columns,
		/* destination_matrix	= */ (ceigen_matrix_t*) dst_matrix->implementation_handle
	);
}

void slime_magneto_multiply_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
) {
	// Multiply two matrices.
	ceigen_multiply_matrix(
		/* left_matrix			= */ (ceigen_matrix_t*) left_matrix	->implementation_handle,
		/* right_matrix			= */ (ceigen_matrix_t*) right_matrix->implementation_handle,
		/* destination_matrix	= */ (ceigen_matrix_t*) dst_matrix	->implementation_handle
	);
}

void slime_magneto_add_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
) {
	// Add two matrices.
	ceigen_add_matrix(
		/* left_matrix			= */ (ceigen_matrix_t*) left_matrix	->implementation_handle,
		/* right_matrix			= */ (ceigen_matrix_t*) right_matrix->implementation_handle,
		/* destination_matrix	= */ (ceigen_matrix_t*) dst_matrix	->implementation_handle
	);
}

void slime_magneto_subtract_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
) {
	// Subtract two matrices.
	ceigen_subtract_matrix(
		/* left_matrix			= */ (ceigen_matrix_t*) left_matrix	->implementation_handle,
		/* right_matrix			= */ (ceigen_matrix_t*) right_matrix->implementation_handle,
		/* destination_matrix	= */ (ceigen_matrix_t*) dst_matrix	->implementation_handle
	);
}

void slime_magneto_invert_matrix_in_place(
	magneto_matrix_t* src_matrix
) {
	// Invert the matrix in place.
	ceigen_invert_matrix_in_place((ceigen_matrix_t*) src_matrix->implementation_handle);
}

void slime_magneto_transpose_matrix_in_place(
	magneto_matrix_t* src_matrix
) {
	// Transpose the matrix in place.
	ceigen_transpose_matrix_in_place((ceigen_matrix_t*) src_matrix->implementation_handle);
}

void slime_magneto_normalize_matrix_in_place(
	magneto_matrix_t* src_matrix
) {
	// Normalize all column vectors of the matrix in place.
	ceigen_normalize_matrix_in_place((ceigen_matrix_t*) src_matrix->implementation_handle);
}

void slime_magneto_multiply_scalar_in_place(
			magneto_matrix_t*	src_matrix,
	const	float_t				value
) {
	// Multiply the matrix with a scalar in place.
	ceigen_multiply_matrix_scalar_in_place((ceigen_matrix_t*) src_matrix->implementation_handle,value);
}

uint8_t slime_magneto_solve_matrix_eigen(
	magneto_matrix_t* src_matrix,
	magneto_matrix_t* dst_eigenvectors_real,
	magneto_matrix_t* dst_eigenvectors_imag,
	magneto_matrix_t* dst_eigenvalues_real,
	magneto_matrix_t* dst_eigenvalues_imag
) {
	// Calculate the eigenvectors and eigenvalues.
	return ceigen_solve_matrix_eigen(
		/* source_matrix					= */ (ceigen_matrix_t*) src_matrix				->implementation_handle,
		/* destination_eigenvectors_real	= */ (ceigen_matrix_t*) dst_eigenvectors_real	->implementation_handle,
		/* destination_eigenvectors_imag	= */ (ceigen_matrix_t*) dst_eigenvectors_imag	->implementation_handle,
		/* destination_eigenvalues_real		= */ (ceigen_matrix_t*) dst_eigenvalues_real	->implementation_handle,
		/* destination_eigenvalues_imag		= */ (ceigen_matrix_t*) dst_eigenvalues_imag	->implementation_handle
	);
}