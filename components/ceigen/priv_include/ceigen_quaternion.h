//
// Created by progr on 2026/9/27.
//

#ifndef CEIGEN_QUATERNION_H
#define CEIGEN_QUATERNION_H

#include <eigen3/Eigen/Dense>

/**
 * @brief The  Eigen quaternion type used in the CEigen.
 */
typedef Eigen::Quaternion<float_t> ceigen_quaternion_impl;

/**
 * @brief The implementation detail of the ceigen_quaternion struct.
 */
struct ceigen_quaternion {
	/**
	 * @brief Eigen quaternion inside the struct, not visible to C.
	 */
	ceigen_quaternion_impl impl;

	/**
	 * @brief Constructor of the struct for initializing the impl.
	 */
	ceigen_quaternion();

	/**
	 * @brief Default deconstructor of the struct for freeing the eigen quaternion inside the struct.
	 */
	~ceigen_quaternion() = default;
};

#endif // CEIGEN_QUATERNION_H
