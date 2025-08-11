#pragma once

//#define USE_MATRIX_I2C

// #define BOOTMAGIC_ROW 0
// #define BOOTMAGIC_COLUMN 0

#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1

#ifdef OLED_ENABLE
#   define OLED_DISPLAY_128X64
#   define I2C_DRIVER I2CD0
#   define OLED_TIMEOUT 60000
#   define OLED_BRIGHTNESS 128
#   define OLED_FONT_H "keyboards/diydipendu/corne66_2040/lib/glcdfont.c"
#endif
