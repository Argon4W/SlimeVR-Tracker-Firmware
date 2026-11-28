#ifndef MMC5603_REG_H
#define MMC5603_REG_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <math.h>

/**
 * @brief Ensure register layout structs to be tightly-packed.
 */
#ifndef PACKED
#define PACKED __attribute__((__packed__))
#endif

/**
 * @brief Endianness definitions.
 */
#ifndef DEVICE_BYTE_ORDER
	#ifndef __BYTE_ORDER__
		#define DEVICE_LITTLE_ENDIAN	1234
		#define DEVICE_BIG_ENDIAN		4321

		// If the byte order is not defined, choose the endianness by uncommenting the following defines.
		#define DEVICE_BYTE_ORDER DEVICE_LITTLE_ENDIAN
		// #define DEVICE_BYTE_ORDER DEVICE_LITTLE_ENDIAN
	#else
		#define DEVICE_LITTLE_ENDIAN	__ORDER_LITTLE_ENDIAN__
		#define DEVICE_BIG_ENDIAN		__ORDER_BIG_ENDIAN__
		#define DEVICE_BYTE_ORDER		__BYTE_ORDER__
	#endif // __BYTE_ORDER__
#endif // DEVICE_BYTE_ORDER

#ifndef MMC_SHARED_TYPES
#define MMC_SHARED_TYPES

#define PROPERTY_DISABLE	(0U)
#define PROPERTY_ENABLE		(1U)


/**
 * @brief				Platform independent function type of reading from a generic register of the device. ("0" = no error)
 * @param user_handle	the user-defined custom handle needed by the implementation, as pointer.
 * @param address		the first register address to be read.
 * @param buffer		the buffer to hold the data read from the registers, as pointer.
 * @param length		the count of registers we want to read from.
 * @retval				status of implementation function interfaces. ("0" = no error)
 */
typedef int32_t (*mmc_read_register_function_ptr)(
	void*		user_handle,
	uint8_t		address,
	uint8_t*	buffer,
	uint16_t	length
);

/**
 * @brief				Platform independent function type of writing to a generic register of the device. ("0" = no error)
 * @param user_handle	the user-defined custom handle needed by the implementation, as pointer.
 * @param address		the first register address to be written.
 * @param buffer		the data to be written to the registers, as pointer.
 * @param length		the count of registers we want to write to.
 * @retval				status of implementation function interfaces. ("0" = no error)
 */
typedef int32_t (*mmc_write_register_function_ptr)(
			void*		user_handle,
			uint8_t		address,
	const	uint8_t*	buffer,
			uint16_t	length
);

/**
 * @brief Platform independent function type of delaying milliseconds.
 */
typedef void (*mmc_delay_milliseconds_function_ptr)(uint32_t milliseconds);

/**
 * @brief Platform independent driver function interface context.
 */
typedef struct {
	mmc_read_register_function_ptr		read_register;			/*!< Implementation function of reading from registers. */
	mmc_write_register_function_ptr		write_register;			/*!< Implementation function of writing to registers. */
	mmc_delay_milliseconds_function_ptr	delay_milliseconds;		/*!< Implementation function of delaying milliseconds. */
	uint8_t								shadow_values[3U];		/*!< Shadow values of 3 control registers. */
	void*								user_handle;			/*!< The user-defined custom handle needed by the implementation. */
} mmc_context_t;

#endif // MMC_SHARED_TYPES

/**
 * @brief Chip properties.
 */
/** I2C Device Address in 7-bit format. */
#define MMC5603_I2C_ADDRESS 0x30U

/**
 * @brief Chip identity register related.
 */
/** Product ID Register. */
#define MMC5603_PRODUCT_ID		0x39U
/** Product ID reference value. */
#define MMC5603_PRODUCT_ID_REF	0x10U

/**
 * @brief	16-bit Data output register parts.
 *			Registers 0x00U ~ 0x05U store the [19:4] part of the measurement data from each axis.
 */
/** Data Output X Register Part XOUT[19:12]. */
#define MMC5603_X_OUT_0 0x00U
/** Data Output X Register Part XOUT[11:4]. */
#define MMC5603_X_OUT_1 0x01U
/** Data Output Y Register Part YOUT[19:12]. */
#define MMC5603_Y_OUT_0 0x02U
/** Data Output Y Register Part YOUT[11:4]. */
#define MMC5603_Y_OUT_1 0x03U
/** Data Output Z Register Part ZOUT[19:12]. */
#define MMC5603_Z_OUT_0 0x04U
/** Data Output Z Register Part ZOUT[11:4]. */
#define MMC5603_Z_OUT_1 0x05U

