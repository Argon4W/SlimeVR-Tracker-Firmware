#include <eigen3/Eigen/Dense>
#include <ceigen.h>
#include <ceigen_matrix.h>

ceigen_matrix::ceigen_matrix(
	const uint32_t rows,
	const uint32_t cols
): impl(
	/* x = */ rows,
	/* y = */ cols
) {
	// Initialize all values in the matrix to zero.
	impl.setZero();
}

#ifdef __cplusplus
extern "C" {
#endif

ceigen_matrix_handle_t ceigen_new_matrix(
	const uint32_t rows,
	const uint32_t columns
) {
	// Create the handle with given size of the matrix.
	return new ceigen_matrix(
		/* rows		= */ rows,
		/* columns	= */ columns
	);
}

void ceigen_delete_matrix(
	const ceigen_matrix_handle_t matrix
) {
	// Free the handle.
	delete matrix;
}

uint32_t ceigen_get_matrix_rows(
	const ceigen_matrix_handle_t matrix
) {
	// Get the count of rows of the matrix.
	return matrix->impl.rows();
}

uint32_t ceigen_get_matrix_columns(
	const ceigen_matrix_handle_t matrix
) {
	// Get the count of columns of the matrix.
	return matrix->impl.cols();
}

float_t ceigen_get_matrix_coefficient(
	const ceigen_matrix_handle_t	matrix,
	const uint32_t					row,
	const uint32_t					column
) {
	// Get the value of the element at given row and column.
	return matrix->impl(row, column);
}

void ceigen_set_matrix_coefficient(
	const ceigen_matrix_handle_t	matrix,
	const uint32_t					row,
	const uint32_t					column,
	const float_t					value
) {
	// Set the value of the element at given row and column.
	matrix->impl(row, column) = value;
}

void ceigen_add_matrix_coefficient(
	const ceigen_matrix_handle_t	matrix,
	const uint32_t					row,
	const uint32_t					column,
	const float_t					value
) {
	// Add the value to the existing value of the element at given row and column.
	matrix->impl(row, column) += value;
}

void ceigen_multiply_matrix_coefficient(
	const ceigen_matrix_handle_t	matrix,
	const uint32_t					row,
	const uint32_t					column,
	const float_t					value
) {
	// Multiply the value to the existing value of the element at given row and column.
	matrix->impl(row, column) *= value;
}

void ceigen_copy_matrix(
	const ceigen_matrix_handle_t source_matrix,
	const ceigen_matrix_handle_t destination_matrix
) {
	// Copy the values in the source matrix to the destination matrix.
	destination_matrix->impl = source_matrix->impl;
}

void ceigen_copy_matrix_block(
	const ceigen_matrix_handle_t	source_matrix,
	const uint32_t					from_row,
	const uint32_t					from_column,
	const uint32_t					to_row,
	const uint32_t					to_column,
	const uint32_t					rows,
	const uint32_t					columns,
	const ceigen_matrix_handle_t	destination_matrix
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

void ceigen_add_matrix(
	const ceigen_matrix_handle_t left_matrix,
	const ceigen_matrix_handle_t right_matrix,
	const ceigen_matrix_handle_t destination_matrix
) {
	// Add the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl + right_matrix->impl).eval();
}

void ceigen_accumulate_matrix(
	const float_t					source_weight,
	const ceigen_matrix_handle_t	source_matrix,
	const ceigen_matrix_handle_t	destination_matrix
) {
	// Multiply the source matrix with the scalar weight, accumulate the result to the destination matrix.
	destination_matrix->impl += (source_weight * source_matrix->impl).eval();
}

void ceigen_subtract_matrix(
	const ceigen_matrix_handle_t left_matrix,
	const ceigen_matrix_handle_t right_matrix,
	const ceigen_matrix_handle_t destination_matrix
) {
	// Subtract the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl - right_matrix->impl).eval();
}

void ceigen_multiply_matrix(
	const ceigen_matrix_handle_t left_matrix,
	const ceigen_matrix_handle_t right_matrix,
	const ceigen_matrix_handle_t destination_matrix
) {
	// Multiply the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl * right_matrix->impl).eval();
}

void ceigen_invert_matrix_in_place(
	const ceigen_matrix_handle_t source_matrix
) {
	// Evaluate the inverse value of the source matrix and write back the result to the source matrix.
	source_matrix->impl = source_matrix->impl.inverse().eval();
}

void ceigen_transpose_matrix_in_place(
	const ceigen_matrix_handle_t source_matrix
) {
	// Transpose the source matrix in place.
	source_matrix->impl.transposeInPlace();
}

void ceigen_normalize_matrix_in_place(
	const ceigen_matrix_handle_t source_matrix
) {
	// Normal all column vectors of the source matrix in place.
	source_matrix->impl.colwise().normalize();
}

void ceigen_multiply_matrix_scalar_in_place(
	const ceigen_matrix_handle_t	source_matrix,
	const float_t					value
) {
	// Multiply the source matrix with the scalar value in place.
	source_matrix->impl *= value;
}

void ceigen_set_matrix_zeros_in_place(
	const ceigen_matrix_handle_t source_matrix
) {
	// Set all coefficients to 0 in place.
	source_matrix->impl.setZero();
}

void ceigen_set_matrix_constants_in_place(
	const ceigen_matrix_handle_t	source_matrix,
	const float_t					value
) {
	// Set all coefficients to given constant value in place.
	source_matrix->impl.fill(value);
}

void ceigen_set_matrix_scaled_identity_in_place(
	const ceigen_matrix_handle_t	source_matrix,
	const float_t					scale
) {
	// Set the source matrix to identity matrix, then multiply the matrix with given scale factor.
	source_matrix->impl.setIdentity();
	source_matrix->impl *= scale;
}

uint8_t ceigen_solve_matrix_eigen(
	const ceigen_matrix_handle_t source_matrix,
	const ceigen_matrix_handle_t destination_eigenvectors_real,
	const ceigen_matrix_handle_t destination_eigenvectors_imag,
	const ceigen_matrix_handle_t destination_eigenvalues_real,
	const ceigen_matrix_handle_t destination_eigenvalues_imag
) {
	// Solve the eigenvectors and eigenvalues.
	const Eigen::EigenSolver<ceigen_matrix_impl> solver(source_matrix->impl);

	// Write the result to the struct only if it is successful.
	if (solver.info() == Eigen::Success) {
		// Get the reference of the packed eigenvectors and eigenvalues.
		const auto& eigenvectors	= solver.eigenvectors	();
		const auto& eigenvalues		= solver.eigenvalues	();

		// Write them to the matrix array in the struct.
		for (uint32_t i = 0; i < source_matrix->impl.rows(); i ++) {
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

float_t ceigen_get_vector_norm(
	const ceigen_matrix_handle_t source_vector
) {
	// Treat the matrix as 3-dimensional column vectors then calculate the norm.
	return source_vector->impl.col(0).norm();
}

float_t ceigen_get_vector_squared_norm(
	const ceigen_matrix_handle_t source_vector
) {
	// Treat the matrix as 3-dimensional column vectors then calculate the squared norm.
	return source_vector->impl.col(0).squaredNorm();
}

void ceigen_vector_cross_product3(
	const ceigen_matrix_handle_t left_vector,
	const ceigen_matrix_handle_t right_vector,
	const ceigen_matrix_handle_t destination_vector
) {
	// Treat the matrices as 3-dimensional column vectors then calculate the cross product, write the result to the destination vector.
	destination_vector->impl.col(0).head<3>() = (left_vector->impl.col(0).head<3>().cross(right_vector->impl.col(0).head<3>())).eval();
}

void ceigen_clip_vector_in_place(
	const ceigen_matrix_handle_t	source_vector,
	const float_t					min_value,
	const float_t					max_value
) {
	// Treat the matrix as a vector then clip all components of the vector to the given range.
	source_vector->impl.col(0).array() = source_vector->impl.col(0).array().cwiseMax(min_value).cwiseMin(max_value);
}

#ifdef __cplusplus
}
#endif