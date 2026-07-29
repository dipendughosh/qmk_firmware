#pragma once

// #define BOOTMAGIC_ROW 0
// #define BOOTMAGIC_COLUMN 0

#define I2C1_SDA_PIN GP0
#define I2C1_SCL_PIN GP1

#ifdef OLED_ENABLE
#   define OLED_DISPLAY_128X64
#   define I2C_DRIVER I2CD0
#   define OLED_BRIGHTNESS 128
#   define OLED_FONT_H "keyboards/diydipendu/macropad24_2040/lib/glcdfont.c"
// The driver's own idle blanking is disabled so the keymap can run its
// screensaver before the display finally sleeps. Note that OLED_TIMEOUT 0 also
// stops QMK waking the panel on keypress, so the keymap calls oled_on()/off()
// itself -- see SCREENSAVER_START_MS in the keymap.
#   define OLED_TIMEOUT 0
#endif
