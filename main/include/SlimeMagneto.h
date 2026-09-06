#ifndef SLIME_MAGNETO_H
#define SLIME_MAGNETO_H

#include "ceigen.h"
#include "magneto.h"
#include "SlimeCommon.h"

#ifdef __cplusplus
extern "C" {
#endif

// Implementation function of creating a new matrix using CEigen.
ptr(magneto_matrix_t) newMatrixCEigen(
	s32 rows,
	s32 columns
);

// Implementation function of deleting an existing matrix using CEigen.
void deleteMatrixCEigen(
	ptr(magneto_matrix_t) matrix
);

// Implementation function of getting the value of an element in an existing matrix using CEigen.
float_t getMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	s32						row,
	s32						column
);

// Implementation function of setting the value of an element in an existing matrix using CEigen.
void setMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	s32						row,
	s32						column,
	float_t					value
);

// Implementation function of adding a value to the existing value of an element in an existing matrix using CEigen.
void addMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	s32						row,
	s32						column,
	float_t					value
);

// Implementation function of multiplying a value to the existing value of an element in an existing matrix using CEigen.
void multiplyMatrixCoefficientCEigen(
	ptr(magneto_matrix_t)	matrix,
	s32						row,
	s32						column,
	float_t					value
);

// Implementation function of copying values from an existing matrix using CEigen.
void copyMatrixCEigen(
	ptr(magneto_matrix_t) sourceMatrix,
	ptr(magneto_matrix_t) destinationMatrix
);

// Implementation function of copying a block of matrix from an existing matrix to a block of another existing matrix using CEigen.
void copyMatrixBlockCEigen(
	ptr(magneto_matrix_t)	sourceMatrix,
	s32						fromRow,
	s32						fromColumn,
	s32						toRow,
	s32						toColumn,
	s32						rows,
	s32						columns,
	ptr(magneto_matrix_t)	destinationMatrix
);

// Implementation function of multiplying two existing matrices using CEigen.
void multiplyMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
);

// Implementation function of adding two existing matrices using CEigen.
void addMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
);

// Implementation function of subtracting two existing matrices using CEigen.
void subtractMatrixCEigen(
	ptr(magneto_matrix_t) leftMatrix,
	ptr(magneto_matrix_t) rightMatrix,
	ptr(magneto_matrix_t) destinationMatrix
);

// Implementation function of inverting an existing matrix in place using CEigen.
void invertMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
);

// Implementation function of transposing an existing matrix in place using CEigen.
void transposeMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
);

// Implementation function of normalizing all column vectors of an existing matrix in place using CEigen.
void normalizeMatrixInPlaceCEigen(
	ptr(magneto_matrix_t) sourceMatrix
);

// Implementation function of multiplying an existing matrix with a scalar in place using CEigen.
void multiplyMatrixScalarInPlaceCEigen(
	ptr(magneto_matrix_t)	sourceMatrix,
	float_t					value
);

// Implementation function of solving eigenvectors and eigenvalues of an existing square matrix using CEigen.
u8 solveMatrixEigenCEigen(
	ptr(magneto_matrix_t) sourceMatrix,
	ptr(magneto_matrix_t) destinationEigenvectorsReal,
	ptr(magneto_matrix_t) destinationEigenvectorsImag,
	ptr(magneto_matrix_t) destinationEigenvaluesReal,
	ptr(magneto_matrix_t) destinationEigenvaluesImag
);

#ifdef __cplusplus
}
#endif

#endif // SLIME_MAGNETO_H
