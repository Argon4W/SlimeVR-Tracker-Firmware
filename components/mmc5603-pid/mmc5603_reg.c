#include "mmc5603_reg.h"

int32_t WEAK mmc5603_raw_read_register(
	const	mmc_context_t*	context,
	const	uint8_t			address,
			uint8_t*		buffer,
	const	uint16_t		length
) {
	// We cannot proceed without the context.
	if (context == NULL) {
		return -1;
	}

	// Read from the registers.
	return context->read_register(
		/* user_handle	= */ context->user_handle,
		/* address		= */ address,
		/* buffer		= */ buffer,
		/* length		= */ length
	);
}

int32_t WEAK mmc5603_raw_write_register(
			mmc_context_t*	context,
	const	uint8_t			address,
	const	uint8_t*		buffer,
	const	uint16_t		length
) {
	// We cannot proceed without the context.
	if (context == NULL) {
		return -1;
	}

	// Write to the registers first.
	FAIL_FAST(context->write_register(
		/* user_handle	= */ context->user_handle,
		/* address		= */ address,
		/* buffer		= */ buffer,
		/* length		= */ length
	));

	// Calculate the overlapping range of write-only registers for updating shadow values.

	// Calculate the bounds of the register write.
	const uint16_t lower_bound_write = (uint16_t) address;
	const uint16_t upper_bound_write = (uint16_t) address + length;

	// Calculate the bounds of the write-only registers (shadow).
	const uint16_t lower_bound_shadow = MMC5603_CONTROL_0;
	const uint16_t upper_bound_shadow = MMC5603_CONTROL_2 + 1U;

	// Calculate the overlapping
	const uint16_t lower_bound = (lower_bound_write > lower_bound_shadow) ? lower_bound_write : lower_bound_shadow;
	const uint16_t upper_bound = (upper_bound_write < upper_bound_shadow) ? upper_bound_write : upper_bound_shadow;

	// Update shadow values in overlapped range.
	for (uint16_t i = lower_bound; i < upper_bound; i ++) {
		switch (i) {
			// Set shadow values of control registers.
			case MMC5603_CONTROL_0: context->shadow_values[0U] = buffer[i - address] & MMC5603_CONTROL_0_STICKY_MASK; break;
			case MMC5603_CONTROL_1:	context->shadow_values[1U] = buffer[i - address] & MMC5603_CONTROL_1_STICKY_MASK; break;
			case MMC5603_CONTROL_2:	context->shadow_values[2U] = buffer[i - address] & MMC5603_CONTROL_2_STICKY_MASK; break;
			default:
		}
	}

	return 0;
}

int32_t WEAK mmc5603_raw_product_id_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the PRODUCT_ID register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_PRODUCT_ID,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_x_0_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the X_OUT_0 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_0,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_x_1_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the X_OUT_1 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_1,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_x_2_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the X_OUT_2 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_2,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_y_0_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Y_OUT_0 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Y_OUT_0,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_y_1_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Y_OUT_1 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Y_OUT_1,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_y_2_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Y_OUT_2 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Y_OUT_2,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_z_0_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Z_OUT_0 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Z_OUT_0,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_z_1_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Z_OUT_1 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Z_OUT_1,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_mag_z_2_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the Z_OUT_2 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Z_OUT_2,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_temp_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the T_OUT register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_T_OUT,
		/* buffer	= */ val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_status_1_get(const mmc_context_t* context, mmc5603_raw_status_1_t* val) {
	// Read from the STATUS_1 register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_STATUS_1,
		/* buffer	= */ (uint8_t*) val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_odr_set(mmc_context_t* context, const uint8_t val) {
	// Write to the ODR register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ODR,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_control_0_set(mmc_context_t* context, const mmc5603_raw_control_0_t val) {
	// Write to the CONTROL_0 register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_CONTROL_0,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_control_1_set(mmc_context_t* context, const mmc5603_raw_control_1_t val) {
	// Write to the CONTROL_1 register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_CONTROL_1,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_control_2_set(mmc_context_t* context, const mmc5603_raw_control_2_t val) {
	// Write to the CONTROL_2 register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_CONTROL_2,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_x_th_set(mmc_context_t* context, const uint8_t val) {
	// Write to the ST_X_TH register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_X_TH,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_y_th_set(mmc_context_t* context, const uint8_t val) {
	// Write to the ST_Y_TH register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_Y_TH,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_z_th_set(mmc_context_t* context, const uint8_t val) {
	// Write to the ST_Z_TH register.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_Z_TH,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_th_set(mmc_context_t* context, const mmc5603_raw_st_th_t val) {
	// Write to the ST_X_TH, ST_Y_TH, and ST_Z_TH registers.
	return mmc5603_raw_write_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_X_TH,
		/* buffer	= */ (const uint8_t*) &val,
		/* length	= */ 3U
	);
}

