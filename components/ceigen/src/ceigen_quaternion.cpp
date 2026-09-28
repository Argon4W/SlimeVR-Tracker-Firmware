#include <eigen3/Eigen/Dense>
#include <ceigen.h>
#include <ceigen_matrix.h>
#include <ceigen_quaternion.h>

ceigen_quaternion::ceigen_quaternion(): impl() {
	// Initialize the quaternion to identity.
	impl.setIdentity();
}

#ifdef __cplusplus
extern "C" {
#endif

ceigen_quaternion_handle_t ceigen_new_quaternion() {
	// Create the handle.
	return new ceigen_quaternion();
}

void ceigen_delete_quaternion(
	const ceigen_quaternion_handle_t quaternion
) {
	// Free the handle.
	delete quaternion;
}

float_t ceigen_get_quaternion_w(
	const ceigen_quaternion_handle_t quaternion
) {
	// Get the value of the W.
	return quaternion->impl.w();
}

float_t ceigen_get_quaternion_x(
	const ceigen_quaternion_handle_t quaternion
) {
	// Get the value of the X.
	return quaternion->impl.x();
}

float_t ceigen_get_quaternion_y(
	const ceigen_quaternion_handle_t quaternion
) {
	// Get the value of the Y.
	return quaternion->impl.y();
}

float_t ceigen_get_quaternion_z(
	const ceigen_quaternion_handle_t quaternion
) {
	// Get the value of the Z.
	return quaternion->impl.z();
}

void ceigen_set_quaternion_w(
	const ceigen_quaternion_handle_t	quaternion,
	const float_t						value
) {
	// Set the value of the W.
	quaternion->impl.w() = value;
}

void ceigen_set_quaternion_x(
	const ceigen_quaternion_handle_t	quaternion,
	const float_t						value
) {
	// Set the value of the X.
	quaternion->impl.x() = value;
}

void ceigen_set_quaternion_y(
	const ceigen_quaternion_handle_t	quaternion,
	const float_t						value
) {
	// Set the value of the Y.
	quaternion->impl.y() = value;
}

void ceigen_set_quaternion_z(
	const ceigen_quaternion_handle_t	quaternion,
	const float_t						value
) {
	// Set the value of the Z.
	quaternion->impl.z() = value;
}

void ceigen_copy_quaternion(
	const ceigen_quaternion_handle_t source_quaternion,
	const ceigen_quaternion_handle_t destination_quaternion
) {
	// Copy the values in the source quaternion to the destination quaternion.
	destination_quaternion->impl = source_quaternion->impl;
}

void ceigen_slerp_quaternion(
	const ceigen_quaternion_handle_t	from_quaternion,
	const ceigen_quaternion_handle_t	to_quaternion,
	const float_t						factor,
	const ceigen_quaternion_handle_t	destination_quaternion
) {
	// Slerp the from_quaternion and to_quaternion, write the result to the destination quaternion.
	destination_quaternion->impl = from_quaternion->impl.slerp(factor, to_quaternion->impl);
}

float_t ceigen_dot_quaternion(
	const ceigen_quaternion_handle_t left_quaternion,
	const ceigen_quaternion_handle_t right_quaternion
) {
	// Calculate the dot product of the left_quaternion and right_quaternion.
	return left_quaternion->impl.dot(right_quaternion->impl);
}

void ceigen_multiply_quaternion(
	const ceigen_quaternion_handle_t left_quaternion,
	const ceigen_quaternion_handle_t right_quaternion,
	const ceigen_quaternion_handle_t destination_quaternion
) {
	// Multiply the left left_quaternion and the right right_quaternion, write the result to the destination destination_quaternion.
	destination_quaternion->impl = left_quaternion->impl * right_quaternion->impl;
}

void ceigen_rotate_quaternion_around_x(
	const float_t						rotation_x_radians,
	const ceigen_quaternion_handle_t	src_quaternion,
	const ceigen_quaternion_handle_t	destination_quaternion
) {
	// Rotate the src_quaternion around X axis in global space, write the result to the destination quaternion.
	destination_quaternion->impl = Eigen::AngleAxis<float_t>(rotation_x_radians, Eigen::Vector3<float_t>::UnitX()) * src_quaternion->impl;
}

void ceigen_rotate_quaternion_around_y(
	const float_t						rotation_y_radians,
	const ceigen_quaternion_handle_t	src_quaternion,
	const ceigen_quaternion_handle_t	destination_quaternion
) {
	// Rotate the src_quaternion around Y axis in global space, write the result to the destination quaternion.
	destination_quaternion->impl = Eigen::AngleAxis<float_t>(rotation_y_radians, Eigen::Vector3<float_t>::UnitY()) * src_quaternion->impl;
}

void ceigen_rotate_quaternion_around_z(
	const float_t						rotation_z_radians,
	const ceigen_quaternion_handle_t	src_quaternion,
	const ceigen_quaternion_handle_t	destination_quaternion
) {
	// Rotate the src_quaternion around Z axis in global space, write the result to the destination quaternion.
	destination_quaternion->impl = Eigen::AngleAxis<float_t>(rotation_z_radians, Eigen::Vector3<float_t>::UnitZ()) * src_quaternion->impl;
}

void ceigen_set_quaternion_rotation(
	const float_t						rotation_angle_radians,
	const ceigen_matrix_handle_t		rotation_axis,
	const ceigen_quaternion_handle_t	destination_quaternion
) {
	// Write the quaternion of the angle-axis rotation to the destination quaternion.
	destination_quaternion->impl = Eigen::AngleAxis<float_t>(rotation_angle_radians, rotation_axis->impl.col(0).head<3>());
}

void ceigen_quaternion_rotate_vector(
	const ceigen_matrix_handle_t		src_vector,
	const ceigen_quaternion_handle_t	quaternion,
	const ceigen_matrix_handle_t		destination_vector
) {
	// Rotate the src_vector with the quaternion, write the result to the destination vector.
	destination_vector->impl.col(0).head<3>() = (quaternion->impl * src_vector->impl.col(0).head<3>()).eval();
}

void ceigen_quaternion_to_rotation_matrix(
	const ceigen_quaternion_handle_t	src_quaternion,
	const ceigen_matrix_handle_t		destination_matrix
) {
	// Write the rotation matrix of the quaternion to the destination matrix.
	destination_matrix->impl = src_quaternion->impl.toRotationMatrix();
}

void ceigen_normalize_quaternion_in_place(
	const ceigen_quaternion_handle_t source_quaternion
) {
	// Normalize the quaternion in place.
	source_quaternion->impl.normalize();
}

void ceigen_conjugate_quaternion_in_place(
	const ceigen_quaternion_handle_t source_quaternion
) {
	// Conjugate the quaternion in place.
	source_quaternion->impl = source_quaternion->impl.conjugate();
}

void ceigen_inverse_quaternion_in_place(
	const ceigen_quaternion_handle_t source_quaternion
) {
	// Invert the quaternion in place.
	source_quaternion->impl = source_quaternion->impl.inverse();
}

void ceigen_set_quaternion_identity_in_place(
	const ceigen_quaternion_handle_t source_quaternion
) {
	// Set the quaternion to identity quaternion in place.
	source_quaternion->impl.setIdentity();
}

#ifdef __cplusplus
}
#endif