/**
 * @brief	20-bit Data output register parts.
 *			Registers 0x06U ~ 0x08U store the [3:0] part of the measurement data from each axis.
 */
/** Data Output X Register Part XOUT[3:0]. */
#define MMC5603_X_OUT_2 0x06U
/** Data Output Y Register Part YOUT[3:0]. */
#define MMC5603_Y_OUT_2 0x07U
/** Data Output Z Register Part ZOUT[3:0]. */
#define MMC5603_Z_OUT_2 0x08U

/**
 * @brief Temperature Output Register TOUT[7:0].
 */
#define MMC5603_T_OUT 0x09U

/**
 * @brief Status Register 1.
 */
#define MMC5603_STATUS_1						0x18U
/** Factory use only, reset value is 0. */
#define MMC5603_STATUS_1_MEAS_M_DONE_INT_BIT	0U
/** Factory use only, reset value is 0. */
#define MMC5603_STATUS_1_MEAS_T_DONE_INT_BIT	1U
/** Factory use only, reset value is 0. */
#define MMC5603_STATUS_1_MDT_FLAG_INT_BIT		2U
/** Factory use only, reset value is 0. */
#define MMC5603_STATUS_1_ST_FAIL_BIT			3U
/** This bit is an indicator of successfully reading its OTP memory either by part of itss power up sequence, or after an I2C command that reloads the OTP memory, such as resetting the chip and refreshing the OTP registers. */
#define MMC5603_STATUS_1_OTP_READ_DONE_BIT		4U
/** This bit is an indicator of the selftest signal, it keeps low once the device PASS selftest. */
#define MMC5603_STATUS_1_SAT_SENSOR_BIT			5U
/** This bit indicates that a measurement of magnetic field is done and the data is ready to be read. This bit is reset only when any of the magnetic data registers is read. ("0" = magnetic data measurement not done; "1" = magnetic data measurement done.) */
#define MMC5603_STATUS_1_MEAS_M_DONE_BIT		6U
/** This bit indicates that a measurement of temperature is done and the data is ready to be read. This bit is reset only when the temperature register is read. ("0" = temperature data measurement not done; "1" = temperature data measurement done.) */
#define MMC5603_STATUS_1_MEAS_T_DONE_BIT		7U

/**
 * @brief Status register 1 data layout struct.
 */
typedef struct {
	#if DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
		uint8_t meas_m_done_int_bit	: 1U; /*!< MMC5603_STATUS_1_MEAS_M_DONE_INT_BIT. */
		uint8_t meas_t_done_int_bit	: 1U; /*!< MMC5603_STATUS_1_MEAS_T_DONE_INT_BIT. */
		uint8_t mdt_flag_int_bit	: 1U; /*!< MMC5603_STATUS_1_MDT_FLAG_INT_BIT. */
		uint8_t st_fail_bit			: 1U; /*!< MMC5603_STATUS_1_ST_FAIL_BIT. */
		uint8_t otp_read_done_bit	: 1U; /*!< MMC5603_STATUS_1_OTP_READ_DONE_BIT. */
		uint8_t sat_sensor_bit		: 1U; /*!< MMC5603_STATUS_1_SAT_SENSOR_BIT. */
		uint8_t meas_m_done_bit		: 1U; /*!< MMC5603_STATUS_1_MEAS_M_DONE_BIT. */
		uint8_t meas_t_done_bit		: 1U; /*!< MMC5603_STATUS_1_MEAS_T_DONE_BIT. */
	#else
		uint8_t meas_t_done_bit		: 1U; /*!< MMC5603_STATUS_1_MEAS_T_DONE_BIT. */
		uint8_t meas_m_done_bit		: 1U; /*!< MMC5603_STATUS_1_MEAS_M_DONE_BIT. */
		uint8_t sat_sensor_bit		: 1U; /*!< MMC5603_STATUS_1_SAT_SENSOR_BIT. */
		uint8_t otp_read_done_bit	: 1U; /*!< MMC5603_STATUS_1_OTP_READ_DONE_BIT. */
		uint8_t st_fail_bit			: 1U; /*!< MMC5603_STATUS_1_ST_FAIL_BIT. */
		uint8_t mdt_flag_int_bit	: 1U; /*!< MMC5603_STATUS_1_MDT_FLAG_INT_BIT. */
		uint8_t meas_t_done_int_bit	: 1U; /*!< MMC5603_STATUS_1_MEAS_T_DONE_INT_BIT. */
		uint8_t meas_m_done_int_bit	: 1U; /*!< MMC5603_STATUS_1_MEAS_M_DONE_INT_BIT. */
	#endif // DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
} PACKED mmc5603_raw_status_1_t;

/**
 * @brief Output Data Rate Register ODR[7:0].
 */