int32_t WEAK mmc5603_raw_st_x_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the ST_X register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_X,
		/* buffer	= */ (uint8_t*) val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_y_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the ST_Y register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_Y,
		/* buffer	= */ (uint8_t*) val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_z_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the ST_Z register.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_Z,
		/* buffer	= */ (uint8_t*) val,
		/* length	= */ 1U
	);
}

int32_t WEAK mmc5603_raw_st_get(const mmc_context_t* context, mmc5603_raw_st_t* val) {
	// Read from the ST_X, ST_Y, and ST_Z registers.
	return mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_ST_X,
		/* buffer	= */ (uint8_t*) val,
		/* length	= */ 3U
	);
}

float_t WEAK mmc5603_ll_mag_16_from_lsb_to_gauss(const uint16_t lsb) {
	// Null field output in 16-bit is 32768, sensitivity in 16-bit is 1024 LSBs per Gauss.
	return 1.0f / 1024.0f * ((int32_t) lsb - 32768);
}

float_t WEAK mmc5603_ll_mag_20_from_lsb_to_gauss(const uint32_t lsb) {
	// Null field output in 20-bit is 524288, sensitivity in 20-bit is 16384 LSBs per Gauss.
	return 1.0f / 16384.0f * ((int32_t) lsb - 524288);
}

float_t WEAK mmc5603_ll_temp_from_lsb_to_celsius(const uint8_t lsb) {
	// The range is -75-125 degC, about 0.8 degC/LSB, 00000000 stands for -75 degC.
	return -75.0f + 0.8f * (int32_t) lsb;
}

int32_t WEAK mmc5603_ll_mag_16_x_get(const mmc_context_t* context, uint16_t* val) {
	// Reserve 2 bytes for the output.
	uint8_t buffer[2U];

	// Read from the X_OUT_0 and X_OUT_1 output registers.
	FAIL_FAST(mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_0,
		/* buffer	= */ buffer,
		/* length	= */ 2U
	));

	// Pack 2 bytes into an unsigned short.
	*val = ((uint16_t) buffer[0U] << 8U) | buffer[1U];

	return 0;
}

int32_t WEAK mmc5603_ll_mag_16_y_get(const mmc_context_t* context, uint16_t* val) {
	// Reserve 2 bytes for the output.
	uint8_t buffer[2U];

	// Read from the Y_OUT_0 and Y_OUT_1 output registers.
	FAIL_FAST(mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Y_OUT_0,
		/* buffer	= */ buffer,
		/* length	= */ 2U
	));

	// Pack 2 bytes into an unsigned short.
	*val = ((uint16_t) buffer[0U] << 8U) | buffer[1U];

	return 0;
}

int32_t WEAK mmc5603_ll_mag_16_z_get(const mmc_context_t* context, uint16_t* val) {
	// Reserve 2 bytes for the output.
	uint8_t buffer[2U];

	// Read from the Z_OUT_0 and Z_OUT_1 output registers.
	FAIL_FAST(mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_Z_OUT_0,
		/* buffer	= */ buffer,
		/* length	= */ 2U
	));

	// Pack 2 bytes into an unsigned short.
	*val = ((uint16_t) buffer[0U] << 8U) | buffer[1U];

	return 0;
}

