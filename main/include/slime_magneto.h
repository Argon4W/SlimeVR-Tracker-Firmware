#ifndef SLIME_MAGNETO_H
#define SLIME_MAGNETO_H

#include "ceigen.h"
#include "magneto.h"
#include "slime_nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The key of the calibration data blob of magneto.
 */
extern const slime_nvs_blob_key_t slime_magneto_blob_key;

/**
 * @brief The linear algebra context of the Magneto.
 */
extern const magneto_linear_algebra_context_t slime_magneto_linear_algebra_context;

/**
 * @brief The blob struct of the calculated magneto calibration data.
 */
typedef struct {
	float_t magneto_soft_iron_matrix[3 * 3];	/*!< The soft iron matrix of the calibration data. */
	float_t magneto_hard_iron_vector[3 * 1];	/*!< The hard iron vector of the calibration data. */
	uint8_t magneto_calibrated;					/*!< True if the calibration data is valid. */
} slime_magneto_blob_t;

/**
 * @brief			Get the CEigen matrix implementation of the magneto matrix.
 * @param matrix	The magneto matrix to get the implementation CEigen matrix.
 * @return			The CEigen matrix of the magneto matrix.
 */
ceigen_matrix_t* slime_magneto_get_ceigen_matrix(
	magneto_matrix_t* matrix
);

/**
 * @brief			Create a new matrix.
 * @param rows		rows of the allocated matrix.
 * @param columns	columns of the allocated matrix.
 * @retval			the created matrix.
 */
magneto_matrix_t* slime_magneto_new_matrix(
	uint32_t rows,
	uint32_t columns
);

/**
 * @brief			Delete an existing matrix.
 * @param matrix	The matrix to be deleted.
 */
void slime_magneto_del_matrix(
	magneto_matrix_t* matrix
);

/**
 * @brief			Get the value of a coefficient in an existing matrix.
 * @param matrix	the matrix of the coefficient.
 * @param row		the row of the coefficient.
 * @param column	the column of the coefficient.
 * @retval			the value of the coefficient get from the matrix.
 */
float_t slime_magneto_get_matrix_coefficient(
	magneto_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column
);

/**
 * @brief			Set the value of a coefficient in an existing matrix.
 * @param matrix	the matrix of the coefficient.
 * @param row		the row of the coefficient.
 * @param column	the column of the coefficient.
 * @param value		the value of the coefficient.
 */
void slime_magneto_set_matrix_coefficient(
	magneto_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief			Add a value to the existing value of a coefficient in an existing matrix.
 * @param matrix	the matrix of the coefficient.
 * @param row		the row of the coefficient.
 * @param column	the column of the coefficient.
 * @param value		the value to be added to the coefficient.
 */
void slime_magneto_add_matrix_coefficient(
	magneto_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief			Multiply a value to the existing value of an coefficient in an existing matrix.
 * @param matrix	the matrix of the coefficient.
 * @param row		the row of the coefficient.
 * @param column	the column of the coefficient.
 * @param value		the value to be multiplied to the coefficient.
 */
void slime_magneto_multiply_matrix_coefficient(
	magneto_matrix_t*	matrix,
	uint32_t			row,
	uint32_t			column,
	float_t				value
);

/**
 * @brief				Copy values from an existing matrix to another matrix.
 * @param src_matrix	the source matrix to be copied.
 * @param dst_matrix	destination matrix the values are copied to.
 */
void slime_magneto_copy_matrix(
	magneto_matrix_t* src_matrix,
	magneto_matrix_t* dst_matrix
);

/**
 * @brief				Copy a block of matrix from an existing matrix to a block of another existing matrix.
 * @param src_matrix	the source matrix to get the sub matrix.
 * @param from_row		the start row of the block in the source matrix.
 * @param from_column	the start column of the block in the source matrix.
 * @param to_row		the start row of the block in the destination matrix.
 * @param to_column		the start column of the block in the destination matrix.
 * @param rows			the count of rows of the block.
 * @param columns		the count of columns of the block.
 * @param dst_matrix	destination matrix the block is copied to.
 */
void slime_magneto_copy_matrix_block(
	magneto_matrix_t*	src_matrix,
	uint32_t			from_row,
	uint32_t			from_column,
	uint32_t			to_row,
	uint32_t			to_column,
	uint32_t			rows,
	uint32_t			columns,
	magneto_matrix_t*	dst_matrix
);

/**
 * @brief				Multiply two existing matrices.
 * @param left_matrix	the left matrix of the multiplication.
 * @param right_matrix	the right matrix of the multiplication.
 * @param dst_matrix	destination matrix to hold the result matrix.
 */
void slime_magneto_multiply_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
);

/**
 * @brief				Add two existing matrices.
 * @param left_matrix	the left matrix of the addition.
 * @param right_matrix	the right matrix of the addition.
 * @param dst_matrix	destination matrix to hold the result matrix.
 */
void slime_magneto_add_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
);

/**
 * @brief				Subtract two existing matrices.
 * @param left_matrix	the left matrix of the subtraction.
 * @param right_matrix	the right matrix of the subtraction.
 * @param dst_matrix	destination matrix to hold the result matrix.
 */
void slime_magneto_subtract_matrix(
	magneto_matrix_t* left_matrix,
	magneto_matrix_t* right_matrix,
	magneto_matrix_t* dst_matrix
);

/**
 * @brief				Invert an existing matrix in place.
 * @param src_matrix	the matrix to be inverted.
 */
void slime_magneto_invert_matrix_in_place(
	magneto_matrix_t* src_matrix
);

/**
 * @brief				Transpose an existing matrix in place.
 * @param src_matrix	the matrix to be inverted.
 */
void slime_magneto_transpose_matrix_in_place(
	magneto_matrix_t* src_matrix
);

/**
 * @brief				Normalize all column vectors of an existing matrix in place.
 * @param src_matrix	the matrix to be normalized.
 */
void slime_magneto_normalize_matrix_in_place(
	magneto_matrix_t* src_matrix
);

/**
 * @brief				Multiply an existing matrix with a scalar in place.
 * @param src_matrix	the matrix to be multiplied with scalar.
 * @param value			the scalar to be multiplied to the matrix.
 */
void slime_magneto_multiply_scalar_in_place(
	magneto_matrix_t*	src_matrix,
	float_t				value
);

/**
 * @brief						Solve eigenvectors and eigenvalues of an existing square matrix.
 * @param src_matrix			the source square matrix to be solved.
 * @param dst_eigenvectors_real	the real part of the eigenvectors ass column vectors packed in the matrix.
 * @param dst_eigenvectors_imag	the imaginary part of the eigenvectors ass column vectors packed in the matrix.
 * @param dst_eigenvalues_real	the real part of the eigenvalues packed in the column vector.
 * @param dst_eigenvalues_imag	the imaginary part of the eigenvalues packed in the column vector.
 * @retval						the status of the eigen solving result.
 */
uint8_t slime_magneto_solve_matrix_eigen(
	magneto_matrix_t* src_matrix,
	magneto_matrix_t* dst_eigenvectors_real,
	magneto_matrix_t* dst_eigenvectors_imag,
	magneto_matrix_t* dst_eigenvalues_real,
	magneto_matrix_t* dst_eigenvalues_imag
);

#ifdef __cplusplus
}
#endif

#endif // SLIME_MAGNETO_H
