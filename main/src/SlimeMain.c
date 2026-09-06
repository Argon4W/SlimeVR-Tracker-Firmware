#include <stdio.h>
#include <tgmath.h>
#include <sys/time.h>

#include "driver/gpio.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "lsm6dsv_reg.h"
#include "qmc6309_reg.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unifont-17.0.05.h"
#include "esp_fast_text_engine.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "SlimeMain.h"
#include "utf8.h"
#include "esp_hmac.h"

static const_string TAG = "SlimeMain";

const static qmc6309_setup_t qmc6309Setup = {
	.mode			= NORMAL,
	.set_reset_mode	= SET_RESET_ON,
	.rng			= RNG_8G,
	.odr			= ODR_200HZ,
	.osr1			= OSR1_8,
	.osr2			= OSR2_8
};

static lsm6dsv_sh_cfg_read_t qmc6309StatusReadConfig = {
	.slv_add	= QMC6309_I2C_ADDRESS << 1,
	.slv_subadd	= QMC6309_STATUS_1,
	.slv_len	= 1
};

static lsm6dsv_sh_cfg_read_t qmc6309OutputReadConfig = {
	.slv_add	= QMC6309_I2C_ADDRESS << 1,
	.slv_subadd	= QMC6309_OUT_X_L,
	.slv_len	= 6
};

static magneto_linear_algebra_context_t magnetoContext = {
	.new_matrix							= newMatrixCEigen,
	.delete_matrix						= deleteMatrixCEigen,
	.get_matrix_coefficient				= getMatrixCoefficientCEigen,
	.set_matrix_coefficient				= setMatrixCoefficientCEigen,
	.add_matrix_coefficient				= addMatrixCoefficientCEigen,
	.multiply_matrix_coefficient		= multiplyMatrixCoefficientCEigen,
	.copy_matrix						= copyMatrixCEigen,
	.copy_matrix_block					= copyMatrixBlockCEigen,
	.multiply_matrix					= multiplyMatrixCEigen,
	.subtract_matrix					= subtractMatrixCEigen,
	.invert_matrix_in_place				= invertMatrixInPlaceCEigen,
	.transpose_matrix_in_place			= transposeMatrixInPlaceCEigen,
	.normalize_matrix_in_place			= normalizeMatrixInPlaceCEigen,
	.multiply_matrix_scalar_in_place	= multiplyMatrixScalarInPlaceCEigen,
	.solve_matrix_eigen					= solveMatrixEigenCEigen,
};

static esp_fast_text_engine_instance_configuration_t textEngineConfig = {
	.font_size_multiplier	= 1U,
	.iram_atlas_slot_count	= 18U,
	.psram_atlas_slot_count	= 32U,
	.atlas_flags			= 0U,
	.name					= "SlimeFontEngine"
};

void cross(
	const float	aIn		[3],
	const float	bIn		[3],
	float		cOut	[3]
) {
	cOut[0] = aIn[1] * bIn[2] - aIn[2] * bIn[1];
	cOut[1] = aIn[2] * bIn[0] - aIn[0] * bIn[2];
	cOut[2] = aIn[0] * bIn[1] - aIn[1] * bIn[0];
}

// IO0	: Button 1 (Top-Left)
// IO39	: Button 2 (Top-Right)
// IO18	: Button 3 (Bottom-Right)

