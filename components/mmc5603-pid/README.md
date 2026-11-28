# 1 - Introduction

Sensor driver for MMC5603 sensor written in C programming language. This repository contains the sensor driver files (.h and .c) to be included, or linked directly as a git submodule, in your project. This sensor driver is highly inspired by [STMicroelectronics/lsm6dsv-pid](https://github.com/STMicroelectronics/lsm6dsv-pid) and refers to its coding and text (e.g. README.md) style.

To `clone` the complete content of the repository, use the command:

```
git clone https://github.com/Argon4W/mmc5603-pid/
```

Currently, there is no examples of driver usage.

------

# 2 - Integration details

The driver is platform-independent, you only need to define two functions for read and write transactions from the sensor hardware bus (e.g. I2C or auxiliary sensor interface) and an optional one to implement a delay of millisecond granularity. **A few devices integrate an extra bit in the communication protocol in order to enable multi read/write access, this bit must be managed in the read and write functions defined by the user.**

### 2.a Source code integration

- Include in your project the driver files of the sensor (.h and .c)
- Define in your code the read and write functions that use the I2C or auxiliary sensor interface implementation driver like the following:

```c++
/** return value: 0 = "no error occurred."; other = "error occurs" */
int32_t read_registerd_impl(void* user_handle, uint8_t address, uint8_t *buffer, uint16_t length)
int32_t write_registerd_impl(void* user_handle, uint8_t address, const uint8_t *buffer, uint16_t length)

/** Optional (Required by the self-test) */
void delay_milliseconds_impl(uint32_t milliseconds)
```

- Declare and initialize the structure of the device context:

```c++
mmc_context_t dev_ctx;
dev_ctx.read_register_function = read_registerd_impl;
dev_ctx.write_register_function = write_registerd_impl;
dev_ctx.delay_milliseconds_function = delay_milliseconds_impl;

```

- If needed by the implementation read and write functions, initialize the user-defined custom handle parameter:

```c++
dev_ctx.user_handle = &implementation_handle;
```

### 2.b Required properties

> - A standard C language compiler for the target MCU
> - A C library for the target MCU and the desired interface (e.g. I2C, auxiliary sensor interface)
