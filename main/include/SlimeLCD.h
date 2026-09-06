#ifndef SLIME_LCD_H
#define SLIME_LCD_H

#include "esp_lcd_panel_dev.h"
#include "esp_fast_lcd.h"
#include "SlimeCommon.h"

// The entrypoint function of LCD panel initialization.
esp_err_t newLCDDeviceHandle(esp_fast_lcd_panel_device_t** context);

#endif // SLIME_LCD_H
