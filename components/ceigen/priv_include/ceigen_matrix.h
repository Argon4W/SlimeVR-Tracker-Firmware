//
// Created by progr on 2026/9/27.
//

#ifndef CEIGEN_MATRIX_H
#define CEIGEN_MATRIX_H

#include <eigen3/Eigen/Dense>

/**
 * @brief The dynamic Eigen matrix type used in the CEigen.
 */
typedef Eigen::Matrix<
	/* _Scalar	= */ float_t,
	/* _Rows	= */ Eigen::Dynamic,
	/* _Cols	= */ Eigen::Dynamic,
	/* _Options	= */ Eigen::RowMajor,
	/* _MaxRows	= */ CEIGEN_MAX_ROWS,
	/* _MaxCols	= */ CEIGEN_MAX_COLS
> ceigen_matrix_impl;

/**
 * @brief The dynamic Eigen matrix type used in the CEigen.
 */
typedef Eigen::Matrix<
	/* _Scalar	= */ double_t,
	/* _Rows	= */ Eigen::Dynamic,
	/* _Cols	= */ Eigen::Dynamic,
	/* _Options	= */ Eigen::RowMajor,
	/* _MaxRows	= */ CEIGEN_MAX_ROWS,
	/* _MaxCols	= */ CEIGEN_MAX_COLS
> ceigen_matrix_double_impl;

/**
 * @brief The implementation detail of the ceigen_matrix struct.
 */
struct ceigen_matrix {
	/**
	 * @brief Dynamic eigen matrix inside the struct, not visible to C.
	 */
	ceigen_matrix_impl impl;

	/**
	 * @brief		Constructor of the struct for initializing the impl.
	 * @param rows	rows of the impl matrix.
	 * @param cols	columns of the impl matrix.
	 */
	ceigen_matrix(
		uint32_t rows,
		uint32_t cols
	);

	/**
	 * @brief Default deconstructor of the struct for freeing the dynamic eigen matrix inside the struct.
	 */
	~ceigen_matrix() = default;
};

/**
 * @brief The implementation detail of the ceigen_matrix_double struct.
 */
struct ceigen_matrix_double {
	/**
	 * @brief Dynamic double precision eigen matrix inside the struct, not visible to C.
	 */
	ceigen_matrix_double_impl impl;

	/**
	 * @brief		Constructor of the struct for initializing the impl.
	 * @param rows	rows of the impl matrix.
	 * @param cols	columns of the impl matrix.
	 */
	ceigen_matrix_double(
		uint32_t rows,
		uint32_t cols
	);

	/**
	 * @brief Default deconstructor of the struct for freeing the dynamic double precision eigen matrix inside the struct.
	 */
	~ceigen_matrix_double() = default;
};

#endif // CEIGEN_MATRIX_H
