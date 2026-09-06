#include <eigen3/Eigen/Dense>
#include <ceigen.h>

using namespace Eigen;

/**
 * @brief The matrix type we used in the CEigen.
 */
typedef Matrix<float, Dynamic, Dynamic, RowMajor> ceigen_matrix_impl;

/**
 * @brief The implementation detail of the ceigen_matrix wrapper struct.
 */
struct ceigen_matrix {
	/**
	 * @brief Real Eigen matrix wrapped by the struct.
	 */
	ceigen_matrix_impl impl;

	/**
	 * @brief		Constructor of the wrapper struct for initializing the impl.
	 * @param rows	rows of the impl matrix.
	 * @param cols	columns of the impl matrix.
	 */
	ceigen_matrix(
		const int32_t rows,
		const int32_t cols
	): impl(rows, cols) {
		// Initialize all values in the matrix to zero.
		impl.setZero();
	}

	/**
	 * @brief Default deconstructor of the wrapper struct for freeing the real matrix wrapped by the struct.
	 */
	~ceigen_matrix() = default;
};

#ifdef __cplusplus
extern "C" {
#endif

ceigen_matrix_t* ceigen_new_matrix(
	const int32_t rows,
	const int32_t columns
) {
	// Create the struct with given size of the matrix.
	return new ceigen_matrix(
		/* rows		= */ rows,
		/* columns	= */ columns
	);
}

void ceigen_delete_matrix(
	const ceigen_matrix_t* matrix
) {
	// Free the wrapper struct and the matrix inside the struct.
	delete matrix;
}

int32_t ceigen_get_matrix_rows(
	const ceigen_matrix_t* matrix
) {
	return matrix->impl.rows();
}

int32_t ceigen_get_matrix_columns(
	const ceigen_matrix_t* matrix
) {
	return matrix->impl.cols();
}

float_t ceigen_get_matrix_coefficient(
	const ceigen_matrix_t*	matrix,
	const int32_t			row,
	const int32_t			column
) {
	// Get the value of the element at given row and column.
	return matrix->impl(row, column);
}

void ceigen_set_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	const int32_t		row,
	const int32_t		column,
	const float_t		value
) {
	// Set the value of the element at given row and column.
	matrix->impl(row, column) = value;
}

void ceigen_add_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	const int32_t		row,
	const int32_t		column,
	const float_t		value
) {
	// Add the value to the existing value of the element at given row and column.
	matrix->impl(row, column) += value;
}

void ceigen_multiply_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	const int32_t		row,
	const int32_t		column,
	const float_t		value
) {
	// Multiply the value to the existing value of the element at given row and column.
	matrix->impl(row, column) *= value;
}

void ceigen_copy_matrix(
	const ceigen_matrix_t*	source_matrix,
	ceigen_matrix_t*		destination_matrix
) {
	// Copy the values in the source matrix to the destination.
	destination_matrix->impl = source_matrix->impl;
}

void ceigen_copy_matrix_block(
	ceigen_matrix_t*	source_matrix,
	const int32_t		from_row,
	const int32_t		from_column,
	const int32_t		to_row,
	const int32_t		to_column,
	const int32_t		rows,
	const int32_t		columns,
	ceigen_matrix_t*	destination_matrix
) {
	// Copy a block of the source matrix to a block of the destination matrix.
	destination_matrix->impl.block(
		/* startRow		= */ to_row,
		/* startCol		= */ to_column,
		/* blockRows	= */ rows,
		/* blockCols	= */ columns
	) = source_matrix->impl.block(
		/* startRow		= */ from_row,
		/* startCol		= */ from_column,
		/* blockRows	= */ rows,
		/* blockCols	= */ columns
	);
}

void ceigen_multiply_matrix(
	const ceigen_matrix_t*	left_matrix,
	const ceigen_matrix_t*	right_matrix,
	ceigen_matrix_t*		destination_matrix
) {
	// Multiply the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = left_matrix->impl * right_matrix->impl;
}

void ceigen_add_matrix(
	const ceigen_matrix_t*	left_matrix,
	const ceigen_matrix_t*	right_matrix,
	ceigen_matrix_t*		destination_matrix
) {
	// Add the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = left_matrix->impl + right_matrix->impl;
}

void ceigen_subtract_matrix(
	const ceigen_matrix_t*	left_matrix,
	const ceigen_matrix_t*	right_matrix,
	ceigen_matrix_t*		destination_matrix
) {
	// Subtract the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = left_matrix->impl - right_matrix->impl;
}

void ceigen_invert_matrix_in_place(
	ceigen_matrix_t* source_matrix
) {
	// Evaluate the inverse value of the source matrix and write back the result to the source matrix.
	source_matrix->impl = source_matrix->impl.inverse().eval();
}

void ceigen_transpose_matrix_in_place(
	ceigen_matrix_t* source_matrix
) {
	// Transpose the source matrix in place.
	source_matrix->impl.transposeInPlace();
}

void ceigen_normalize_matrix_in_place(
	ceigen_matrix_t* source_matrix
) {
	// Normal all column vectors of the source matrix in place.
	source_matrix->impl.colwise().normalize();
}

void ceigen_multiply_matrix_scalar_in_place(
	ceigen_matrix_t*	source_matrix,
	const float_t		value
) {
	// Multiply the source matrix with the scalar value in place.
	source_matrix->impl *= value;
}

uint8_t ceigen_solve_matrix_eigen(
	const ceigen_matrix_t*	source_matrix,
	ceigen_matrix_t*		destination_eigenvectors_real,
	ceigen_matrix_t*		destination_eigenvectors_imag,
	ceigen_matrix_t*		destination_eigenvalues_real,
	ceigen_matrix_t*		destination_eigenvalues_imag
) {
	// Solve the eigenvectors and eigenvalues.
	const EigenSolver<ceigen_matrix_impl> solver(source_matrix->impl);

	// Write the result to the struct only if it is successful.
	if (solver.info() == Success) {
		// Get the reference of the packed eigenvectors and eigenvalues.
		const auto& eigenvectors	= solver.eigenvectors	();
		const auto& eigenvalues		= solver.eigenvalues	();

		// Write them to the matrix array in the struct.
		for (int32_t i = 0; i < source_matrix->impl.rows(); i ++) {
			// Write the eigenvector.
			destination_eigenvectors_real->impl.col(i) = eigenvectors.col(i).real(); // First column is the real part of the eigenvector.
			destination_eigenvectors_imag->impl.col(i) = eigenvectors.col(i).imag(); // Second column is the imagine part of the eigenvector.

			// Write the eigenvalue.
			destination_eigenvalues_real->impl(i, 0) = eigenvalues(i).real(); // First value is the real part of the eigenvalue.
			destination_eigenvalues_imag->impl(i, 0) = eigenvalues(i).imag(); // Second value is the imagine part of the eigenvalue.
		}
	}

	return solver.info();
}

#ifdef __cplusplus
}
#endif