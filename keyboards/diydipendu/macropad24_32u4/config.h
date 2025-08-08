#pragma once

#define I2C1_SDA_PIN D1
#define I2C1_SCL_PIN D0

#ifdef OLED_ENABLE
#   define OLED_DISPLAY_128X64
#   define I2C_DRIVER I2CD0
#   define OLED_TIMEOUT 60000
#   define OLED_BRIGHTNESS 128
#   define OLED_FONT_H "keyboards/1upkeyboards/pi40/lib/glcdfont.c"
#endif
