#ifndef SLIME_MAIN_H
#define SLIME_MAIN_H

#include "slime_nvs.h"
#include "slime_gpio.h"
#include "slime_i2c.h"
#include "slime_lcd.h"
#include "slime_screen.h"
#include "slime_button.h"
#include "slime_magneto.h"
#include "slime_sensor.h"
#include "slime_config.h"

#define VQF_MATRIX_HANDLE_TYPE			ceigen_matrix_handle_t
#define VQF_MATRIX_DOUBLE_HANDLE_TYPE	ceigen_matrix_double_handle_t
#define VQF_QUATERNION_HANDLE_TYPE		ceigen_quaternion_handle_t

#include "vqf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SLIME_MAIN_H
