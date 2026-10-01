#include <eigen3/Eigen/Dense>
#include <ceigen.h>
#include <ceigen_matrix.h>

ceigen_matrix_double::ceigen_matrix_double(
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

ceigen_matrix_double_handle_t ceigen_new_matrix_double(
	const uint32_t rows,
	const uint32_t columns
) {
	// Create the handle.
	return new ceigen_matrix_double(
		/* rows		= */ rows,
		/* columns	= */ columns
	);
}

void ceigen_delete_matrix_double(
	const ceigen_matrix_double_handle_t matrix
) {
	// Free the handle.
	delete matrix;
}

uint32_t ceigen_get_matrix_double_rows(
	const ceigen_matrix_double_handle_t source_matrix
) {
	// Get the count of rows of the source matrix.
	return source_matrix->impl.rows();
}

uint32_t ceigen_get_matrix_double_columns(
	const ceigen_matrix_double_handle_t source_matrix
) {
	// Get the count of columns of the source matrix.
	return source_matrix->impl.cols();
}

double_t ceigen_get_matrix_double_min(
	const ceigen_matrix_double_handle_t source_matrix
) {
	// Get the value of the smallest coefficient of the source matrix.
	return source_matrix->impl.minCoeff();
}

double_t ceigen_get_matrix_double_max(
	const ceigen_matrix_double_handle_t source_matrix
) {
	// Get the value of the largest coefficient of the source matrix.
	return source_matrix->impl.maxCoeff();
}

uint8_t ceigen_is_matrix_double_zeros(
	const ceigen_matrix_double_handle_t source_matrix
) {
	// Check if all coefficients of the source matrix are 0.
	return source_matrix->impl.isZero();
}

uint8_t ceigen_is_matrix_double_all_greater_than(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the smallest coefficient is greater than the value, then all coefficients should be greater than the value.
	return source_matrix->impl.minCoeff() > value;
}

uint8_t ceigen_is_matrix_double_all_greater_than_or_equal_to(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the smallest coefficient is greater than or equal to the value, then all coefficients should be greater than or equal to than the value.
	return source_matrix->impl.minCoeff() >= value;
}

uint8_t ceigen_is_matrix_double_all_less_than(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the largest coefficient is less than the value, then all coefficients should be less than the value.
	return source_matrix->impl.maxCoeff() < value;
}

uint8_t ceigen_is_matrix_double_all_less_than_or_equal_to(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the largest coefficient is less than or equal to the value, then all coefficients should be less than or equal to than the value.
	return source_matrix->impl.maxCoeff() <= value;
}

uint8_t ceigen_is_matrix_double_any_greater_than(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the largest coefficient is greater than the value, than at least one (the largest one) coefficient is greater than the value.
	return source_matrix->impl.maxCoeff() > value;
}

uint8_t ceigen_is_matrix_double_any_greater_than_or_equal_to(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the largest coefficient is greater than or equal to the value, than at least one (the largest one) coefficient is greater than or equal to the value.
	return source_matrix->impl.maxCoeff() >= value;
}

uint8_t ceigen_is_matrix_double_any_less_than(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the smallest coefficient is less than the value, than at least one (the smallest one) coefficient is less than the value.
	return source_matrix->impl.minCoeff() < value;
}

uint8_t ceigen_is_matrix_double_any_less_than_or_equal_to(
	const ceigen_matrix_double_handle_t	source_matrix,
	const double_t						value
) {
	// If the smallest coefficient is less than or equal to the value, than at least one (the smallest one) coefficient is less than or equal to the value.
	return source_matrix->impl.minCoeff() <= value;
}

double_t ceigen_get_matrix_double_coefficient(
	const ceigen_matrix_double_handle_t	source_matrix,
	const uint32_t						row,
	const uint32_t						column
) {
	// Get the value of the element at given row and column.
	return source_matrix->impl(row, column);
}

void ceigen_set_matrix_double_coefficient(
	const ceigen_matrix_double_handle_t	destination_matrix,
	const uint32_t						row,
	const uint32_t						column,
	const double_t						value
) {
	// Set the value of the element at given row and column.
	destination_matrix->impl(row, column) = value;
}

void ceigen_add_matrix_double_coefficient(
	const ceigen_matrix_double_handle_t	destination_matrix,
	const uint32_t						row,
	const uint32_t						column,
	const double_t						value
) {
	// Add the value to the existing value of the element at given row and column.
	destination_matrix->impl(row, column) += value;
}

void ceigen_multiply_matrix_double_coefficient(
	const ceigen_matrix_double_handle_t	destination_matrix,
	const uint32_t						row,
	const uint32_t						column,
	const double_t						value
) {
	// Multiply the value to the existing value of the element at given row and column.
	destination_matrix->impl(row, column) *= value;
}

void ceigen_copy_matrix_double(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Copy the values in the source matrix to the destination matrix.
	destination_matrix->impl = source_matrix->impl;
}

void ceigen_copy_matrix_double_block(
	const ceigen_matrix_double_handle_t	source_matrix,
	const uint32_t						from_row,
	const uint32_t						from_column,
	const uint32_t						to_row,
	const uint32_t						to_column,
	const uint32_t						rows,
	const uint32_t						columns,
	const ceigen_matrix_double_handle_t	destination_matrix
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
	).eval();
}

void ceigen_add_matrix_double(
	const ceigen_matrix_double_handle_t left_matrix,
	const ceigen_matrix_double_handle_t right_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Add the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl + right_matrix->impl).eval();
}