#define MMC5603_ODR 0x1AU

/**
 * @brief Control register 0.
 */
#define MMC5603_CONTROL_0					0x1BU
/** Take Measure of Magnetic field. Writing a 1 into this location causes the chip to perform a magnetic measurement. This bit is self-clearing at the end of each measurement.  */
#define MMC5603_CONTROL_0_TAKE_MEAS_M_BIT	0U
/** Take Measure of Temperature. Writing a 1 into this location causes the chip to perform a temperature measurement. This bit is self-clearing at the end of each measurement.  */
#define MMC5603_CONTROL_0_TAKE_MEAS_T_BIT	1U
/** Factory use only, reset value is 0. */
#define MMC5603_CONTROL_0_START_MDT_BIT		2U
/** Writing a 1 into this location will cause the chip to do the Set operation, which will allow large set current to flow through the sensor coils for 375ns. This bit is self-cleared at the end of Set operation. */
#define MMC5603_CONTROL_0_DO_SET_BIT		3U
/** Writing a 1 into this location will cause the chip to do the Reset operation, which will allow large reset current to flow through the sensor coils for 375ns. This bit is self-cleared at the end of Reset operation. */
#define MMC5603_CONTROL_0_DO_RESET_BIT		4U
/** Writing a 1 into this location will enable the function of automatic set/reset. This function applies to both on-demand and continuous-time measurements. This bit must be set to 1 in order to activate the feature of periodic set. This bit is recommended to set to “1” in the application. */
#define MMC5603_CONTROL_0_AUTO_SR_EN_BIT	5U
/** Writing a 1 into this location will enable the function of automatic self-test. The threshold in register 1EH, 1FH, 20H should be set before this bit is set to 1. This bit clears itself after the operation is completed. */
#define MMC5603_CONTROL_0_AUTO_ST_EN_BIT	6U
/** Writing a 1 into this location will start the calculation of the measurement period according to the ODR. This bit should be set before continuous-mode measurements are started. This bit is self-cleared after the measurement period is calculated by internal circuits. */
#define MMC5603_CONTROL_0_CMM_FREQ_EN_BIT	7U

/**
 * @brief Mask of sticky bits in Control Register 0.
 */
#define MMC5603_CONTROL_0_STICKY_MASK 0b00100000U

/**
 * @brief Control register 0 data layout struct.
 */
typedef struct {
	#if DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
		uint8_t take_meas_m_bit	: 1U; /*!< MMC5603_CONTROL_0_TAKE_MEAS_M_BIT */
		uint8_t take_meas_t_bit	: 1U; /*!< MMC5603_CONTROL_0_TAKE_MEAS_T_BIT */
		uint8_t start_mdt_bit	: 1U; /*!< MMC5603_CONTROL_0_START_MDT_BIT */
		uint8_t do_set_bit		: 1U; /*!< MMC5603_CONTROL_0_DO_SET_BIT. */
		uint8_t do_reset_bit	: 1U; /*!< MMC5603_CONTROL_0_DO_RESET_BIT. */
		uint8_t auto_sr_en_bit	: 1U; /*!< MMC5603_CONTROL_0_AUTO_SR_EN_BIT. */
		uint8_t auto_st_en_bit	: 1U; /*!< MMC5603_CONTROL_0_AUTO_ST_EN_BIT. */
		uint8_t cmm_freq_en_bit	: 1U; /*!< MMC5603_CONTROL_0_CMM_FREQ_EN_BIT. */
	#else
		uint8_t cmm_freq_en_bit	: 1U; /*!< MMC5603_CONTROL_0_CMM_FREQ_EN_BIT. */
		uint8_t auto_st_en_bit	: 1U; /*!< MMC5603_CONTROL_0_AUTO_ST_EN_BIT. */
		uint8_t auto_sr_en_bit	: 1U; /*!< MMC5603_CONTROL_0_AUTO_SR_EN_BIT. */
		uint8_t do_reset_bit	: 1U; /*!< MMC5603_CONTROL_0_DO_RESET_BIT. */
		uint8_t do_set_bit		: 1U; /*!< MMC5603_CONTROL_0_DO_SET_BIT. */
		uint8_t start_mdt_bit	: 1U; /*!< MMC5603_CONTROL_0_START_MDT_BIT */
		uint8_t take_meas_t_bit	: 1U; /*!< MMC5603_CONTROL_0_TAKE_MEAS_T_BIT */
		uint8_t take_meas_m_bit	: 1U; /*!< MMC5603_CONTROL_0_TAKE_MEAS_M_BIT */
	#endif // DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
} PACKED mmc5603_raw_control_0_t;

/**
 * @brief Control Register 1.
 */
