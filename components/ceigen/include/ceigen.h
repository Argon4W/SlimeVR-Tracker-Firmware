#ifndef CEIGEN_H
#define CEIGEN_H

#include "stdint.h"
#include "stddef.h"
#include "math.h"

/**
 * The maximum count of rows of all allocated matrices.
 */
#ifndef CEIGEN_MAX_ROWS
#define CEIGEN_MAX_ROWS 10
#endif // CEIGEN_MAX_ROWS

/**
 * The Maximum count of columns of all allocated matrices.
 */
#ifndef CEIGEN_MAX_COLS
#define CEIGEN_MAX_COLS 10
#endif // CEIGEN_MAX_COLS

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The opaque matrix handle type of CEigen.
 */
typedef struct ceigen_matrix* ceigen_matrix_handle_t;

/**
 * @brief The opaque double precision matrix type of CEigen.
 */
typedef struct ceigen_matrix_double* ceigen_matrix_double_handle_t;

/**
 * @brief The opaque double precision matrix type of CEigen.
 */
typedef struct ceigen_quaternion* ceigen_quaternion_handle_t;

/**
 * @brief			Create a new matrix. All values of the matrix is 0.0f when initialized.
 * @param rows		rows of the allocated matrix.
 * @param columns	columns of the allocated matrix.
 * @retval			the created matrix.
 */
ceigen_matrix_handle_t ceigen_new_matrix(
	uint32_t rows,
	uint32_t columns
);

/**
 * @brief			Delete an existing matrix.
 * @param matrix	The matrix to be deleted.
 */
void ceigen_delete_matrix(
	ceigen_matrix_handle_t matrix
);

/**
 * @brief				Get the count of rows of an existing matrix.
 * @param source_matrix	the source matrix to get the count of rows.
 * @return				count of rows of the matrix.
 */
uint32_t ceigen_get_matrix_rows(
	ceigen_matrix_handle_t source_matrix
);

/**
 * @brief				Get the count of columns of an existing matrix.
 * @param source_matrix	the source matrix to get the count of columns.
 * @return				count of columns of the matrix.
 */
uint32_t ceigen_get_matrix_columns(
	ceigen_matrix_handle_t source_matrix
);

/**
 * @brief				Get the value of the smallest coefficient of an existing matrix.
 * @param source_matrix	the source matrix of to get the value of the smallest coefficient.
 * @return				value of the smallest coefficient.
 */
float_t ceigen_get_matrix_min(
	ceigen_matrix_handle_t source_matrix
);

/**
 * @brief				Get the value of the largest coefficient of an existing matrix.
 * @param source_matrix	the source matrix of to get the value of the largest coefficient.
 * @return				value of the largest coefficient.
 */
float_t ceigen_get_matrix_max(
	ceigen_matrix_handle_t source_matrix
);

/**
 * @brief				Check if all coefficients of an existing matrix are 0.
 * @param source_matrix	the source matrix to check.
 * @return				true if all coefficients are 0.
 */
uint8_t ceigen_is_matrix_zeros(
	ceigen_matrix_handle_t source_matrix
);

/**
 * @brief				Compare and check if all coefficients of an existing matrix are greater than the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if all coefficients are greater than the given scalar.
 */