void app_main(void) {
	esp_err_t ret = ESP_OK;

	size_t									nvsSize;
	nvs_handle_t							nvsHandle;
	ptr(esp_fast_lcd_panel_device_t)		lcdContext				= NULL;
	ptr(esp_fast_text_engine_instance_t)	textEngineContext		= NULL;
	ptr(I2CRuntimeContext)					i2cContext				= alloc_ptr						(I2CRuntimeContext);
	ptr(qmc_context_t)						qmcContext				= alloc_ptr						(qmc_context_t);
	ptr(stmdev_ctx_t)						lsmContext				= alloc_ptr						(stmdev_ctx_t);
	ptr(float_t)							magnetoNVSCalibration	= alloc_arr						(float_t, 12);
	ptr(magneto_sample_container_t)			magnetoSampleContainer	= magneto_new_sample_container	(&magnetoContext);
	ptr(magneto_matrix_t)					magnetoSoftIronMatrix	= newMatrixCEigen				(3, 3);
	ptr(magneto_matrix_t)					magnetoHardIronVector	= newMatrixCEigen				(3, 1);
	ptr(magneto_matrix_t)					magnetoCalibrateInput	= newMatrixCEigen				(3, 1);
	ptr(magneto_matrix_t)					magnetoUnbiasedOutput	= newMatrixCEigen				(3, 1);
	ptr(magneto_matrix_t)					magnetoCalibrateOutput	= newMatrixCEigen				(3, 1);
	u8										magnetoCalibrated		= 0U;

	// create gpio button
	const button_config_t btn_cfg = {0};
	const button_gpio_config_t btn_gpio_cfg = {
		.gpio_num = 39,
		.active_level = 0
	};
	button_handle_t gpio_btn = NULL;
	iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &gpio_btn);

	if(NULL == gpio_btn) {
		ESP_LOGE(TAG, "Button create failed");
	}

	esp_err_t err = nvs_flash_init();

	if (	err == ESP_ERR_NVS_NO_FREE_PAGES
		||	err == ESP_ERR_NVS_NEW_VERSION_FOUND
	) {
		ESP_GOTO_ON_ERROR(nvs_flash_erase	(), error, TAG, "Failed to erase non-volatile storage flash.");
		ESP_GOTO_ON_ERROR(nvs_flash_init	(), error, TAG, "Failed to initialize non-volatile storage flash.");
	}

	esp_fast_text_engine_font_t unifontFont = {
		.size_x_max		= 16,
		.size_y			= 16,
		.glyph_data		= unifont_17_0_05_1bpp_bits,
		.glyph_table	= glyph_table,
	};

	ESP_GOTO_ON_ERROR(newI2CRuntimeContext							(i2cContext),											error, TAG, "Failed to create I2C runtime context.");
	ESP_GOTO_ON_ERROR(newLCDDeviceHandle							(&lcdContext),											error, TAG, "Failed to create LCD panel device.");
	ESP_GOTO_ON_ERROR(esp_fast_text_engine_new_text_engine_instance	(&textEngineContext, textEngineConfig, unifontFont),	error, TAG, "Failed to create text engine instance.");
	ESP_GOTO_ON_ERROR(nvs_open										(TAG, NVS_READWRITE, &nvsHandle),						error, TAG, "Failed to open non-volatile storage flash handle.");

	// Try getting the magneto calibration data from NVS.
	esp_err_t nvsError = nvs_get_blob(nvsHandle, "magneto", magnetoNVSCalibration, &nvsSize);

	// Check if the NVS magneto calibration data is present.
	if (nvsError == ESP_OK) {
		// Mark as calibrated.
		magnetoCalibrated = true;

		// Fill the soft iron matrix and with the NVS magneto calibration data.
		for		(s32 row = 0; row < 3; row ++) {
			for	(s32 col = 0; col < 3; col ++) {
				setMatrixCoefficientCEigen(
					/* matrix	= */ magnetoSoftIronMatrix,
					/* row		= */ row,
					/* column	= */ col,
					/* value	= */ magnetoNVSCalibration[row * 3 + col]
				);
			}
		}

		// Fill the hard iron vector with the NVS magneto calibration data.
		setMatrixCoefficientCEigen(magnetoHardIronVector, 0, 0, magnetoNVSCalibration[3 * 3 + 0]); // Load the X-axis hard iron bias.
		setMatrixCoefficientCEigen(magnetoHardIronVector, 1, 0, magnetoNVSCalibration[3 * 3 + 1]); // Load the Y-axis hard iron bias.
		setMatrixCoefficientCEigen(magnetoHardIronVector, 2, 0, magnetoNVSCalibration[3 * 3 + 2]); // Load the Z-axis hard iron bias.
	} else if (nvsError != ESP_ERR_NVS_NOT_FOUND) {
		ESP_LOGE(TAG, "Failed to load NVS magneto calibration data from non-volatile storage flash.");
	}

	// Close the NVS handle, release the allocated memory.
	nvs_close(nvsHandle);

	gpio_config_t ledConfig = {
		.pin_bit_mask	= (1ULL << CONFIG_SENSOR_LED),
		.mode			= GPIO_MODE_OUTPUT,
		.pull_up_en		= GPIO_PULLUP_DISABLE,
		.pull_down_en	= GPIO_PULLDOWN_DISABLE,
		.intr_type		= GPIO_INTR_DISABLE
	};

	ESP_GOTO_ON_ERROR(gpio_config(&ledConfig), error, TAG, "Failed to config LED.");

	qmcContext->delay_milliseconds	= i2cContext->platformDelay;
	qmcContext->write_register		= i2cContext->platformI2CWrite;
	qmcContext->read_register		= i2cContext->platformI2CRead;
	qmcContext->user_handle			= i2cContext->qmc6309_I2CDeviceContext;

	lsmContext->mdelay		= i2cContext->platformDelay;
	lsmContext->write_reg	= i2cContext->platformI2CWrite;
	lsmContext->read_reg	= i2cContext->platformI2CRead;
	lsmContext->handle		= i2cContext->lsm6dsv_I2CDeviceContext;

	i2cContext->platformDelay(100);

	u8 lsm6dsvID = 0x00U;
	u8 qmc6309ID = 0x00U;

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sw_por							(lsmContext)),						error, TAG, "Failed to soft reset LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_master_interface_pull_up_set(lsmContext, PROPERTY_DISABLE)),	error, TAG, "Failed to disable master I2C internal pull-up of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_master_set					(lsmContext, PROPERTY_DISABLE)),	error, TAG, "Failed to disable master I2C of LSM6DSV.");

	vTaskDelay(pdMS_TO_TICKS(300));

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_pass_through_set(lsmContext, PROPERTY_ENABLE)), error, TAG, "Failed to enable I2C passthrough of LSM6DSV.");

	vTaskDelay(pdMS_TO_TICKS(300));

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_device_id_get		(lsmContext, &lsm6dsvID)),		error, TAG, "Failed to read device ID of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, qmc6309_raw_chip_id_get	(qmcContext, &qmc6309ID)),		error, TAG, "Failed to read chip ID of QMC6309.");

	ESP_GOTO_ON_FALSE((lsm6dsvID == LSM6DSV_ID),			ESP_ERR_INVALID_RESPONSE, error, TAG, "Wrong LSM6DSV ID: 0x%02" PRIX8, lsm6dsvID);
	ESP_GOTO_ON_FALSE((qmc6309ID == QMC6309_CHIP_ID_REF),	ESP_ERR_INVALID_RESPONSE, error, TAG, "Wrong QMC6309 ID: 0x%02" PRIX8, qmc6309ID);

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, qmc6309_hl_setup(qmcContext, qmc6309Setup)), error, TAG, "Failed to setup QMC6309.");

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_pass_through_set			(lsmContext, PROPERTY_DISABLE)),			error, TAG, "Failed to disable I2C passthrough of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_master_interface_pull_up_set(lsmContext, PROPERTY_ENABLE)),				error, TAG, "Failed to enable master I2C internal pull-up of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_slv_cfg_read				(lsmContext, 0, &qmc6309StatusReadConfig)),	error, TAG, "Failed to setup sensor hub QMC6309 status register read of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_slv_cfg_read				(lsmContext, 1, &qmc6309OutputReadConfig)),	error, TAG, "Failed to setup sensor hub QMC6309 output registers read of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_data_rate_set				(lsmContext, LSM6DSV_SH_240Hz)),			error, TAG, "Failed to set sensor hub data rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_slave_connected_set			(lsmContext, LSM6DSV_SLV_0_1)),				error, TAG, "Failed to set slave connected state of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_write_mode_set				(lsmContext, LSM6DSV_ONLY_FIRST_CYCLE)),	error, TAG, "Failed to set sensor hub write mode of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_sh_master_set					(lsmContext, PROPERTY_ENABLE)),				error, TAG, "Failed to enable master I2C of LSM6DSV.");

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_block_data_update_set	(lsmContext, PROPERTY_ENABLE)),				error, TAG, "Failed to enable block data update of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_gy_full_scale_set		(lsmContext, LSM6DSV_1000dps)),				error, TAG, "Failed to set gyroscope full scale range of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_xl_full_scale_set		(lsmContext, LSM6DSV_4g)),					error, TAG, "Failed to set accelerometer full scale range of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_gy_data_rate_set		(lsmContext, LSM6DSV_ODR_HA01_AT_1000Hz)),	error, TAG, "Failed to set gyroscope output data rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_xl_data_rate_set		(lsmContext, LSM6DSV_ODR_HA01_AT_1000Hz)),	error, TAG, "Failed to set accelerometer output data rate of LSM6DSV.");

	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_watermark_set			(lsmContext, 1)),							error, TAG, "Failed to set FIFO watermark of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_gy_batch_set			(lsmContext, LSM6DSV_GY_BATCHED_AT_960Hz)),	error, TAG, "Failed to set FIFO gyroscope batch rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_xl_batch_set			(lsmContext, LSM6DSV_XL_BATCHED_AT_960Hz)),	error, TAG, "Failed to set FIFO accelerometer batch rate of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_sh_batch_slave_set	(lsmContext, 0, PROPERTY_ENABLE)),			error, TAG, "Failed to enable sensor hub slave 0 FIFO batch of LSM6DSV.");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_sh_batch_slave_set	(lsmContext, 1, PROPERTY_ENABLE)),			error, TAG, "Failed to enable sensor hub slave 1 FIFO batch of LSM6DSV");
	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_mode_set				(lsmContext, LSM6DSV_STREAM_MODE)),			error, TAG, "Failed to set FIFO mode of LSM6DSV.");

	gpio_set_level(CONFIG_SENSOR_LED, 1);

	float_t calibratedMagnetometerOutput[3];

	while (1) {
		struct timeval stop, start;
		gettimeofday(&start, NULL);
		gettimeofday(&start, NULL);

		esp_fast_lcd_draw_rectangle(
			/* context			= */ lcdContext,
			/* position_x		= */ 0,
			/* position_y		= */ 0,
			/* size_x			= */ 96,
			/* size_y			= */ 54,
			/* color_rgba8888	= */ 0x000000FFU
		);

		string	str	= "加速度";
		s32		x	= 24;

		if (iot_button_get_event(gpio_btn) == BUTTON_PRESS_DOWN) {
			str = "磁力计";
		}

		s32 codepoint;

		while (*str != '\0') {
			str = utf8codepoint(str, &codepoint);

			esp_fast_text_engine_draw_glyph(
				/* text_engine_context	= */ textEngineContext,
				/* panel_device_context	= */ lcdContext,
				/* codepoint			= */ codepoint,
				/* position_x			= */ x,
				/* position_y			= */ 11,
				/* color_rgba8888		= */ 0xFFFFFFFFU,
				/* advance_x			= */ &x
			);
		}

		str	= "校准";
		x	= 32;

		while (*str != '\0') {
			str = utf8codepoint(str, &codepoint);

			esp_fast_text_engine_draw_glyph(
				/* text_engine_context	= */ textEngineContext,
				/* panel_device_context	= */ lcdContext,
				/* codepoint			= */ codepoint,
				/* position_x			= */ x,
				/* position_y			= */ 27,
				/* color_rgba8888		= */ 0xFFFFFFFFU,
				/* advance_x			= */ &x
			);
		}

		gettimeofday(&stop, NULL);
		printf("took %lld us\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

		esp_fast_lcd_commit(lcdContext);

		lsm6dsv_fifo_status_t	fifoStatus	= {0};
		lsm6dsv_fifo_out_raw_t	fifoOutRaw	= {0};
		u16						fifoCount	= 0U;
		u8						fifoMag		= 0U;

		// ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_status_get(lsmContext, &fifoStatus)), error, TAG, "Failed to get FIFO status of LSM6DSV.");
		//
		// fifoCount = fifoStatus.fifo_level;
		//
		// while (fifoCount --) {
		// 	ESP_GOTO_ON_ERROR(cast_to(esp_err_t, lsm6dsv_fifo_out_raw_get(lsmContext, &fifoOutRaw)), error, TAG, "Failed to get FIFO raw output of LSM6DSV.");
		//
		// 	const s16 rawDataX = pack_s16(fifoOutRaw.data[1], fifoOutRaw.data[0]);
		// 	const s16 rawDataY = pack_s16(fifoOutRaw.data[3], fifoOutRaw.data[2]);
		// 	const s16 rawDataZ = pack_s16(fifoOutRaw.data[5], fifoOutRaw.data[4]);
		//
		// 	switch (fifoOutRaw.tag) {
		// 		case LSM6DSV_GY_NC_TAG:
		// 			// ESP_LOGI(TAG, "LSM6DSV gyroscope data output: %.2f, %.2f, %.2f",
		// 			// 		/* f */ lsm6dsv_from_fs4_to_mg(rawDataX),
		// 			// 		/* f */ lsm6dsv_from_fs4_to_mg(rawDataY),
		// 			// 		/* f */ lsm6dsv_from_fs4_to_mg(rawDataZ)
		// 			// );
		// 			break;
		// 		case LSM6DSV_XL_NC_TAG:
		// 			if (magnetoCalibrated) {
		// 				float_t westVectorOutput	[3];
		// 				float_t northVectorOutput	[3];
		// 				float_t accelerometerOutput	[3] = {
		// 					- lsm6dsv_from_fs4_to_mg(rawDataX),
		// 					- lsm6dsv_from_fs4_to_mg(rawDataY),
		// 					- lsm6dsv_from_fs4_to_mg(rawDataZ)
		// 				};
		//
		// 				float_t norm =	sqrt(
		// 					accelerometerOutput[0] * accelerometerOutput[0] +
		// 					accelerometerOutput[1] * accelerometerOutput[1] +
		// 					accelerometerOutput[2] * accelerometerOutput[2]
		// 				);
		//
		// 				accelerometerOutput[0] /= norm;
		// 				accelerometerOutput[1] /= norm;
		// 				accelerometerOutput[2] /= norm;
		//
		// 				cross(calibratedMagnetometerOutput,	accelerometerOutput, westVectorOutput);
		// 				cross(westVectorOutput,				accelerometerOutput, northVectorOutput);
		//
		// 				ESP_LOGI(TAG, "LSM6DSV+QMC6309 heading output: %.2f, %.2f, %.2f",
		// 						/* f */ northVectorOutput[0],
		// 						/* f */ northVectorOutput[1],
		// 						/* f */ northVectorOutput[2]
		// 				);
		// 			}
		//
		// 			break;
		// 		case LSM6DSV_SENSORHUB_SLAVE0_TAG:
		// 			const ptr(qmc6309_status_1_t) qmc6309Status = cast_ptr(qmc6309_status_1_t, ref(fifoOutRaw.data[0]));
		//
		// 			if (	qmc6309Status->drdy_bit != 0
		// 				&&	qmc6309Status->ovfl_bit == 0
		// 			) {
		// 				fifoMag = 1U;
		// 			}
		// 			break;
		// 		case LSM6DSV_SENSORHUB_SLAVE1_TAG:
		// 			if (fifoMag) {
		// 				fifoMag = 0U;
		//
		// 				const float_t qmc6309RawGaussX = qmc6309_ll_from_rng8_to_gauss(rawDataX);
		// 				const float_t qmc6309RawGaussY = qmc6309_ll_from_rng8_to_gauss(rawDataY);
		// 				const float_t qmc6309RawGaussZ = qmc6309_ll_from_rng8_to_gauss(rawDataZ);
		//
		// 				if (!magnetoCalibrated) {
		// 					if (magnetoSampleContainer->sample_norm_count < 30 * 200) {
		// 						ESP_LOGI(TAG, "QMCC6309 magnetometer calibration raw data output: %.2f, %.2f, %.2f",
		// 							/* f */ qmc6309RawGaussX,
		// 							/* f */ qmc6309RawGaussY,
		// 							/* f */ qmc6309RawGaussZ
		// 						);
		//
		// 						// Add the raw magnetometer output gauss as the sample of the calibration.
		// 						magneto_sample(
		// 							/* context			= */ &magnetoContext,
		// 							/* sample_container	= */ magnetoSampleContainer,
		// 							/* sample_x			= */ qmc6309RawGaussX,
		// 							/* sample_y			= */ qmc6309RawGaussY,
		// 							/* sample_z			= */ qmc6309RawGaussZ
		// 						);
		// 					} else {
		// 						ESP_LOGI(TAG, "QMC6309 Calibrating.");
		//
		// 						// Calibrate.
		// 						magneto_calculate(
		// 							/* context			= */ &magnetoContext,
		// 							/* sample_container	= */ magnetoSampleContainer,
		// 							/* soft_iron_matrix	= */ magnetoSoftIronMatrix,
		// 							/* hard_iron_vector	= */ magnetoHardIronVector
		// 						);
		//
		// 						// Print the calibrated soft iron matrix to the serial.
		// 						ESP_LOGI(TAG, "QMC6309 Calibrated.");
		// 						ESP_LOGI(TAG, "QMC6309 Calibrated soft iron matrix: ");
		// 						ESP_LOGI(TAG, "[");
		//
		// 						// Print rows of the soft iron matrix.
		// 						for (s32 row = 0; row < 3; row ++) {
		// 							ESP_LOGI(TAG, "    %.2f, %.2f, %.2f",
		// 								getMatrixCoefficientCEigen(magnetoSoftIronMatrix, row, 0),
		// 								getMatrixCoefficientCEigen(magnetoSoftIronMatrix, row, 1),
		// 								getMatrixCoefficientCEigen(magnetoSoftIronMatrix, row, 2)
		// 							);
		// 						}
		//
		// 						ESP_LOGI(TAG, "]");
		//
		// 						// Print the calibrated hard iron vector to the serial.
		// 						ESP_LOGI(TAG, "QMC 6309 Calibrated hard iron vector: ");
		// 						ESP_LOGI(TAG, "[");
		// 						ESP_LOGI(TAG, "   %.2f,", getMatrixCoefficientCEigen(magnetoHardIronVector, 0, 0));
		// 						ESP_LOGI(TAG, "   %.2f,", getMatrixCoefficientCEigen(magnetoHardIronVector, 1, 0));
		// 						ESP_LOGI(TAG, "   %.2f,", getMatrixCoefficientCEigen(magnetoHardIronVector, 2, 0));
		// 						ESP_LOGI(TAG, "]");
		//
		// 						// Mark as calibrated.
		// 						magnetoCalibrated = true;
		//
		// 						// Open the NVS handle for saving calibration data.
		// 						ESP_GOTO_ON_ERROR(nvs_open(TAG, NVS_READWRITE, &nvsHandle), error, TAG, "Failed to open non-volatile storage flash handle.");
		//
		// 						// Extract the coefficients from the CEigen matrices first.
		// 						// Extract the coefficients from the soft iron matrix.
		// 						for		(s32 row = 0; row < 3; row ++) {
		// 							for	(s32 col = 0; col < 3; col ++) {
		// 								magnetoNVSCalibration[row * 3 + col] = getMatrixCoefficientCEigen(magnetoSoftIronMatrix, row, col);
		// 							}
		// 						}
		//
		// 						// Extract the coefficients from the hard iron vector.
		// 						magnetoNVSCalibration[3 * 3 + 0] = getMatrixCoefficientCEigen(magnetoHardIronVector, 0, 0); // Extract the X-axis calibration bias.
		// 						magnetoNVSCalibration[3 * 3 + 1] = getMatrixCoefficientCEigen(magnetoHardIronVector, 1, 0); // Extract the Y-axis calibration bias.
		// 						magnetoNVSCalibration[3 * 3 + 2] = getMatrixCoefficientCEigen(magnetoHardIronVector, 2, 0); // Extract the Z-axis calibration bias.
		//
		// 						// Save the extracted data to the NVS.
		// 						ESP_GOTO_ON_ERROR(nvs_set_blob	(nvsHandle, "magneto", magnetoNVSCalibration, sizeof(float_t) * 12),	error, TAG, "Failed to save calibration coefficients to non-volatile storage flash.");
		// 						ESP_GOTO_ON_ERROR(nvs_commit	(nvsHandle),															error, TAG, "Failed to commit data changes on non-volatile storage flash.");
		//
		// 						// Close the NVS handle, release the allocated memory.
		// 						nvs_close(nvsHandle);
		// 					}
		// 				} else {
		// 					// Load the raw magnetometer data into the magneto calibration input vector.
		// 					setMatrixCoefficientCEigen(magnetoCalibrateInput, 0, 0, qmc6309RawGaussX); // Load the X axis uncalibrated data into the input vector.
		// 					setMatrixCoefficientCEigen(magnetoCalibrateInput, 1, 0, qmc6309RawGaussY); // Load the Y axis uncalibrated data into the input vector.
		// 					setMatrixCoefficientCEigen(magnetoCalibrateInput, 2, 0, qmc6309RawGaussZ); // Load the Z axis uncalibrated data into the input vector.
		//
		// 					// Unbias the input and write it to the unbiased output vector.
		// 					subtractMatrixCEigen(
		// 						/* left_matrix			= */ magnetoCalibrateInput,
		// 						/* right_matrix			= */ magnetoHardIronVector,
		// 						/* destination_matrix	= */ magnetoUnbiasedOutput
		// 					);
		//
		// 					// Calibrate the unbiased magnetometer output with the soft iron matrix.
		// 					multiplyMatrixCEigen(
		// 						/* left_matrix			= */ magnetoSoftIronMatrix,
		// 						/* right_matrix			= */ magnetoUnbiasedOutput,
		// 						/* destination_matrix	= */ magnetoCalibrateOutput
		// 					);
		//
		// 					// Normalize the calibration output.
		// 					normalizeMatrixInPlaceCEigen(magnetoCalibrateOutput);
		//
		// 					// Store the current calibrated magnetometer output.
		// 					calibratedMagnetometerOutput[0] = getMatrixCoefficientCEigen(magnetoCalibrateOutput, 0, 0);	// The X axis of the calibrated magnetometer output.
		// 					calibratedMagnetometerOutput[1] = getMatrixCoefficientCEigen(magnetoCalibrateOutput, 1, 0);	// The Y axis of the calibrated magnetometer output.
		// 					calibratedMagnetometerOutput[2] = getMatrixCoefficientCEigen(magnetoCalibrateOutput, 2, 0);	// The Z axis of the calibrated magnetometer output.
		// 				}
		// 			}
		// 			break;
		// 		default:
		// 			ESP_LOGE(TAG, "Invalid FIFO tag: 0x%02." PRIX8, fifoOutRaw.tag);
		// 			break;
		// 	}
		// }
	}

	error: {
		ESP_LOGE(TAG, "Error occurred: 0x%" PRIX8, ret);
		ESP_LOGE(TAG, "Cleaning up resources.");

		// ESP_ERROR_CHECK(deleteI2CRuntimeContext(i2cContext));

		if (nvsHandle) {
			nvs_close(nvsHandle);
		}

		free(magnetoNVSCalibration);
		free(qmcContext);
		// free(lsmContext);
		// free(i2cContext);

		magneto_delete_sample_container	(&magnetoContext, magnetoSampleContainer);
		deleteMatrixCEigen				(magnetoSoftIronMatrix);
		deleteMatrixCEigen				(magnetoHardIronVector);
		deleteMatrixCEigen				(magnetoCalibrateInput);
		deleteMatrixCEigen				(magnetoUnbiasedOutput);
		deleteMatrixCEigen				(magnetoCalibrateOutput);

		while (1) {
			if (lsm6dsvID != LSM6DSV_ID) {
				u8 deviceId;
				lsm6dsv_device_id_get(lsmContext, &deviceId);

				gpio_set_level(CONFIG_SENSOR_LED, 0);
				ESP_LOGE(TAG, "LSM6DSV broken. 0x%02" PRIX8, deviceId);
			} else if (qmc6309ID != QMC6309_CHIP_ID_REF) {
				ESP_LOGE(TAG, "QMC6309 broken.");
				gpio_set_level(CONFIG_SENSOR_LED, 0);
				vTaskDelay(pdMS_TO_TICKS(250));
				gpio_set_level(CONFIG_SENSOR_LED, 1);
				vTaskDelay(pdMS_TO_TICKS(250));
			} else {
				gpio_set_level(CONFIG_SENSOR_LED, 0);
				vTaskDelay(pdMS_TO_TICKS(1000));
				gpio_set_level(CONFIG_SENSOR_LED, 1);
				vTaskDelay(pdMS_TO_TICKS(1000));
			}
		}
	}
}