#define MMC5603_CONTROL_1						0x1CU
/** These bandwidth selection bits[1:0] adjust the length of the decimation filter. They control the duration of each measurement. There are 4 modes for selection: 6.6ms (0b00U), 3.5 (0b01U), 2.0ms (0b10U), 1.2ms (0b11U). */
#define MMC5603_CONTROL_1_BANDWIDTH_START_BIT	0U
/** Writing "1" will disable channel X, and reduce Measurement Time and total charge per measurement. When channel X is disabled it is simply skipped during Take Measure routine. Its output register is not reset and will maintain the last value written to it when this channel was active. */
#define MMC5603_CONTROL_1_X_INHIBIT_BIT			2U
/** Writing "1" will disable channel Y, and reduce Measurement Time and total charge per measurement. When channel Y is disabled it is simply skipped during Take Measure routine. Its output register is not reset and will maintain the last value written to it when this channel was active. (Y/Z needs to be inhibited the same time in case needed.) */
#define MMC5603_CONTROL_1_Y_INHIBIT_BIT			3U
/** Writing "1" will disable channel Z, and reduce Measurement Time and total charge per measurement. When channel Z is disabled it is simply skipped during Take Measure routine. Its output register is not reset and will maintain the last value written to it when this channel was active. (Y/Z needs to be inhibited the same time in case needed.) */
#define MMC5603_CONTROL_1_Z_INHIBIT_BIT			4U
/** Writing 1 into this location will bring a DC current through the self-test coil of the sensor. This current will cause an offset of the magnetic field. This function is used to check whether the sensor has been saturated. */
#define MMC5603_CONTROL_1_ST_ENP_BIT			5U
/** The function of this bit is similar to ST_ENP, but the offset of the magnetic field is of opposite polarity. */
#define MMC5603_CONTROL_1_ST_ENM_BIT			6U
/** Software Reset. Writing "1" will cause the part to reset, similar to power-up. It will clear all registers and also re-read OTP as part of its startup routine. The power on time is 20mS. */
#define MMC5603_CONTROL_1_SW_RESET_BIT			7U

/**
 * @brief Mask of sticky bits in Control Register 1.
 */
#define MMC5603_CONTROL_1_STICKY_MASK 0b01111111U

/**
 * @brief Bandwidth selections[1:0] in control register 1.
 */
typedef enum {
	MEAS_TIME_6_6_MS = 0b00U,	/*!< 6.6 milliseconds measurement time. */
	MEAS_TIME_3_5_MS = 0b01U,	/*!< 3.5 milliseconds measurement time. */
	MEAS_TIME_2_0_MS = 0b10U,	/*!< 2.0 milliseconds measurement time. */
	MEAS_TIME_1_2_MS = 0b10U	/*!< 1.2 milliseconds measurement time. */
} mmc5603_bw_t;

/**
 * @brief Control register 1 data layout struct.
 */
typedef struct {
	#if DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
		uint8_t bw				: 2U; /*!< MMC5603_CONTROL_1_BANDWIDTH_START_BIT, mmc5603_bw_t. */
		uint8_t x_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_X_INHIBIT_BIT. */
		uint8_t y_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_Y_INHIBIT_BIT. */
		uint8_t z_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_Z_INHIBIT_BIT. */
		uint8_t st_enp_bit		: 1U; /*!< MMC5603_CONTROL_1_ST_ENP_BIT. */
		uint8_t st_enm_bit		: 1U; /*!< MMC5603_CONTROL_1_ST_ENM_BIT. */
		uint8_t sw_reset_bit	: 1U; /*!< MMC5603_CONTROL_1_SW_RESET_BIT. */
	#else
		uint8_t sw_reset_bit	: 1U; /*!< MMC5603_CONTROL_1_SW_RESET_BIT. */
		uint8_t st_enm_bit		: 1U; /*!< MMC5603_CONTROL_1_ST_ENM_BIT. */
		uint8_t st_enp_bit		: 1U; /*!< MMC5603_CONTROL_1_ST_ENP_BIT. */
		uint8_t z_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_Z_INHIBIT_BIT. */
		uint8_t y_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_Y_INHIBIT_BIT. */
		uint8_t x_inhibit_bit	: 1U; /*!< MMC5603_CONTROL_1_X_INHIBIT_BIT. */
		uint8_t bw				: 2U; /*!< MMC5603_CONTROL_1_BANDWIDTH_START_BIT, mmc5603_bw_t. */
	#endif // DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
} PACKED mmc5603_raw_control_1_t;

/**
 * @brief Control Register 2.
 */