void ceigen_accumulate_matrix_double(
	const double_t						source_weight,
	const ceigen_matrix_double_handle_t	source_matrix,
	const ceigen_matrix_double_handle_t	destination_matrix
) {
	// Multiply the source matrix with the scalar weight, accumulate the result to the destination matrix.
	destination_matrix->impl += (source_weight * source_matrix->impl).eval();
}

void ceigen_subtract_matrix_double(
	const ceigen_matrix_double_handle_t left_matrix,
	const ceigen_matrix_double_handle_t right_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Subtract the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl - right_matrix->impl).eval();
}

void ceigen_multiply_matrix_double(
	const ceigen_matrix_double_handle_t left_matrix,
	const ceigen_matrix_double_handle_t right_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Multiply the left matrix and the right matrix, write the result to the destination matrix.
	destination_matrix->impl = (left_matrix->impl * right_matrix->impl).eval();
}

void ceigen_clip_matrix_double(
	const double_t						min_value,
	const double_t						max_value,
	const ceigen_matrix_double_handle_t	source_matrix,
	const ceigen_matrix_double_handle_t	destination_matrix
) {
	// Clip all coefficients of the source matrix to the given range, write the result to the destination matrix.
	destination_matrix->impl.array() = source_matrix->impl.array().cwiseMax(min_value).cwiseMin(max_value).eval();
}

void ceigen_abs_matrix_double(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Calculate the coefficient-wise absolute value of all coefficients, write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.cwiseAbs();
}

void ceigen_invert_matrix_double(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Evaluate the inverse value of the source matrix and write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.inverse().eval();
}

void ceigen_transpose_matrix_double(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Transpose the source matrix and write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.transpose().eval();
}

void ceigen_normalize_matrix_double(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Normal all column vectors of the source matrix and write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.colwise().normalized().eval();
}

void ceigen_multiply_matrix_double_scalar(
	const double_t						value,
	const ceigen_matrix_double_handle_t	source_matrix,
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Multiply the source matrix with the scalar value and write the result to the destination matrix.
	destination_matrix->impl = (source_matrix->impl * value).eval();
}

void ceigen_set_matrix_double_zeros(
	const ceigen_matrix_double_handle_t destination_matrix
) {
	// Set all coefficients of the destination matrix to 0.
	destination_matrix->impl.setZero();
}

void ceigen_set_matrix_double_constants(
	const double_t						value,
	const ceigen_matrix_double_handle_t	destination_matrix
) {
	// Set all coefficients of the destination matrix to given constant value.
	destination_matrix->impl.fill(value);
}

void ceigen_set_matrix_double_scaled_identity(
	const double_t						scale,
	const ceigen_matrix_double_handle_t	destination_matrix
) {
	// Set the destination matrix to identity matrix, then multiply the destination matrix with given scale factor.
	destination_matrix->impl.setIdentity();
	destination_matrix->impl *= scale;
}

uint8_t ceigen_solve_matrix_double_eigen(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_eigenvectors_real,
	const ceigen_matrix_double_handle_t destination_eigenvectors_imag,
	const ceigen_matrix_double_handle_t destination_eigenvalues_real,
	const ceigen_matrix_double_handle_t destination_eigenvalues_imag
) {
	// Solve the eigenvectors and eigenvalues.
	const Eigen::EigenSolver<ceigen_matrix_double_impl> solver(source_matrix->impl);

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

double_t ceigen_get_vector_double_norm(
	const ceigen_matrix_double_handle_t source_vector
) {
	// Treat the matrix as n-dimensional column vectors then calculate the norm.
	return source_vector->impl.col(0).norm();
}

double_t ceigen_get_vector_double_squared_norm(
	const ceigen_matrix_double_handle_t source_vector
) {
	// Treat the matrix as n-dimensional column vectors then calculate the squared norm.
	return source_vector->impl.col(0).squaredNorm();
}

float_t ceigen_vector_double_dot_product(
	const ceigen_matrix_double_handle_t left_vector,
	const ceigen_matrix_double_handle_t right_vector
) {
	// Treat the matrices as n-dimensional column vectors then calculate the dot product.
	return left_vector->impl.col(0).dot(right_vector->impl.col(0));
}

void ceigen_vector_double_cross_product3(
	const ceigen_matrix_double_handle_t left_vector,
	const ceigen_matrix_double_handle_t right_vector,
	const ceigen_matrix_double_handle_t destination_vector
) {
	// Treat the matrices as 3-dimensional column vectors then calculate the cross product, write the result to the destination vector.
	destination_vector->impl.col(0).head<3>() = left_vector->impl.col(0).head<3>().cross(right_vector->impl.col(0).head<3>()).eval();
}

void ceigen_sum_matrix_double_column_vectors(
	const ceigen_matrix_double_handle_t source_matrix,
	const ceigen_matrix_double_handle_t destination_vector
) {
	// Sum all column vectors in the source matrix, write the result to the destination vector.
	destination_vector->impl.col(0) = source_matrix->impl.rowwise().sum().eval();
}

void ceigen_copy_matrix_to_matrix_double(
	const ceigen_matrix_handle_t		source_matrix,
	const ceigen_matrix_double_handle_t	destination_matrix
) {
	// Cast the type of the coefficients of source matrix to double_t then write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.cast<double_t>();
}

void ceigen_copy_matrix_double_to_matrix(
	const ceigen_matrix_double_handle_t	source_matrix,
	const ceigen_matrix_handle_t		destination_matrix
) {
	// Cast the type of the coefficients of source matrix to float_t then write the result to the destination matrix.
	destination_matrix->impl = source_matrix->impl.cast<float_t>();
}

#ifdef __cplusplus
}
#endif