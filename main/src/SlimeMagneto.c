#include "SlimeMagneto.h"

ptr(magneto_matrix_t) newMatrixCEigen(
	const s32 rows,
	const s32 columns
) {
	// Allocate the magneto matrix struct wrapper.
	ptr(magneto_matrix_t) matrix = alloc_ptr(magneto_matrix_t);

	// Fill the implementation handle with the ceigen matrix.
	matrix->implementation_handle = cast_to(opaque, ceigen_new_matrix(rows, columns));

	return matrix;
}

void deleteMatrixCEigen(
	ptr(magneto_matrix_t) matrix
) {
	// Free the CEigen matrix in the implementation handle.
	ceigen_delete_matrix(cast_to(ptr(ceigen_matrix_t), matrix->implementation_handle));
	// Then we can safely free the magneto struct wrapper.
	free(matrix);
}

float_t getMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	const s32				row,
	const s32				column
) {
	// Set the value of the element.
	return ceigen_get_matrix_coefficient(
		/* matrix	= */ cast_to(ptr(ceigen_matrix_t), matrix->implementation_handle),
		/* row		= */ row,
		/* column	= */ column
	);
}

void setMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	const s32				row,
	const s32				column,
	const float_t			value
) {
	// Set the value of the element.
	ceigen_set_matrix_coefficient(
		/* matrix	= */ cast_to(ptr(ceigen_matrix_t), matrix->implementation_handle),
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void addMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	const s32				row,
	const s32				column,
	const float_t			value
) {
	// Add a value to the value of the element.
	ceigen_add_matrix_coefficient(
		/* matrix	= */ cast_to(ptr(ceigen_matrix_t), matrix->implementation_handle),
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void multiplyMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	const s32				row,
	const s32				column,
	const float_t			value
) {
	// Multiply a value to the value of the element.
	ceigen_multiply_matrix_coefficient(
		/* matrix	= */ cast_to(ptr(ceigen_matrix_t), matrix->implementation_handle),
		/* row		= */ row,
		/* column	= */ column,
		/* value	= */ value
	);
}

void copyMatrixCEigen(
	ptr(magneto_matrix_t) sourceMatrix,
	ptr(magneto_matrix_t) destinationMatrix
) {
	// Copy the values from the source matrix to the destination matrix.
	ceigen_copy_matrix(
		/* source_matrix		= */ cast_to(ptr(ceigen_matrix_t), sourceMatrix			->implementation_handle),
		/* destination_matrix	= */ cast_to(ptr(ceigen_matrix_t), destinationMatrix	->implementation_handle)
	);
}

void copyMatrixBlockCEigen(
	ptr(magneto_matrix_t)	sourceMatrix,
	const s32				fromRow,
	const s32				fromColumn,
	const s32				toRow,
	const s32				toColumn,
	const s32				rows,
	const s32				columns,
	ptr(magneto_matrix_t)	destinationMatrix
) {
	// Copy a block of the source matrix to a block the destination matrix.
	ceigen_copy_matrix_block(
		/* source_matrix		= */ cast_to(ptr(ceigen_matrix_t), sourceMatrix->implementation_handle),
		/* from_row				= */ fromRow,
		/* from_column			= */ fromColumn,
		/* to_row				= */ toRow,
		/* to_column			= */ toColumn,
		/* rows					= */ rows,
		/* columns				= */ columns,
		/* destination_matrix	= */ cast_to(ptr(ceigen_matrix_t), destinationMatrix->implementation_handle)
	);
}

void multiplyMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
) {
	// Multiply two matrices.
	ceigen_multiply_matrix(
		/* left_matrix			= */ cast_to(ptr(ceigen_matrix_t), leftMatrix			->implementation_handle),
		/* right_matrix			= */ cast_to(ptr(ceigen_matrix_t), rightMatrix			->implementation_handle),
		/* destination_matrix	= */ cast_to(ptr(ceigen_matrix_t), destinationMatrix	->implementation_handle)
	);
}

void addMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
) {
	// Add two matrices.
	ceigen_add_matrix(
		/* left_matrix			= */ cast_to(ptr(ceigen_matrix_t), leftMatrix			->implementation_handle),
		/* right_matrix			= */ cast_to(ptr(ceigen_matrix_t), rightMatrix			->implementation_handle),
		/* destination_matrix	= */ cast_to(ptr(ceigen_matrix_t), destinationMatrix	->implementation_handle)
	);
}

void subtractMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
) {
	// Subtract two matrices.
	ceigen_subtract_matrix(
		/* left_matrix			= */ cast_to(ptr(ceigen_matrix_t), leftMatrix			->implementation_handle),
		/* right_matrix			= */ cast_to(ptr(ceigen_matrix_t), rightMatrix			->implementation_handle),
		/* destination_matrix	= */ cast_to(ptr(ceigen_matrix_t), destinationMatrix	->implementation_handle)
	);
}

void invertMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
) {
	// Invert the matrix in place.
	ceigen_invert_matrix_in_place(cast_to(ptr(ceigen_matrix_t), sourceMatrix->implementation_handle));
}

void transposeMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
) {
	// Transpose the matrix in place.
	ceigen_transpose_matrix_in_place(cast_to(ptr(ceigen_matrix_t), sourceMatrix->implementation_handle));
}

void normalizeMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
) {
	// Normalize all column vectors of the matrix in place.
	ceigen_normalize_matrix_in_place(cast_to(ptr(ceigen_matrix_t), sourceMatrix->implementation_handle));
}

void multiplyMatrixScalarInPlaceCEigen(
	ptr(magneto_matrix_t)	sourceMatrix,
	const float_t			value
) {
	// Multiply the matrix with a scalar in place.
	ceigen_multiply_matrix_scalar_in_place(cast_to(ptr(ceigen_matrix_t), sourceMatrix->implementation_handle),value);
}

u8 solveMatrixEigenCEigen(
	ptr(magneto_matrix_t) sourceMatrix,
	ptr(magneto_matrix_t) destinationEigenvectorsReal,
	ptr(magneto_matrix_t) destinationEigenvectorsImag,
	ptr(magneto_matrix_t) destinationEigenvaluesReal,
	ptr(magneto_matrix_t) destinationEigenvaluesImag
) {
	// Calculate the eigenvectors and eigenvalues.
	return ceigen_solve_matrix_eigen(
		/* source_matrix					= */ cast_to(ptr(ceigen_matrix_t), sourceMatrix					->implementation_handle),
		/* destination_eigenvectors_real	= */ cast_to(ptr(ceigen_matrix_t), destinationEigenvectorsReal	->implementation_handle),
		/* destination_eigenvectors_imag	= */ cast_to(ptr(ceigen_matrix_t), destinationEigenvectorsImag	->implementation_handle),
		/* destination_eigenvalues_real		= */ cast_to(ptr(ceigen_matrix_t), destinationEigenvaluesReal	->implementation_handle),
		/* destination_eigenvalues_imag		= */ cast_to(ptr(ceigen_matrix_t), destinationEigenvaluesImag	->implementation_handle)
	);
}