#define MMC5603_CONTROL_2						0x1DU
/** These bits determine how many measurements are done before a set is executed, when the part is in continuous mode and the automatic set/reset is enabled.  From 000 to 111, the sensor will do one set for every 1, 25, 75, 100, 250, 500, 1000, and 2000 samples. In order to enable this feature, both En_prd_set and Auto_SR must be set to 1, and the part should work in continuous mode. Please note that during this operation, the sensor will not be reset. */
#define MMC5603_CONTROL_2_PRD_SET_START_BIT		0U
/** Writing 1 into this location will enable the function of periodical set. */
#define MMC5603_CONTROL_2_EN_PRD_SET_BIT		3U
/** The device will enter continuous mode, if ODR has been set to a non-zero value and a 1 has been written into Cmm_freq_en. The internal counter will start counting as well since this bit is set. */
#define MMC5603_CONTROL_2_CMM_EN_BIT			4U
/** Factory use only, reset value is 0. */
#define MMC5603_CONTROL_2_INT_MDT_EN_BIT		5U
/** Factory use only, reset value is 0. */
#define MMC5603_CONTROL_2_INT_MEAS_DONE_EN_BIT	6U
/** If this bit is set to 1 to achieve 1000Hz ODR. */
#define MMC5603_CONTROL_2_HPOWER_BIT			7U

/**
 * @brief Mask of sticky bits in Control Register 2.
 */
#define MMC5603_CONTROL_2_STICKY_MASK 0b10011111U

/**
 * @brief Periodic set period selections[1:0] in control register 2.
 */
typedef enum {
	EVERY_1_SAMPLE		= 0b000U,	/*!< The sensor will do one set for every sample. */
	EVERY_25_SAMPLES	= 0b001U,	/*!< The sensor will do one set for every 25 samples. */
	EVERY_75_SAMPLES	= 0b010U,	/*!< The sensor will do one set for every 75 samples. */
	EVERY_100_SAMPLES	= 0b011U,	/*!< The sensor will do one set for every 100 samples. */
	EVERY_250_SAMPLES	= 0b100U,	/*!< The sensor will do one set for every 250 samples. */
	EVERY_500_SAMPLES	= 0b101U,	/*!< The sensor will do one set for every 500 samples. */
	EVERY_1000_SAMPLES	= 0b110U,	/*!< The sensor will do one set for every 1000 samples. */
	EVERY_2000_SAMPLES	= 0b111U	/*!< The sensor will do one set for every 2000 samples. */
} mmc5603_prd_st_t;

/**
 * @brief Control register 2 data layout struct.
 */
typedef struct {
	#if DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
		uint8_t prd_set				: 3U; /*!< MMC5603_CONTROL_2_PRD_SET_START_BIT, mmc5603_prd_st_t. */
		uint8_t en_prd_set_bit		: 1U; /*!< MMC5603_CONTROL_2_EN_PRD_SET_BIT. */
		uint8_t cmm_en_bit			: 1U; /*!< MMC5603_CONTROL_2_CMM_EN_BIT. */
		uint8_t int_mdt_en_bit		: 1U; /*!< MMC5603_CONTROL_2_INT_MDT_EN_BIT. */
		uint8_t int_meas_done_en	: 1U; /*!< MMC5603_CONTROL_2_INT_MEAS_DONE_EN_BIT. */
		uint8_t hpower_bit			: 1U; /*!< MMC5603_CONTROL_2_HPOWER_BIT. */
	#else
		uint8_t hpower_bit			: 1U; /*!< MMC5603_CONTROL_2_HPOWER_BIT. */
		uint8_t int_meas_done_en	: 1U; /*!< MMC5603_CONTROL_2_INT_MEAS_DONE_EN_BIT. */
		uint8_t int_mdt_en_bit		: 1U; /*!< MMC5603_CONTROL_2_INT_MDT_EN_BIT. */
		uint8_t cmm_en_bit			: 1U; /*!< MMC5603_CONTROL_2_CMM_EN_BIT. */
		uint8_t en_prd_set_bit		: 1U; /*!< MMC5603_CONTROL_2_EN_PRD_SET_BIT. */
		uint8_t prd_set				: 3U; /*!< MMC5603_CONTROL_2_PRD_SET_START_BIT, mmc5603_prd_st_t. */
	#endif // DEVICE_BYTE_ORDER == DEVICE_LITTLE_ENDIAN
} PACKED mmc5603_raw_control_2_t;

/**
 * @brief Self-test threshold registers.
 */
/** X-axis Self-test Threshold Register ST_X_TH[7:0]. */
#define MMC5603_ST_X_TH 0x1EU
/** Y-axis Self-test Threshold Register ST_Y_TH[7:0]. */
#define MMC5603_ST_Y_TH 0x1FU
/** Z-axis Self-test Threshold Register ST_Z_TH[7:0]. */
#define MMC5603_ST_Z_TH 0x20U

