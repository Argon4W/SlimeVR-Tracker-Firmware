#ifndef CEIGEN_H
#define CEIGEN_H

#include "stdint.h"
#include "stddef.h"
#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief The opaque wrapped matrix type of CEigen.
 */
typedef struct ceigen_matrix ceigen_matrix_t;

/**
 * @brief			Creating a new matrix. All values of the matrix is 0.0f when initialized.
 * @param rows		rows of the allocated matrix.
 * @param columns	columns of the allocated matrix.
 * @retval			the created matrix.
 */
ceigen_matrix_t* ceigen_new_matrix(
	uint32_t rows,
	uint32_t columns
);

/**
 * @brief			Delete an existing matrix.
 * @param matrix	The matrix to be deleted.
 */
void ceigen_delete_matrix(
	const ceigen_matrix_t* matrix
);

/**
 * @brief			Get the count of rows of the matrix.
 * @param matrix	the matrix to get the count of rows.
 * @return			count of rows of the matrix.
 */
uint32_t ceigen_get_matrix_rows(
	const ceigen_matrix_t* matrix
);

/**
 * @brief			Get the count of columns of the matrix.
 * @param matrix	the matrix to get the count of columns.
 * @return			count of columns of the matrix.
 */
uint32_t ceigen_get_matrix_columns(
	const ceigen_matrix_t* matrix
);

/**
 * @brief			Get the value of an element in an existing matrix.
 * @param matrix	the matrix of the element.
 * @param row		the row of the element.
 * @param column	the column of the element.
 * @retval			the value of the element get from the matrix.
 */
float_t ceigen_get_matrix_coefficient(
	const	ceigen_matrix_t*	matrix,
			uint32_t			row,
			uint32_t			column
);

/**
 * @brief			Set the value of an element in an existing matrix.
 * @param matrix	the matrix of the element.
 * @param row		the row of the element.
 * @param column	the column of the element.
 * @param value		the value of the element.
 */
void ceigen_set_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief			Add a value to the existing value of an element in an existing matrix.
 * @param matrix	the matrix of the element.
 * @param row		the row of the element.
 * @param column	the column of the element.
 * @param value		the value to be added to the element.
 */
void ceigen_add_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief			Multiply a value to the existing value of an element in an existing matrix.
 * @param matrix	the matrix of the element.
 * @param row		the row of the element.
 * @param column	the column of the element.
 * @param value		the value to be multiplied to the element.
 */
void ceigen_multiply_matrix_coefficient(
	ceigen_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief						Copy values from an existing matrix to another matrix.
 * @param source_matrix			the source matrix to be copied.
 * @param destination_matrix	destination matrix the values are copied to.
 */
void ceigen_copy_matrix(
	const	ceigen_matrix_t* source_matrix,
			ceigen_matrix_t* destination_matrix
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
	const	ceigen_matrix_t*	source_matrix,
			uint32_t			from_row,
			uint32_t			from_column,
			uint32_t			to_row,
			uint32_t			to_column,
			uint32_t			rows,
			uint32_t			columns,
			ceigen_matrix_t*	destination_matrix
);

/**
 * @brief						Multiply two existing matrices. (destination_matrix = left_matrix * right_matrix)
 * @param left_matrix			the left matrix of the multiplication.
 * @param right_matrix			the right matrix of the multiplication.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_multiply_matrix(
	const	ceigen_matrix_t* left_matrix,
	const	ceigen_matrix_t* right_matrix,
			ceigen_matrix_t* destination_matrix
);

/**
 * @brief						Add two existing matrices. (destination_matrix = left_matrix + right_matrix)
 * @param left_matrix			the left matrix of the addition.
 * @param right_matrix			the right matrix of the addition.
 * @param destination_matrix	destination matrix to hold the result matrix.
 */
void ceigen_add_matrix(
	const	ceigen_matrix_t* left_matrix,
	const	ceigen_matrix_t* right_matrix,
			ceigen_matrix_t* destination_matrix
);

/**
 * @brief						Subtract two existing matrices. (destination_matrix = left_matrix - right_matrix)
 * @param left_matrix			the left matrix of the subtraction.
 * @param right_matrix			the right matrix of the subtraction.
 * @param destination_matrix	destination matrix to hold the result matrix.
*/
void ceigen_subtract_matrix(
	const	ceigen_matrix_t* left_matrix,
	const	ceigen_matrix_t* right_matrix,
			ceigen_matrix_t* destination_matrix
);

/**
 * @brief						Treat two existing matrices as 3-dimension column vectors, then calculate the cross product.
 *								(destination_vector = cross(left_vector, right_vector))
 * @param left_vector			the left vector of the cross product.
 * @param right_vector			the right vector of the cross product.
 * @param destination_vector	destination vector to hold the result vector.
 */
void ceigen_cross_product3(
	const	ceigen_matrix_t* left_vector,
	const	ceigen_matrix_t* right_vector,
			ceigen_matrix_t* destination_vector
);

/**
 * @brief				Invert an existing matrix in place.
 * @param source_matrix	the matrix to be inverted.
 */
void ceigen_invert_matrix_in_place(
	ceigen_matrix_t* source_matrix
);

/**
 * @brief				Transpose an existing matrix in place.
 * @param source_matrix	the matrix to be transposed.
 */
void ceigen_transpose_matrix_in_place(
	ceigen_matrix_t* source_matrix
);

/**
 * @brief				Normalize all column vectors of an existing matrix in place.
 * @param source_matrix	the matrix to be normalized.
 */
void ceigen_normalize_matrix_in_place(
	ceigen_matrix_t* source_matrix
);

/**
 * @brief				Multiply an existing matrix with a scalar in place.
 * @param source_matrix	the matrix to be multiplied with scalar.
 * @param value			the scalar to be multiplied to the matrix.
 */
void ceigen_multiply_matrix_scalar_in_place(
	ceigen_matrix_t*	source_matrix,
	float_t				value
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
	const	ceigen_matrix_t* source_matrix,
			ceigen_matrix_t* destination_eigenvectors_real,
			ceigen_matrix_t* destination_eigenvectors_imag,
			ceigen_matrix_t* destination_eigenvalues_real,
			ceigen_matrix_t* destination_eigenvalues_imag
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CEIGEN_H