uint8_t ceigen_is_matrix_all_greater_than(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if all coefficients of an existing matrix are greater than or equal to the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if all coefficients are greater than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_all_greater_than_or_equal_to(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if all coefficients of an existing matrix are less than given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if all coefficients are less than the given scalar.
 */
uint8_t ceigen_is_matrix_all_less_than(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if all coefficients of an existing matrix are less than or equal to the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if all coefficients are less than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_all_less_than_or_equal_to(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if any coefficient of an existing matrix is greater than the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if any coefficient is greater than the given scalar.
 */
uint8_t ceigen_is_matrix_any_greater_than(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if any coefficient of an existing matrix is greater than or equal to the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if any coefficient is greater than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_any_greater_than_or_equal_to(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if any coefficient of an existing matrix is less than given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if any coefficient is less than the given scalar.
 */
uint8_t ceigen_is_matrix_any_less_than(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Compare and check if any coefficient of an existing matrix is less than or equal to the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if any coefficient is less than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_any_less_than_or_equal_to(
	ceigen_matrix_handle_t	source_matrix,
	float_t					value
);

/**
 * @brief				Get the value of an element in an existing matrix.
 * @param source_matrix	the source matrix of the element.
 * @param row			the row of the element.
 * @param column		the column of the element.
 * @retval				the value of the element get from the matrix.
 */
float_t ceigen_get_matrix_coefficient(
	ceigen_matrix_handle_t	source_matrix,
	uint32_t				row,
	uint32_t				column
);

/**
 * @brief						Set the value of an element in an existing matrix.
 * @param destination_matrix	the destination matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value of the element.
 */
void ceigen_set_matrix_coefficient(
	ceigen_matrix_handle_t	destination_matrix,
	uint32_t				row,
	uint32_t				column,
	float_t					value
);

/**
 * @brief						Add a value to the existing value of an element in an existing matrix.
 * @param destination_matrix	the destination matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value to be added to the element.
 */
void ceigen_add_matrix_coefficient(
	ceigen_matrix_handle_t	destination_matrix,
	uint32_t				row,
	uint32_t				column,
	float_t					value
);

/**
 * @brief						Multiply a value to the existing value of an element in an existing matrix.
 * @param destination_matrix	the destination matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value to be multiplied to the element.
 */
void ceigen_multiply_matrix_coefficient(
	ceigen_matrix_handle_t	destination_matrix,
	uint32_t				row,
	uint32_t				column,
	float_t					value
);

/**
 * @brief						Copy values from an existing matrix to another matrix.
 * @param source_matrix			the source matrix to be copied.
 * @param destination_matrix	destination matrix the values are copied to.
 */
void ceigen_copy_matrix(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Copy a block of matrix from an existing matrix to a block of another existing matrix.
 * @param source_matrix			the source matrix to get the sub matrix.
 * @param from_row				the start row of the block in the source matrix.
 * @param from_column			the start column of the block in the source matrix.
 * @param to_row				the start row of the block in the destination matrix.
 * @param to_column				the start column of the block in the destination matrix.
 * @param rows					the count of rows of the block.
 * @param columns				the count of columns of the sub block.
 * @param destination_matrix	destination matrix the block is copied to.
 */
void ceigen_copy_matrix_block(
	ceigen_matrix_handle_t	source_matrix,
	uint32_t				from_row,
	uint32_t				from_column,
	uint32_t				to_row,
	uint32_t				to_column,
	uint32_t				rows,
	uint32_t				columns,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief						Add two existing matrices. (destination_matrix = left_matrix + right_matrix)
 * @param left_matrix			the left matrix of the addition.
 * @param right_matrix			the right matrix of the addition.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_add_matrix(
	ceigen_matrix_handle_t left_matrix,
	ceigen_matrix_handle_t right_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Accumulate an existing matrix into another existing matrix. (destination_matrix += weight * source_matrix)
 * @param source_matrix			the source matrix to be accumulated.
 * @param source_weight			the weight of the source matrix to be accumulated.
 * @param destination_matrix	destination matrix to accumulate the weighted matrix.
 */
void ceigen_accumulate_matrix(
	float_t					source_weight,
	ceigen_matrix_handle_t	source_matrix,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief						Subtract two existing matrices. (destination_matrix = left_matrix - right_matrix)
 * @param left_matrix			the left matrix of the subtraction.
 * @param right_matrix			the right matrix of the subtraction.
 * @param destination_matrix	destination matrix to hold the result matrix.
*/
void ceigen_subtract_matrix(
	ceigen_matrix_handle_t left_matrix,
	ceigen_matrix_handle_t right_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Multiply two existing matrices. (destination_matrix = left_matrix * right_matrix)
 * @param left_matrix			the left matrix of the multiplication.
 * @param right_matrix			the right matrix of the multiplication.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_multiply_matrix(
	ceigen_matrix_handle_t left_matrix,
	ceigen_matrix_handle_t right_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Clip the coefficients of an existing matrix.
 * @param min_value				The minimal allowed value of the coefficients.
 * @param max_value				The maximum allowed value of the coefficients.
 * @param source_matrix			The source matrix to be clipped.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_clip_matrix(
	float_t					min_value,
	float_t					max_value,
	ceigen_matrix_handle_t	source_matrix,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief						Get the coefficient-wise absolute value of all coefficients of an existing matrix.
 * @param source_matrix			the source matrix to get the absolute value.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_abs_matrix(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Invert an existing matrix.
 * @param source_matrix			the source matrix to be inverted.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_invert_matrix(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Transpose an existing matrix.
 * @param source_matrix			the source matrix to be transposed.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_transpose_matrix(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Normalize all column vectors of an existing matrix.
 * @param source_matrix			the source matrix to be normalized.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_normalize_matrix(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Multiply an existing matrix with a scalar.
 * @param value					the scalar to be multiplied to the matrix.
 * @param source_matrix			the source matrix to be multiplied with scalar.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_multiply_matrix_scalar(
	float_t					value,
	ceigen_matrix_handle_t	source_matrix,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief						Set values of all coefficients of an existing matrix to 0.
 * @param destination_matrix	destination matrix to be set to zeros.
 */
void ceigen_set_matrix_zeros(
	ceigen_matrix_handle_t destination_matrix
);

/**
 * @brief						Set all coefficients of an existing matrix with given constant scalar.
 * @param value					the scalar that all coefficients of the matrix will be set to.
 * @param destination_matrix	destination matrix to be set to constants.
 */
void ceigen_set_matrix_constants(
	float_t					value,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief						Set an existing matrix to scaled identity matrix.
 * @param scale					the scale of the matrix.
 * @param destination_matrix	destination matrix to be set to scaled identity matrix.
 */
void ceigen_set_matrix_scaled_identity(
	float_t					scale,
	ceigen_matrix_handle_t	destination_matrix
);

/**
 * @brief								Solve eigenvectors and eigenvalues of an existing square matrix.
 * @attention							destination_eigenvalues_real and destination_eigenvalues_imag are column vectors (n rows 1 col matrices).
 * @param source_matrix					the source square matrix to be solved.
 * @param destination_eigenvectors_real	the real part of the eigenvectors ass column vectors packed in the matrix. The i-th column vector is the real part of the i-th eigenvector.
 * @param destination_eigenvectors_imag	the imaginary part of the eigenvectors ass column vectors packed in the matrix. The i-th column vector is the imaginary part of the i-th eigenvector.
 * @param destination_eigenvalues_real	the real part of the eigenvalues packed in the column vector. The i-th value is the real part of the i-th eigenvalue.
 * @param destination_eigenvalues_imag	the imaginary part of the eigenvalues packed in the column vector. The i-th value is the imaginary part of the i-th eigenvalue.
 * @retval								the status of the eigen solving result. ("0" = successful, other value = failed).
 */
uint8_t ceigen_solve_matrix_eigen(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_eigenvectors_real,
	ceigen_matrix_handle_t destination_eigenvectors_imag,
	ceigen_matrix_handle_t destination_eigenvalues_real,
	ceigen_matrix_handle_t destination_eigenvalues_imag
);

/**
 * @brief				Treat an existing matrix as an n-dimensional vector then calculate the norm.
 * @param source_vector	The source vector to calculate the norm.
 * @retval				the norm.
 */
float_t ceigen_get_vector_norm(
	ceigen_matrix_handle_t source_vector
);

/**
 * @brief				Treat an existing matrix as an n-dimensional vector then calculate the squared norm.
 * @param source_vector	The source vector to calculate the squared norm.
 * @retval				the squared norm.
 */
float_t ceigen_get_vector_squared_norm(
	ceigen_matrix_handle_t source_vector
);

/**
 * @brief						Treat two existing matrices as n-dimensional column vectors, then calculate the dot product.
 *								(destination_vector = dot(left_vector, right_vector))
 * @param left_vector			the left vector of the dot product.
 * @param right_vector			the right vector of the dot product
 * @return						the dot product.
 */
float_t ceigen_vector_dot_product(
	ceigen_matrix_handle_t left_vector,
	ceigen_matrix_handle_t right_vector
);

/**
 * @brief						Treat two existing matrices as 3-dimensional column vectors, then calculate the cross product.
 *								(destination_vector = cross(left_vector, right_vector))
 * @param left_vector			the left vector of the cross product.
 * @param right_vector			the right vector of the cross product.
 * @param destination_vector	destination vector to hold the result vector.
 */
void ceigen_vector_cross_product3(
	ceigen_matrix_handle_t left_vector,
	ceigen_matrix_handle_t right_vector,
	ceigen_matrix_handle_t destination_vector
);

/**
 * @brief						Sum all column vectors of an existing matrix together.
 * @param source_matrix			The source matrix to calculate the sum of all column vectors.
 * @param destination_vector	destination vector to store the result vector.
 */
void ceigen_sum_matrix_column_vectors(
	ceigen_matrix_handle_t source_matrix,
	ceigen_matrix_handle_t destination_vector
);

/**
 * @brief			Create a new double precision matrix. All values of the double precision matrix is 0.0f when initialized.
 * @param rows		rows of the allocated double precision matrix.
 * @param columns	columns of the allocated double precision matrix.
 * @retval			the created double precision matrix.
 */
ceigen_matrix_double_handle_t ceigen_new_matrix_double(
	uint32_t rows,
	uint32_t columns
);

/**
 * @brief			Delete an existing double precision matrix.
 * @param matrix	The double precision matrix to be deleted.
 */
void ceigen_delete_matrix_double(
	ceigen_matrix_double_handle_t matrix
);

/**
 * @brief			Get the count of rows of an existing double precision matrix.
 * @param source_matrix	the source double precision matrix to get the count of rows.
 * @return			count of rows of the double precision matrix.
 */
uint32_t ceigen_get_matrix_double_rows(
	ceigen_matrix_double_handle_t source_matrix
);

/**
 * @brief				Get the count of columns of an existing double precision matrix.
 * @param source_matrix	the source double precision matrix to get the count of columns.
 * @return				count of columns of the double precision matrix.
 */
uint32_t ceigen_get_matrix_double_columns(
	ceigen_matrix_double_handle_t source_matrix
);

/**
 * @brief				Get the value of the smallest coefficient of an existing double precision matrix.
 * @param source_matrix	the source double precision matrix of to get the value of the smallest coefficient.
 * @return				value of the smallest coefficient.
 */
double_t ceigen_get_matrix_double_min(
	ceigen_matrix_double_handle_t source_matrix
);

/**
 * @brief				Get the value of the largest coefficient of an existing double precision matrix.
 * @param source_matrix	the source double precision matrix of to get the value of the largest coefficient.
 * @return				value of the largest coefficient.
 */
double_t ceigen_get_matrix_double_max(
	ceigen_matrix_double_handle_t source_matrix
);

/**
 * @brief				Check if all coefficients of an existing double precision matrix are 0.
 * @param source_matrix	the source double precision matrix to check.
 * @return				true if all coefficients are 0.
 */
uint8_t ceigen_is_matrix_double_zeros(
	ceigen_matrix_double_handle_t source_matrix
);

/**
 * @brief				Compare and check if all coefficients of an existing double precision matrix are greater than the given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if all coefficients are greater than the given scalar.
 */
uint8_t ceigen_is_matrix_double_all_greater_than(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if all coefficients of an existing double precision matrix are greater than or equal to the given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if all coefficients are greater than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_double_all_greater_than_or_equal_to(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if all coefficients of an existing double precision matrix are less than given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if all coefficients are less than the given scalar.
 */
uint8_t ceigen_is_matrix_double_all_less_than(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if all coefficients of an existing double precision matrix are less than or equal to the given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if all coefficients are less than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_double_all_less_than_or_equal_to(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if any coefficient of an existing double precision matrix is greater than the given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if any coefficient is greater than the given scalar.
 */
uint8_t ceigen_is_matrix_double_any_greater_than(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if any coefficient of an existing matrix is greater than or equal to the given scalar.
 * @param source_matrix	the source matrix to compare.
 * @param value			the scalar to be compared to the matrix.
 * @return				true if any coefficient is greater than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_double_any_greater_than_or_equal_to(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if any coefficient of an existing double precision matrix is less than given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if any coefficient is less than the given scalar.
 */
uint8_t ceigen_is_matrix_double_any_less_than(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Compare and check if any coefficient of an existing double precision matrix is less than or equal to the given double precision scalar.
 * @param source_matrix	the source double precision matrix to compare.
 * @param value			the double precision scalar to be compared to the matrix.
 * @return				true if any coefficient is less than or equal to the given scalar.
 */
uint8_t ceigen_is_matrix_double_any_less_than_or_equal_to(
	ceigen_matrix_double_handle_t	source_matrix,
	double_t						value
);

/**
 * @brief				Get the value of an element in an existing double precision matrix.
 * @param source_matrix	the source double precision matrix of the element.
 * @param row			the row of the element.
 * @param column		the column of the element.
 * @retval				the value of the element get from the double precision matrix.
 */
double_t ceigen_get_matrix_double_coefficient(
	ceigen_matrix_double_handle_t	source_matrix,
	uint32_t						row,
	uint32_t						column
);

/**
 * @brief						Set the value of an element in an existing double precision matrix.
 * @param destination_matrix	the destination double precision matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value of the element.
 */
void ceigen_set_matrix_double_coefficient(
	ceigen_matrix_double_handle_t	destination_matrix,
	uint32_t						row,
	uint32_t						column,
	double_t						value
);

/**
 * @brief						Add a value to the existing value of an element in an existing double precision matrix.
 * @param destination_matrix	the destination double precision matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value to be added to the element.
 */
void ceigen_add_matrix_double_coefficient(
	ceigen_matrix_double_handle_t	destination_matrix,
	uint32_t						row,
	uint32_t						column,
	double_t						value
);

/**
 * @brief						Multiply a value to the existing value of an element in an existing double precision matrix.
 * @param destination_matrix	the double precision matrix of the element.
 * @param row					the row of the element.
 * @param column				the column of the element.
 * @param value					the value to be multiplied to the element.
 */
void ceigen_multiply_matrix_double_coefficient(
	ceigen_matrix_double_handle_t	destination_matrix,
	uint32_t						row,
	uint32_t						column,
	double_t						value
);

/**
 * @brief						Copy values from an existing double precision matrix to another double precision matrix.
 * @param source_matrix			the source double precision matrix to be copied.
 * @param destination_matrix	destination double precision matrix the values are copied to.
 */
void ceigen_copy_matrix_double(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Copy a block of double precision matrix from an existing double precision matrix to a block of another existing double precision matrix.
 * @param source_matrix			the source double precision matrix to get the sub matrix.
 * @param from_row				the start row of the block in the source double precision matrix.
 * @param from_column			the start column of the block in the source double precision matrix.
 * @param to_row				the start row of the block in the destination double precision matrix.
 * @param to_column				the start column of the block in the destination double precision matrix.
 * @param rows					the count of rows of the block.
 * @param columns				the count of columns of the sub block.
 * @param destination_matrix	destination double precision matrix the block is copied to.
 */
void ceigen_copy_matrix_double_block(
	ceigen_matrix_double_handle_t	source_matrix,
	uint32_t						from_row,
	uint32_t						from_column,
	uint32_t						to_row,
	uint32_t						to_column,
	uint32_t						rows,
	uint32_t						columns,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Add two double precision existing matrices. (destination_matrix = left_matrix + right_matrix)
 * @param left_matrix			the left double precision matrix of the addition.
 * @param right_matrix			the right double precision matrix of the addition.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_add_matrix_double(
	ceigen_matrix_double_handle_t left_matrix,
	ceigen_matrix_double_handle_t right_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Accumulate an existing double precision matrix into another existing double precision matrix. (destination_matrix += weight * source_matrix)
 * @param source_matrix			the source double precision matrix to be accumulated.
 * @param source_weight			the weight of the source double precision matrix to be accumulated.
 * @param destination_matrix	destination double precision matrix to accumulate the weighted matrix.
 */
void ceigen_accumulate_matrix_double(
	double_t						source_weight,
	ceigen_matrix_double_handle_t	source_matrix,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Subtract two existing double precision matrices. (destination_matrix = left_matrix - right_matrix)
 * @param left_matrix			the left double precision matrix of the subtraction.
 * @param right_matrix			the right double precision matrix of the subtraction.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
*/
void ceigen_subtract_matrix_double(
	ceigen_matrix_double_handle_t left_matrix,
	ceigen_matrix_double_handle_t right_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Multiply two existing double precision matrices. (destination_matrix = left_matrix * right_matrix)
 * @param left_matrix			the left double precision matrix of the multiplication.
 * @param right_matrix			the right double precision matrix of the multiplication.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_multiply_matrix_double(
	ceigen_matrix_double_handle_t left_matrix,
	ceigen_matrix_double_handle_t right_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Clip the coefficients of an existing double precision matrix.
 * @param min_value				The minimal allowed value of the coefficients.
 * @param max_value				The maximum allowed value of the coefficients.
 * @param source_matrix			The source double precision matrix to be clipped.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_clip_matrix_double(
	double_t						min_value,
	double_t						max_value,
	ceigen_matrix_double_handle_t	source_matrix,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Get the coefficient-wise absolute value of an existing double precision matrix.
 * @param source_matrix			the source double precision matrix to get the absolute value.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_abs_matrix_double(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Invert an existing double precision matrix.
 * @param source_matrix			the source double precision matrix to be inverted.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_invert_matrix_double(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Transpose an existing double precision matrix.
 * @param source_matrix			the source double precision matrix to be transposed.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_transpose_matrix_double(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Normalize all column vectors of an existing double precision matrix.
 * @param source_matrix			the source double precision matrix to be normalized.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_normalize_matrix_double(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Multiply an existing double precision matrix with a scalar.
 * @param value					the double precision scalar to be multiplied to the matrix.
 * @param source_matrix			the source double precision matrix to be multiplied with scalar.
 * @param destination_matrix	destination double precision matrix to hold the result matrix.
 */
void ceigen_multiply_matrix_double_scalar(
	double_t						value,
	ceigen_matrix_double_handle_t	source_matrix,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Set values of all coefficients of an existing double precision matrix to 0.
 * @param destination_matrix	destination double precision matrix to be set to zeros.
 */
void ceigen_set_matrix_double_zeros(
	ceigen_matrix_double_handle_t destination_matrix
);

/**
 * @brief						Set all coefficients of an existing double precision matrix with given constant scalar.
 * @param value					the double precision scalar that all coefficients of the double precision matrix will be set to.
 * @param destination_matrix	destination double precision matrix to be set to constants.
 */
void ceigen_set_matrix_double_constants(
	double_t						value,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Set an existing double precision matrix to scaled identity matrix.
 * @param scale					the scale of the double precision matrix.
 * @param destination_matrix	destination double precision matrix to be set to scaled identity matrix.
 */
void ceigen_set_matrix_double_scaled_identity(
	double_t						scale,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief								Solve eigenvectors and eigenvalues of an existing double precision square matrix.
 * @attention							destination_eigenvalues_real and destination_eigenvalues_imag are column vectors (n rows 1 col matrices).
 * @param source_matrix					the source double precision square matrix to be solved.
 * @param destination_eigenvectors_real	the real part of the eigenvectors ass column vectors packed in the double precision matrix. The i-th column vector is the real part of the i-th eigenvector.
 * @param destination_eigenvectors_imag	the imaginary part of the eigenvectors ass column vectors packed in the double precision matrix. The i-th column vector is the imaginary part of the i-th eigenvector.
 * @param destination_eigenvalues_real	the real part of the eigenvalues packed in the double precision column vector. The i-th value is the real part of the i-th eigenvalue.
 * @param destination_eigenvalues_imag	the imaginary part of the eigenvalues packed in the double precision column vector. The i-th value is the imaginary part of the i-th eigenvalue.
 * @retval								the status of the eigen solving result. ("0" = successful, other value = failed).
 */
uint8_t ceigen_solve_matrix_double_eigen(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_eigenvectors_real,
	ceigen_matrix_double_handle_t destination_eigenvectors_imag,
	ceigen_matrix_double_handle_t destination_eigenvalues_real,
	ceigen_matrix_double_handle_t destination_eigenvalues_imag
);

/**
 * @brief				Treat an existing double precision matrix as an n-dimensional double precision vector then calculate the norm.
 * @param source_vector	The source double precision vector to calculate the norm.
 * @retval				the norm.
 */
double_t ceigen_get_vector_double_norm(
	ceigen_matrix_double_handle_t source_vector
);

/**
 * @brief				Treat an existing double precision matrix as an n-dimensional double precision vector then calculate the squared norm.
 * @param source_vector	The source double precision vector to calculate the squared norm.
 * @retval				the squared norm.
 */
double_t ceigen_get_vector_double_squared_norm(
	ceigen_matrix_double_handle_t source_vector
);

/**
 * @brief						Treat two existing double precision matrices as n-dimensional column vectors, then calculate the dot product.
 *								(destination_vector = dot(left_vector, right_vector))
 * @param left_vector			the left double precision vector of the dot product.
 * @param right_vector			the right double precision vector of the dot product
 * @return						the dot product.
 */
float_t ceigen_vector_double_dot_product(
	ceigen_matrix_double_handle_t left_vector,
	ceigen_matrix_double_handle_t right_vector
);

/**
 * @brief						Treat two existing double precision matrices as 3-dimensional double precision column vectors, then calculate the cross product.
 *								(destination_vector = cross(left_vector, right_vector))
 * @param left_vector			the left double precision vector of the cross product.
 * @param right_vector			the right double precision vector of the cross product.
 * @param destination_vector	destination double precision vector to hold the result vector.
 */
void ceigen_vector_double_cross_product3(
	ceigen_matrix_double_handle_t left_vector,
	ceigen_matrix_double_handle_t right_vector,
	ceigen_matrix_double_handle_t destination_vector
);

/**
 * @brief						Sum all column vectors of an existing matrix together.
 * @param source_matrix			The source matrix to calculate the sum of all column vectors.
 * @param destination_vector	destination vector to store the result vector.
 */
void ceigen_sum_matrix_double_column_vectors(
	ceigen_matrix_double_handle_t source_matrix,
	ceigen_matrix_double_handle_t destination_vector
);

/**
 * @brief						Copy coefficients of an existing matrix to another existing double precision matrix.
 * @param source_matrix			the source matrix to be copied.
 * @param destination_matrix	destination double precision matrix coefficients are copied to.
 */
void ceigen_copy_matrix_to_matrix_double(
	ceigen_matrix_handle_t			source_matrix,
	ceigen_matrix_double_handle_t	destination_matrix
);

/**
 * @brief						Copy coefficients of an existing double precision matrix to another existing matrix.
 * @param source_matrix			the source double precision matrix to be copied.
 * @param destination_matrix	destination matrix coefficients are copied to.
 */
void ceigen_copy_matrix_double_to_matrix(
	ceigen_matrix_double_handle_t	source_matrix,
	ceigen_matrix_handle_t			destination_matrix
);

/**
 * @brief	Create a new quaternion. Quaternion should be identity quaternion when initialized.
 * @retval	the created quaternion.
 */
ceigen_quaternion_handle_t ceigen_new_quaternion();

/**
 * @brief				Delete an existing quaternion.
 * @param quaternion	The quaternion to be deleted.
 */
void ceigen_delete_quaternion(
	ceigen_quaternion_handle_t quaternion
);

/**
 * @brief	Get the value of the W in an existing quaternion.
 * @retval	the value of the W get from the quaternion.
 */
float_t ceigen_get_quaternion_w(
	ceigen_quaternion_handle_t quaternion
);

/**
 * @brief	Get the value of the X in an existing quaternion.
 * @retval	the value of the X get from the quaternion.
 */
float_t ceigen_get_quaternion_x(
	ceigen_quaternion_handle_t quaternion
);

/**
 * @brief	Get the value of the Y in an existing quaternion.
 * @retval	the value of the Y get from the quaternion.
 */
float_t ceigen_get_quaternion_y(
	ceigen_quaternion_handle_t quaternion
);

/**
 * @brief	Get the value of the Z in an existing quaternion.
 * @retval	the value of the Z get from the quaternion.
 */
float_t ceigen_get_quaternion_z(
	ceigen_quaternion_handle_t quaternion
);

/**
 * @brief				Set the value of the W in an existing quaternion.
 * @param quaternion	the quaternion to set the W.
 * @param value			the value of the new W.
 */
void ceigen_set_quaternion_w(
	ceigen_quaternion_handle_t	quaternion,
	float_t						value
);

/**
 * @brief				Set the value of the X in an existing quaternion.
 * @param quaternion	the quaternion to set the X.
 * @param value			the value of the new X.
 */
void ceigen_set_quaternion_x(
	ceigen_quaternion_handle_t	quaternion,
	float_t						value
);

/**
 * @brief				Set the value of the Y in an existing quaternion.
 * @param quaternion	the quaternion to set the Y.
 * @param value			the value of the new Y.
 */
void ceigen_set_quaternion_y(
	ceigen_quaternion_handle_t	quaternion,
	float_t						value
);

/**
 * @brief				Set the value of the Z in an existing quaternion.
 * @param quaternion	the quaternion to set the Z.
 * @param value			the value of the new Z.
 */
void ceigen_set_quaternion_z(
	ceigen_quaternion_handle_t	quaternion,
	float_t						value
);

/**
 * @brief							Copy coefficients of an existing quaternion to another quaternion.
 * @param source_quaternion			the source quaternion to be copied.
 * @param destination_quaternion	destination quaternion coefficients are copied to.
 */
void ceigen_copy_quaternion(
	ceigen_quaternion_handle_t source_quaternion,
	ceigen_quaternion_handle_t destination_quaternion
);

/**
 * @brief					Calculate the dot product of two existing quaternions. (return = left_quaternion dot right_quaternion)
 * @param left_quaternion	the left quaternion of the dot product.
 * @param right_quaternion	the right quaternion of the dot product.
 * @return					The result of the dot product.
 */
float_t ceigen_dot_quaternion(
	ceigen_quaternion_handle_t left_quaternion,
	ceigen_quaternion_handle_t right_quaternion
);

/**
 * @brief							Multiply two existing quaternions. (destination_quaternion = left_quaternion * right_quaternion)
 * @param left_quaternion			the left quaternion of the multiplication.
 * @param right_quaternion			the right quaternion of the multiplication.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_multiply_quaternion(
	ceigen_quaternion_handle_t left_quaternion,
	ceigen_quaternion_handle_t right_quaternion,
	ceigen_quaternion_handle_t destination_quaternion
);

/**
 * @brief							Slerp from from_quaternion to to_quaternion. (destination_quaternion = slerp(from_quaternion, to_quaternion, factor)
 * @attention						Both from_quaternion and to_quaternion are assumed to be unit quaternions.
 * @param from_quaternion			The start/from quaternion of the interpolation.
 * @param to_quaternion				The end/to quaternion of the interpolation.
 * @param factor					The interpolation factor.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_slerp_quaternion(
	ceigen_quaternion_handle_t	from_quaternion,
	ceigen_quaternion_handle_t	to_quaternion,
	float_t						factor,
	ceigen_quaternion_handle_t	destination_quaternion
);

/**
 * @brief							Rotate an existing quaternion by given radians around X axis. (destination_quaternion = [cos(rotation_x_radians/2), sin(rotation_x_radians/2), 0, 0] * src_quaternion)
 * @param rotation_x_radians		the rotation around X axis in radians.
 * @param src_quaternion			the source quaternion to be rotated.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_rotate_quaternion_around_x(
	float_t						rotation_x_radians,
	ceigen_quaternion_handle_t	src_quaternion,
	ceigen_quaternion_handle_t	destination_quaternion
);

/**
 * @brief							Rotate an existing quaternion by given radians around Y axis. (destination_quaternion = [cos(rotation_y_radians/2), 0, sin(rotation_y_radians/2), 0] * src_quaternion)
 * @param rotation_y_radians		the rotation around Y axis in radians.
 * @param src_quaternion			the source quaternion to be rotated.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_rotate_quaternion_around_y(
	float_t						rotation_y_radians,
	ceigen_quaternion_handle_t	src_quaternion,
	ceigen_quaternion_handle_t	destination_quaternion
);

/**
 * @brief							Rotate an existing quaternion by given radians around Z axis. (destination_quaternion = [cos(rotation_z_radians/2), 0, 0, sin(rotation_z_radians/2)] * src_quaternion)
 * @param src_quaternion			the source quaternion to be rotated.
 * @param rotation_z_radians		the rotation around Z axis in radians.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_rotate_quaternion_around_z(
	float_t						rotation_z_radians,
	ceigen_quaternion_handle_t	src_quaternion,
	ceigen_quaternion_handle_t	destination_quaternion
);

/**
 * @brief							Set the coefficients of an existing quaternion to a given rotation.
 * @attention						rotation_axis is assumed to be unit vector.
 * @param rotation_angle_radians	The angle of the rotation in radians.
 * @param rotation_axis				the axis of the rotation, treat the matrix as a 3-dimensional vector.
 * @param destination_quaternion	destination quaternion to hold the result rotation quaternion.
 */
void ceigen_set_quaternion_rotation(
	float_t						rotation_angle_radians,
	ceigen_matrix_handle_t		rotation_axis,
	ceigen_quaternion_handle_t	destination_quaternion
);

/**
 * @brief						Treat an existing matrix as a 3-dimensional vector then rotate the vector with a given quaternion. (destination_vector = quaternion * src_vector * (quaternion^{-1}))
 * @param src_vector			the source vector to be rotated.
 * @param quaternion			the quaternion to rotate the vector.
 * @param destination_vector	destination vector to hold the result vector.
 */
void ceigen_quaternion_rotate_vector(
	ceigen_matrix_handle_t		src_vector,
	ceigen_quaternion_handle_t	quaternion,
	ceigen_matrix_handle_t		destination_vector
);

/**
 * @brief						Convert an existing quaternion to rotation matrix.
 * @param src_quaternion		the source quaternion to be converted into rotation matrix.
 * @param destination_matrix	destination matrix to hold the result rotation matrix.
 */
void ceigen_quaternion_to_rotation_matrix(
	ceigen_quaternion_handle_t	src_quaternion,
	ceigen_matrix_handle_t		destination_matrix
);

/**
 * @brief							Normalize an existing quaternion.
 * @param source_quaternion			the source quaternion to be normalized.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_normalize_quaternion(
	ceigen_quaternion_handle_t source_quaternion,
	ceigen_quaternion_handle_t destination_quaternion
);

/**
 * @brief							Conjugate an existing quaternion.
 * @param source_quaternion			the source quaternion to be conjugated.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_conjugate_quaternion(
	ceigen_quaternion_handle_t source_quaternion,
	ceigen_quaternion_handle_t destination_quaternion
);

/**
 * @brief							Invert an existing quaternion.
 * @param source_quaternion			the source quaternion to be inverted.
 * @param destination_quaternion	destination quaternion to hold the result quaternion.
 */
void ceigen_inverse_quaternion(
	ceigen_quaternion_handle_t source_quaternion,
	ceigen_quaternion_handle_t destination_quaternion
);

/**
 * @brief							Set an existing quaternion to identity quaternion.
 * @param destination_quaternion	destination quaternion to be set to identity quaternion.
 */
void ceigen_set_quaternion_identity(
	ceigen_quaternion_handle_t destination_quaternion
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CEIGEN_H
