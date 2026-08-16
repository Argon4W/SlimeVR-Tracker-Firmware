//
// Created by progr on 2026/8/16.
//

#ifndef SLIME_I2C_H
#define SLIME_I2C_H

#include "driver/i2c_master.h"
#include "SlimeCommon.h"

// Function types of platform independent I2C operation functions.
typedef func(PlatformI2CWriteFunc,	s32		/* return */ ,	opaque	/* userHandle */, u8 /* registerAddress */, const_ptr	(u8) /* buffer */, u16  /* length */);	// Write bytes to a given register address.
typedef func(PlatformI2CReadFunc,	s32		/* return */ ,	opaque	/* userHandle */, u8 /* registerAddress */, ptr			(u8) /* buffer */, u16  /* length */);	// Read bytes from a given register address.
typedef func(PlatformDelayFunc,		void	/* return */,	u32		/* milliseconds */);																			// Delay milliseconds.

typedef struct {
	i2c_master_dev_handle_t	I2CDeviceHandle;
	const_string			I2CDeviceName;
	u8						I2CDeviceAddress;
} I2CDeviceContext;

typedef struct {
	// ESP I2C Handles for resource management.
	i2c_master_bus_handle_t	I2CMasterBus;				// I2C master bus handle.
	ptr(I2CDeviceContext)	LSM6DSV_I2CDeviceContext;	// I2C master device handle for LSM6DSV.
	ptr(I2CDeviceContext)	QMC6309_I2CDeviceContext;	// I2C master device handle for QMC6309.

	// Function handles of platform independent I2C operation functions.
	PlatformI2CWriteFunc	platformI2CWriteFunc;	// I2C write function handle.
	PlatformI2CReadFunc		platformI2CReadFunc;	// I2c read function handle.
	PlatformDelayFunc		platformDelayFunc;		// Delay function handle.
} I2CRuntimeContext;

// The entry point functions of the I2C initialization/deinitialization.
esp_err_t newI2CRuntimeContext		(ptr(I2CRuntimeContext) I2CRuntimeContextOut);
esp_err_t deleteI2CRuntimeContext	(ptr(I2CRuntimeContext) I2CRuntimeContextIn);

#endif // SLIME_I2C_H