/**
 * @brief Full self-test threshold (3-axis) raw data struct.
 */
typedef struct {
	uint8_t st_x_th; /*!< X-axis self-test threshold register ST_X_TH[7:0]. */
	uint8_t st_y_th; /*!< Y-axis self-test threshold register ST_Y_TH[7:0]. */
	uint8_t st_z_th; /*!< Z-axis self-test threshold register ST_Z_TH[7:0]. */
} mmc5603_raw_st_th_t;

/**
 * @brief Self-test set value registers.
 */
/** X-axis Self-test Set Value Register ST_X[7:0]. */
#define MMC5603_ST_X 0x27U
/** Y-axis Self-test Set Value Register ST_Y[7:0]. */
#define MMC5603_ST_Y 0x28U
/** Z-axis Self-test Set Value Register ST_Z[7:0]. */
#define MMC5603_ST_Z 0x29U

/**
 * @brief Full self-test set value (3-axis) raw data struct.
 */
typedef struct {
	uint8_t st_x; /*!< X-axis self-test set value register ST_X_TH[7:0]. */
	uint8_t st_y; /*!< Y-axis self-test set value register ST_Y_TH[7:0]. */
	uint8_t st_z; /*!< Z-axis self-test set value register ST_Z_TH[7:0]. */
} mmc5603_raw_st_t;

/**
 * @brief Make sure driver functions can be overridden by user-defined custom implementations.
 */
#ifndef WEAK
#define WEAK __attribute__((weak))
#endif // WEAK

/**
 * @brief		Fail fast when the return value is indicating an error. ("0" = no error; other value = error)
 * @param val	The expression that needs to be checked.
 */
#ifndef FAIL_FAST
#define FAIL_FAST(val) do {	\
	int32_t result = (val);	\
	if (result != 0) {		\
		return result;		\
	}						\
} while (0);
#endif

/**
 * @brief			Read from a generic device register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param address	first register address to read.
 * @param buffer	buffer for the data read, as pointer.
 * @param length	number of registers to read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_read_register(
	const	mmc_context_t*	context,
			uint8_t			address,
			uint8_t*		buffer,
			uint16_t		length
);

/**
 * @brief			Write to a generic device register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param address	first register address to write.
 * @param buffer	buffer for the data write, as pointer.
 * @param length	number of registers to write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_write_register(
			mmc_context_t*	context,
			uint8_t			address,
	const	uint8_t*		buffer,
			uint16_t		length
);

/**
 * @brief			Get raw product ID.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_product_id_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [19:12] on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_x_0_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [11:4] on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_x_1_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [3:0] on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_x_2_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [19:12] on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_y_0_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [11:4] on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_y_1_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [3:0] on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_y_2_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [19:12] on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_z_0_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [11:4] on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_z_1_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw magnetometer data part [3:0] on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_mag_z_2_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw temperature data
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_temp_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw status from status 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_status_1_get(const mmc_context_t* context, mmc5603_raw_status_1_t* val);

/**
 * @brief			Set raw output data rate.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_odr_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Overwrite raw configurations into control 0 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_control_0_set(mmc_context_t* context, mmc5603_raw_control_0_t val);

/**
 * @brief			Overwrite raw configurations into control 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_control_1_set(mmc_context_t* context, mmc5603_raw_control_1_t val);

/**
 * @brief			Overwrite raw configurations into control 2 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_control_2_set(mmc_context_t* context, mmc5603_raw_control_2_t val);

/**
 * @brief			Set the raw self-test threshold on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_x_th_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Set the raw self-test threshold on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_y_th_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Set the raw self-test threshold on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_z_th_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Set the raw self-test thresholds.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		value for the register write.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_th_set(mmc_context_t* context, mmc5603_raw_st_th_t val);

/**
 * @brief			Get raw self-test set value on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_x_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw self-test set value on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_y_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw self-test set value on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the register read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_z_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get raw self-test set values.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the registers read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_raw_st_get(const mmc_context_t* context, mmc5603_raw_st_t* val);

/**
 * @brief		Convert 16-bit magnetometer lsb into gauss.
 * @param lsb	magnetometer raw data in 16-bit.
 * @retval		magnetometer value in gauss.
 */
float_t mmc5603_ll_mag_16_from_lsb_to_gauss(uint16_t lsb);

/**
 * @brief		Convert 20-bit magnetometer lsb into gauss.
 * @param lsb	magnetometer raw data in 20-bit.
 * @retval		magnetometer value in gauss.
 */
float_t mmc5603_ll_mag_20_from_lsb_to_gauss(uint32_t lsb);

