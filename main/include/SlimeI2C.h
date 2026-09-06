#ifndef SLIME_I2C_H
#define SLIME_I2C_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "driver/i2c_master.h"
#include "SlimeCommon.h"

// Function types of platform independent I2C operation functions.
typedef func(PlatformI2CWriteFunction,	s32		/* return */, opaque	userHandle, u8 registerAddress, const	ptr(u8) buffer, u16  length);	// Write bytes to a given register address.
typedef func(PlatformI2CReadFunction,	s32		/* return */, opaque	userHandle, u8 registerAddress,			ptr(u8) buffer, u16  length);	// Read bytes from a given register address.
typedef func(PlatformDelayFunction,		void	/* return */, u32		milliseconds);															// Delay milliseconds.

// The device context of I2C devices.
typedef struct {
	i2c_master_dev_handle_t	deviceHandle;	// The I2C device handle of the device.
	const_string			deviceName;		// The name of the I2C device.
	u8						deviceAddress;	// The address of the I2C device.
} I2CDeviceContext;

// The runtime context of the I2C.
typedef struct {
	// ESP I2C Handles for resource management.
	i2c_master_bus_handle_t	i2cMasterBus;				// I2C master bus handle.
	ptr(I2CDeviceContext)	lsm6dsv_I2CDeviceContext;	// I2C master device handle for LSM6DSV.
	ptr(I2CDeviceContext)	qmc6309_I2CDeviceContext;	// I2C master device handle for QMC6309.

	// Function handles of platform independent I2C operation functions.
	PlatformI2CWriteFunction	platformI2CWrite;	// I2C write function handle.
	PlatformI2CReadFunction		platformI2CRead;	// I2c read function handle.
	PlatformDelayFunction		platformDelay;		// Delay function handle.
} I2CRuntimeContext;

// The entry point functions of the I2C initialization/deinitialization.
esp_err_t newI2CRuntimeContext		(ptr(I2CRuntimeContext) i2cRuntimeContextOut);
esp_err_t deleteI2CRuntimeContext	(ptr(I2CRuntimeContext) i2cRuntimeContextIn);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_I2C_H