int32_t WEAK mmc5603_ll_mag_16_get(const mmc_context_t* context, mmc5603_ll_mag_16_out_t* val) {
	// Reserve 6 bytes for the output.
	uint8_t buffer[6U];

	// Read from all 16-bit output registers.
	FAIL_FAST(mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_0,
		/* buffer	= */ buffer,
		/* length	= */ 6U
	));

	// Pack every 2 bytes into unsigned shorts.
	val->x_out = ((uint16_t) buffer[0U] << 8U) | buffer[1U]; // Pack X-axis output data.
	val->y_out = ((uint16_t) buffer[2U] << 8U) | buffer[3U]; // Pack Y-axis output data.
	val->z_out = ((uint16_t) buffer[4U] << 8U) | buffer[5U]; // Pack Z-axis output data.

	return 0;
}

int32_t WEAK mmc5603_ll_mag_20_x_get(const mmc_context_t* context, uint32_t* val) {
	// Reserve 3 bytes for the output.
	uint8_t buffer[3U];

	// Read from the X_OUT_0, X_OUT_1 and X_OUT_2 output registers.
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_X_OUT_0, &buffer[0U], 2U));
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_X_OUT_2, &buffer[2U], 1U));

	// Pack 3 bytes into an unsigned integer.
	*val = ((uint32_t) buffer[0U] << 12U) | ((uint32_t) buffer[1U] << 4U) | (buffer[2U] & 0b1111U);

	return 0;
}

int32_t WEAK mmc5603_ll_mag_20_y_get(const mmc_context_t* context, uint32_t* val) {
	// Reserve 3 bytes for the output.
	uint8_t buffer[3U];

	// Read from the Y_OUT_0, Y_OUT_1 and Y_OUT_2 output registers.
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_Y_OUT_0, &buffer[0U], 2U));
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_Y_OUT_2, &buffer[2U], 1U));

	// Pack 3 bytes into an unsigned integer.
	*val = ((uint32_t) buffer[0U] << 12U) | ((uint32_t) buffer[1U] << 4U) | (buffer[2U] & 0b1111U);

	return 0;
}

int32_t WEAK mmc5603_ll_mag_20_z_get(const mmc_context_t* context, uint32_t* val) {
	// Reserve 3 bytes for the output.
	uint8_t buffer[3U];

	// Read from the Z_OUT_0, Z_OUT_1 and Z_OUT_2 output registers.
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_Z_OUT_0, &buffer[0U], 2U));
	FAIL_FAST(mmc5603_raw_read_register(context, MMC5603_Z_OUT_2, &buffer[2U], 1U));

	// Pack 3 bytes into an unsigned integer.
	*val = ((uint32_t) buffer[0U] << 12U) | ((uint32_t) buffer[1U] << 4U) | (buffer[2U] & 0b1111U);

	return 0;
}

int32_t WEAK mmc5603_ll_mag_20_get(const mmc_context_t* context, mmc5603_ll_mag_20_out_t* val) {
	// Reserve 6 bytes for the output.
	uint8_t buffer[9U];

	// Read from all 20-bit output registers.
	FAIL_FAST(mmc5603_raw_read_register(
		/* context	= */ context,
		/* address	= */ MMC5603_X_OUT_0,
		/* buffer	= */ buffer,
		/* length	= */ 9U
	));

	// Pack every 3 bytes into unsigned integers.
	val->x_out = ((uint32_t) buffer[0U] << 12U) | ((uint32_t) buffer[1U] << 4U) | (buffer[6U] & 0b1111U);
	val->x_out = ((uint32_t) buffer[2U] << 12U) | ((uint32_t) buffer[3U] << 4U) | (buffer[7U] & 0b1111U);
	val->x_out = ((uint32_t) buffer[4U] << 12U) | ((uint32_t) buffer[5U] << 4U) | (buffer[8U] & 0b1111U);

	return 0;
}