/**
 * @brief		Convert temperature lsb into degree(s) Celsius.
 * @param lsb	temperature raw data in LSB.
 * @retval		temperature value in degree(s) Celsius.
 */
float_t mmc5603_ll_temp_from_lsb_to_celsius(uint8_t lsb);

/**
 * @brief			Get 16-bit raw magnetometer data on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_16_x_get(const mmc_context_t* context, uint16_t* val);

/**
 * @brief			Get 16-bit raw magnetometer data on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_16_y_get(const mmc_context_t* context, uint16_t* val);

/**
 * @brief			Get 16-bit raw magnetometer data on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_16_z_get(const mmc_context_t* context, uint16_t* val);

/**
 * @brief MMC5603 16-bit 3-axis raw magnetometer data struct.
 */
typedef struct {
	uint16_t x_out; /*!< Magnetometer data output X registers part XOUT[19:4]. */
	uint16_t y_out; /*!< Magnetometer data output Y registers part YOUT[19:4]. */
	uint16_t z_out; /*!< Magnetometer data output Z registers part ZOUT[19:4]. */
} mmc5603_ll_mag_16_out_t;

/**
 * @brief			Get 16-bit raw magnetometer data.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_16_get(const mmc_context_t* context, mmc5603_ll_mag_16_out_t* val);

/**
 * @brief			Get 20-bit raw magnetometer data on X-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_20_x_get(const mmc_context_t* context, uint32_t* val);

/**
 * @brief			Get 20-bit raw magnetometer data on Y-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_20_y_get(const mmc_context_t* context, uint32_t* val);

/**
 * @brief			Get 20-bit raw magnetometer data on Z-axis.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_20_z_get(const mmc_context_t* context, uint32_t* val);

/**
 * @brief MMC5603 20-bit 3-axis raw data struct.
 */
typedef struct {
	int32_t x_out; /*!< Magnetometer data output X registers XOUT[19:0]. */
	int32_t y_out; /*!< Magnetometer data output Y registers YOUT[19:0]. */
	int32_t z_out; /*!< Magnetometer data output Z registers ZOUT[19:0]. */
} mmc5603_ll_mag_20_out_t;

/**
 * @brief			Get 20-bit raw magnetometer data.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetometer data read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_mag_20_get(const mmc_context_t* context, mmc5603_ll_mag_20_out_t* val);

/**
 * @brief			Get the OTP memory reading status from the status 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the OTP memory reading status read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_status_1_otp_read_done_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get the self-test signal status from the status 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the self-test signal status read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_status_1_sat_sensor_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get the magnetic field measurement done status from the status 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the magnetic field measurement done status read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_status_1_meas_m_done_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Get temperature measurement done status from the status 1 register.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the temperature measurement done status read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_status_1_meas_t_done_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Take measure of magnetic field through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_take_meas_m(mmc_context_t* context);

/**
 * @brief			Take measure of temperature through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_take_meas_t(mmc_context_t* context);

/**
 * @brief			Do the SET operation through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_do_set(mmc_context_t* context);

/**
 * @brief			Do the RESET operation through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_do_reset(mmc_context_t* context);

/**
 * @brief			Set the state of automatic SET/RESET function of the control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		Automatic SET/RESET function state to be set. (PROPERTY_ENABLE = enable auto SET/RESET; PROPERTY_DISABLE = disable auto SET/RESET)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_auto_sr_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the state of automatic SET/RESET function from the control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		Buffer for automatic SET/RESET function state read, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_auto_sr_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Enable automatic self-test function through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_auto_st_en(mmc_context_t* context);

/**
 * @brief			Calculate the measurement period through control register 0.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_0_cmm_freq_en(mmc_context_t* context);

/**
 * @brief			Set the bandwidth selection of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		bandwidth selection to be set.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_bw_set(mmc_context_t* context, mmc5603_bw_t val);

/**
 * @brief			Get the bandwidth selection from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		Buffer for the bandwidth selection read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_bw_get(const mmc_context_t* context, mmc5603_bw_t* val);

/**
 * @brief			Set the X-axis channel inhibit state of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		X-axis channel inhibit state to be set. (PROPERTY_ENABLE = disable channel; PROPERTY_DISABLE = enable channel)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_x_inhibit_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the X-axis channel inhibit state from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the X-axis channel inhibit state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_x_inhibit_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the Y-axis channel inhibit state of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		Y-axis channel inhibit state to be set. (PROPERTY_ENABLE = disable channel; PROPERTY_DISABLE = enable channel)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_y_inhibit_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the Y-axis channel inhibit state from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the Y-axis channel inhibit state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_y_inhibit_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the Z-axis channel inhibit state of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		Z-axis channel inhibit state to be set. (PROPERTY_ENABLE = disable channel; PROPERTY_DISABLE = enable channel)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_z_inhibit_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the Z-axis channel inhibit state from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the Z-axis channel inhibit state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_z_inhibit_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the positive self-test DC current state of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		positive self-test DC current state to be set. (PROPERTY_ENABLE = positive current through coil; PROPERTY_DISABLE = no current through coil)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_st_enp_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the positive self-test DC current state from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the positive self-test DC current state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_st_enp_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the negative self-test DC current state of the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		negative self-test DC current state to be set. (PROPERTY_ENABLE = negative current through coil; PROPERTY_DISABLE = no current through coil)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_st_enm_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the negative self-test DC current state from the control register 1.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the negative self-test DC current state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_st_enm_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Software reset through control register 1.
 * @attention		This function will NOT clear control register shadow values.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_1_sw_reset(mmc_context_t* context);

/**
 * @brief			Set the periodic SET period selection of the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		periodic SET period selection to be set.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_prd_set_set(mmc_context_t* context, mmc5603_prd_st_t val);

/**
 * @brief			Get the periodic SET period selection from the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the periodic SET period selection read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_prd_set_get(const mmc_context_t* context, mmc5603_prd_st_t* val);

/**
 * @brief			Set the periodic SET state of the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		periodic SET state to be set.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_en_prd_set_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the periodic SET state from the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the periodic SET state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_en_prd_set_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the continuous mode state of the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		continuous mode state to be set.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t WEAK mmc5603_ll_control_2_cmm_en_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the continuous mode state from the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the continuous mode state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_cmm_en_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief			Set the high power state of the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		high power state to be set.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_hpower_set(mmc_context_t* context, uint8_t val);

/**
 * @brief			Get the high power state from the control register 2.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the high power state read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_ll_control_2_hpower_get(const mmc_context_t* context, uint8_t* val);

/**
 * @brief MMC5603 device setup struct.
 */
typedef struct {
	uint8_t				odr;					/*!< The output data rate, or 0 to enable on-demand mode. */
	uint8_t				high_power;				/*!< PROPERTY_ENABLE if enable 1000Hz output data rate. */
	uint8_t				auto_set_reset;			/*!< PROPERTY_ENABLE if enable automatic SET/RESET. (Recommended to be PROPERTY_ENABLE) */
	uint8_t				periodic_set;			/*!< PROPERTY_ENABLE if enable periodic SET. */
	mmc5603_prd_st_t	periodic_set_period;	/*!< The frequency of the periodic SET. */
	mmc5603_bw_t		bandwidth;				/*!< The bandwidth. (measurement time) */
} mmc5603_hl_setup_t;

/**
 * @brief			Set up device.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		the parameters to set up the device.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_setup(mmc_context_t* context, mmc5603_hl_setup_t val);

/**
 * @brief			Perform the self-test.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		the buffer to hold the result of the self-test. ("0" = working properly; "1" = not passing the self-test)
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_selftest(mmc_context_t* context, uint8_t* val);

/**
 * @brief			Software reset.
 * @attention		This function will clear control register shadow values.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_software_reset(mmc_context_t* context);

/**
 * @brief MMC5603 magnetometer data output struct in Gauss.
 */
typedef struct {
	uint8_t done;		/*!< The magnetic field measurement done status of this read. ("0" = measurement is not done; "1" = measurement is done) */
	float_t x_gauss;	/*!< Magnetometer data output of X-Axis in Gauss. (or undefined if measurement is not done) */
	float_t y_gauss;	/*!< Magnetometer data output of Y-Axis in Gauss. (or undefined if measurement is not done) */
	float_t z_gauss;	/*!< Magnetometer data output of Z-Axis in Gauss. (or undefined if measurement is not done) */
} mmc5603_hl_mag_out_t;

/**
 * @brief			Get the 16-bit data output in gauss.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the 16-bit magnetometer data read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_mag_16_get(const mmc_context_t* context, mmc5603_hl_mag_out_t* val);

/**
 * @brief			Get the 20-bit data output in gauss.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param val		buffer for the 20-bit magnetometer data read.
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_mag_20_get(const mmc_context_t* context, mmc5603_hl_mag_out_t* val);

/**
 * @brief			Get the temperature data output in degree(s) Celsius.
 * @param context	implementation of the platform independent function interfaces, as pointer.
 * @param done		The buffer to hold the measurement done state read.
 * @param val		The buffer to hold the temperature data read (or undefined if measurement is not done).
 * @retval			status of implementation function interfaces. ("0" = no error)
 */
int32_t mmc5603_hl_temp_get(
	const	mmc_context_t*	context,
			uint8_t*		done,
			float_t*		val
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMC5603_REG_H