int32_t WEAK mmc5603_ll_status_1_otp_read_done_get(const mmc_context_t* context, uint8_t* val) {
	// Reserve space for the read.
	mmc5603_raw_status_1_t status_1 = {0U};

	// Read from the STATUS_1 register.
	FAIL_FAST(mmc5603_raw_status_1_get(context, &status_1));

	// Return the value of OTP_READ_DONE bit of STATUS_1 register.
	*val = status_1.otp_read_done_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_status_1_sat_sensor_get(const mmc_context_t* context, uint8_t* val) {
	// Reserve space for the read.
	mmc5603_raw_status_1_t status_1 = {0U};

	// Read from the STATUS_1 register.
	FAIL_FAST(mmc5603_raw_status_1_get(context, &status_1));

	// Return the value of SAT_SENSOR bit of STATUS_1 register.
	*val = status_1.sat_sensor_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_status_1_meas_m_done_get(const mmc_context_t* context, uint8_t* val) {
	// Reserve space for the read.
	mmc5603_raw_status_1_t status_1 = {0U};

	// Read from the STATUS_1 register.
	FAIL_FAST(mmc5603_raw_status_1_get(context, &status_1));

	// Return the value of MEAS_M_DONE bit of STATUS_1 register.
	*val = status_1.meas_m_done_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_status_1_meas_t_done_get(const mmc_context_t* context, uint8_t* val) {
	// Reserve space for the read.
	mmc5603_raw_status_1_t status_1 = {0U};

	// Read from the STATUS_1 register.
	FAIL_FAST(mmc5603_raw_status_1_get(context, &status_1));

	// Return the value of MEAS_T_DONE bit of STATUS_1 register.
	*val = status_1.meas_t_done_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_0_take_meas_m(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set TAKE_MEAS_M bit on shadow value to prevent overwriting other bits.
	control_0.take_meas_m_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_take_meas_t(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set TAKE_MEAS_T bit on shadow value to prevent overwriting other bits.
	control_0.take_meas_t_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_do_set(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set DO_SET bit on shadow value to prevent overwriting other bits.
	control_0.do_set_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_do_reset(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set DO_RESET bit on shadow value to prevent overwriting other bits.
	control_0.do_reset_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_auto_sr_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set AUTO_SR_EN bit on shadow value to prevent overwriting other bits.
	control_0.auto_sr_en_bit = val;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_auto_sr_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_0 register.
	const mmc5603_raw_control_0_t control_0 = *(const mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Return the value of AUTO_SR_EN bit of CONTROL_0 register.
	*val = control_0.auto_sr_en_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_0_auto_st_en(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set AUTO_ST_EN bit on shadow value to prevent overwriting other bits.
	control_0.auto_st_en_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_0_cmm_freq_en(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_0 register.
	mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];

	// Set CMM_FREQ_EN bit on shadow value to prevent overwriting other bits.
	control_0.cmm_freq_en_bit = PROPERTY_ENABLE;

	// Write to CONTROL_0 register.
	return mmc5603_raw_control_0_set(context, control_0);
}

int32_t WEAK mmc5603_ll_control_1_bw_set(mmc_context_t* context, const mmc5603_bw_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set BW on shadow value to prevent overwriting other bits.
	control_1.bw = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_bw_get(const mmc_context_t* context, mmc5603_bw_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of BW of CONTROL_1 register.
	*val = control_1.bw;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_x_inhibit_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set X_INHIBIT bit on shadow value to prevent overwriting other bits.
	control_1.x_inhibit_bit = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_x_inhibit_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of X_INHIBIT of CONTROL_1 register.
	*val = control_1.x_inhibit_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_y_inhibit_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set Y_INHIBIT bit on shadow value to prevent overwriting other bits.
	control_1.y_inhibit_bit = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_y_inhibit_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of Y_INHIBIT of CONTROL_1 register.
	*val = control_1.y_inhibit_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_z_inhibit_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set Z_INHIBIT bit on shadow value to prevent overwriting other bits.
	control_1.z_inhibit_bit = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_z_inhibit_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of Z_INHIBIT of CONTROL_1 register.
	*val = control_1.z_inhibit_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_st_enp_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set ST_ENP bit on shadow value to prevent overwriting other bits.
	control_1.st_enp_bit = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_st_enp_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of ST_ENP bit of CONTROL_1 register.
	*val = control_1.st_enp_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_st_enm_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set ST_ENM bit on shadow value to prevent overwriting other bits.
	control_1.st_enm_bit = val;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_1_st_enm_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_1 register.
	const mmc5603_raw_control_1_t control_1 = *(const mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Return the value of ST_ENM bit of CONTROL_1 register.
	*val = control_1.st_enm_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_1_sw_reset(mmc_context_t* context) {
	// Read from the shadow value of the CONTROL_1 register.
	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];

	// Set SW_RESET bit on shadow value to prevent overwriting other bits.
	control_1.sw_reset_bit = PROPERTY_ENABLE;

	// Write to CONTROL_1 register.
	return mmc5603_raw_control_1_set(context, control_1);
}

int32_t WEAK mmc5603_ll_control_2_prd_set_set(mmc_context_t* context, const mmc5603_prd_st_t val) {
	// Read from the shadow value of the CONTROL_2 register.
	mmc5603_raw_control_2_t control_2 = *(mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Set PRD_SET on shadow value to prevent overwriting other bits.
	control_2.prd_set = val;

	// Write to CONTROL_2 register.
	return mmc5603_raw_control_2_set(context, control_2);
}

int32_t WEAK mmc5603_ll_control_2_prd_set_get(const mmc_context_t* context, mmc5603_prd_st_t* val) {
	// Read from the shadow value of the CONTROL_2 register.
	const mmc5603_raw_control_2_t control_2 = *(const mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Return the value of PRD_SET of CONTROL_2 register.
	*val = control_2.prd_set;

	return 0;
}

int32_t WEAK mmc5603_ll_control_2_en_prd_set_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_2 register.
	mmc5603_raw_control_2_t control_2 = *(mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Set EN_PRD_SET bit on shadow value to prevent overwriting other bits.
	control_2.en_prd_set_bit = val;

	// Write to CONTROL_2 register.
	return mmc5603_raw_control_2_set(context, control_2);
}

int32_t WEAK mmc5603_ll_control_2_en_prd_set_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_2 register.
	const mmc5603_raw_control_2_t control_2 = *(const mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Return the value of EN_PRD_SET bit of CONTROL_2 register.
	*val = control_2.en_prd_set_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_2_cmm_en_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_2 register.
	mmc5603_raw_control_2_t control_2 = *(mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Set CMM_EN bit on shadow value to prevent overwriting other bits.
	control_2.cmm_en_bit = val;

	// Write to CONTROL_2 register.
	return mmc5603_raw_control_2_set(context, control_2);
}

int32_t WEAK mmc5603_ll_control_2_cmm_en_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_2 register.
	const mmc5603_raw_control_2_t control_2 = *(const mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Return the value of CMM_EN bit of CONTROL_2 register.
	*val = control_2.cmm_en_bit;

	return 0;
}

int32_t WEAK mmc5603_ll_control_2_hpower_set(mmc_context_t* context, const uint8_t val) {
	// Read from the shadow value of the CONTROL_2 register.
	mmc5603_raw_control_2_t control_2 = *(mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Set HPOWER bit on shadow value to prevent overwriting other bits.
	control_2.hpower_bit = val;

	// Write to CONTROL_2 register.
	return mmc5603_raw_control_2_set(context, control_2);
}

int32_t WEAK mmc5603_ll_control_2_hpower_get(const mmc_context_t* context, uint8_t* val) {
	// Read from the shadow value of the CONTROL_2 register.
	const mmc5603_raw_control_2_t control_2 = *(const mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Return the value of HPOWER bit of CONTROL_2 register.
	*val = control_2.hpower_bit;

	return 0;
}

int32_t WEAK mmc5603_hl_setup(mmc_context_t* context, const mmc5603_hl_setup_t val) {
	// Set up the CONTROL_0 register value.
	const mmc5603_raw_control_0_t control_0 = {
		.auto_sr_en_bit		= val.auto_set_reset,
		.cmm_freq_en_bit	= val.odr > 0U,
		.take_meas_m_bit	= PROPERTY_DISABLE,
		.take_meas_t_bit	= PROPERTY_DISABLE,
		.start_mdt_bit		= PROPERTY_DISABLE,
		.do_set_bit			= PROPERTY_DISABLE,
		.do_reset_bit		= PROPERTY_DISABLE,
		.auto_st_en_bit		= PROPERTY_DISABLE
	};

	// Set up the CONTROL_1 register value.
	const mmc5603_raw_control_1_t control_1 = {
		.bw				= val.bandwidth,
		.x_inhibit_bit	= PROPERTY_DISABLE,
		.y_inhibit_bit	= PROPERTY_DISABLE,
		.z_inhibit_bit	= PROPERTY_DISABLE,
		.st_enp_bit		= PROPERTY_DISABLE,
		.st_enm_bit		= PROPERTY_DISABLE,
		.sw_reset_bit	= PROPERTY_DISABLE
	};

	// Set up the CONTROL_2 register value.
	const mmc5603_raw_control_2_t control_2 = {
		.prd_set			= val.periodic_set_period,
		.en_prd_set_bit		= val.periodic_set,
		.hpower_bit			= val.high_power,
		.cmm_en_bit			= val.odr > 0U,
		.int_mdt_en_bit		= PROPERTY_DISABLE,
		.int_meas_done_en	= PROPERTY_DISABLE,
	};

	// Set up output data rate of the continuous measurement mode.
	FAIL_FAST(mmc5603_raw_odr_set(context, val.odr));

	// Set up control registers then enable the continuous measurement mode.
	FAIL_FAST(mmc5603_raw_control_1_set(context, control_1));
	FAIL_FAST(mmc5603_raw_control_0_set(context, control_0));
	FAIL_FAST(mmc5603_raw_control_2_set(context, control_2));

	return 0;
}

int32_t WEAK mmc5603_hl_selftest(mmc_context_t* context, uint8_t* val) {
	// Cannot proceed without the delay function.
	if (context->delay_milliseconds == NULL) {
		return -1;
	}

	// Reserve space for the read.
	mmc5603_raw_st_t st = {0U};

	// Read from the self-test set values.
	FAIL_FAST(mmc5603_raw_st_get(context, &st));

	// Calculate the self-test thresholds.
	const mmc5603_raw_st_th_t st_th = {
		.st_x_th = (uint8_t) ((uint16_t) st.st_x * 80U) / 100U,
		.st_y_th = (uint8_t) ((uint16_t) st.st_y * 80U) / 100U,
		.st_z_th = (uint8_t) ((uint16_t) st.st_z * 80U) / 100U
	};

	// Write the self-test thresholds to registers.
	FAIL_FAST(mmc5603_raw_st_th_set(context, st_th));

	// Read and record from the shadow values of the control register.
			mmc5603_raw_control_0_t control_0 = *(mmc5603_raw_control_0_t*) &context->shadow_values[0U];
	const	mmc5603_raw_control_1_t control_1 = *(mmc5603_raw_control_1_t*) &context->shadow_values[1U];
	const	mmc5603_raw_control_2_t control_2 = *(mmc5603_raw_control_2_t*) &context->shadow_values[2U];

	// Set up the TAKE_MEAS_M and AUTO_ST_EN bit in CONTROL_0 register.
	const mmc5603_raw_control_0_t control_0_st = {
		.take_meas_m_bit	= PROPERTY_ENABLE,
		.auto_st_en_bit		= PROPERTY_ENABLE,
		.take_meas_t_bit	= PROPERTY_DISABLE,
		.start_mdt_bit		= PROPERTY_DISABLE,
		.do_set_bit			= PROPERTY_DISABLE,
		.do_reset_bit		= PROPERTY_DISABLE,
		.auto_sr_en_bit		= PROPERTY_DISABLE,
		.cmm_freq_en_bit	= PROPERTY_DISABLE
	};

	// Reset the control registers.
	FAIL_FAST(mmc5603_raw_control_2_set(context, (mmc5603_raw_control_2_t) {0U}));
	FAIL_FAST(mmc5603_raw_control_1_set(context, (mmc5603_raw_control_1_t) {0U}));

	// Delay 15 milliseconds to wait for the continuous measurement mode to disable.
	if (control_2.cmm_en_bit) {
		context->delay_milliseconds(15U);
	}

	// Take the measurement and enable automatic self-test.
	FAIL_FAST(mmc5603_raw_control_0_set(context, control_0_st));

	// Delay 15 milliseconds to wait for the self-test measurement to complete.
	context->delay_milliseconds(15U);

	// Read and return the result of the self-test from STATUS_1 register.
	FAIL_FAST(mmc5603_ll_status_1_sat_sensor_get(context, val));

	// Re-calculate frequency if needed.
	if (control_2.cmm_en_bit) {
		control_0.cmm_freq_en_bit = PROPERTY_ENABLE;
	}

	// Restore the values of the control registers.
	FAIL_FAST(mmc5603_raw_control_1_set(context, control_1));
	FAIL_FAST(mmc5603_raw_control_0_set(context, control_0));
	FAIL_FAST(mmc5603_raw_control_2_set(context, control_2));

	return 0;
}

int32_t WEAK mmc5603_hl_software_reset(mmc_context_t* context) {
	// Reset the device.
	FAIL_FAST(mmc5603_ll_control_1_sw_reset(context));

	// Reset the shadow values.
	context->shadow_values[0U] = 0U;
	context->shadow_values[1U] = 0U;
	context->shadow_values[2U] = 0U;

	return 0;
}

int32_t WEAK mmc5603_hl_mag_16_get(const mmc_context_t* context, mmc5603_hl_mag_out_t* val) {
	// Read the MEAS_M_DONE bit from STATUS_1 register.
	FAIL_FAST(mmc5603_ll_status_1_meas_m_done_get(context, &val->done));

	// We cannot proceed if the data is not ready.
	if (!val->done) {
		return 0;
	}

	// Reserve space for the read.
	mmc5603_ll_mag_16_out_t mag_16_out = {0U};

	// Read 16-bit magnetometer data output from registers.
	FAIL_FAST(mmc5603_ll_mag_16_get(context, &mag_16_out));

	// Convert the LSBs into Gauss and return.
	val->x_gauss = mmc5603_ll_mag_16_from_lsb_to_gauss(mag_16_out.x_out);
	val->y_gauss = mmc5603_ll_mag_16_from_lsb_to_gauss(mag_16_out.y_out);
	val->z_gauss = mmc5603_ll_mag_16_from_lsb_to_gauss(mag_16_out.z_out);

	return 0;
}

int32_t WEAK mmc5603_hl_mag_20_get(const mmc_context_t* context, mmc5603_hl_mag_out_t* val) {
	// Read the MEAS_M_DONE bit from STATUS_1 register.
	FAIL_FAST(mmc5603_ll_status_1_meas_m_done_get(context, &val->done));

	// We cannot proceed if the data is not ready.
	if (!val->done) {
		return 0;
	}

	// Reserve space for the read.
	mmc5603_ll_mag_20_out_t mag_20_out = {0U};

	// Read 20-bit magnetometer data output from registers.
	FAIL_FAST(mmc5603_ll_mag_20_get(context, &mag_20_out));

	// Convert the LSBs into Gauss and return.
	val->x_gauss = mmc5603_ll_mag_20_from_lsb_to_gauss(mag_20_out.x_out);
	val->y_gauss = mmc5603_ll_mag_20_from_lsb_to_gauss(mag_20_out.y_out);
	val->z_gauss = mmc5603_ll_mag_20_from_lsb_to_gauss(mag_20_out.z_out);

	return 0;
}

int32_t WEAK mmc5603_hl_temp_get(
	const	mmc_context_t*	context,
			uint8_t*		done,
			float_t*		val
) {
	// Read the MEAS_M_DONE bit from STATUS_1 register.
	FAIL_FAST(mmc5603_ll_status_1_meas_m_done_get(context, done));

	// We cannot proceed if the data is not ready.
	if (!*done) {
		return 0;
	}

	// Reserve space for the read.
	uint8_t temp;

	// Read from the T_OUT register.
	FAIL_FAST(mmc5603_raw_temp_get(context, &temp));

	// Convert the LSBs into degree(s) Celsius and return.
	*val = mmc5603_ll_temp_from_lsb_to_celsius(temp);

	return 0;
}