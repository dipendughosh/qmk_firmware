#include QMK_KEYBOARD_H

enum layer_names {
    _MAIN,
    _FUNCTION,
    _SYMBOL,
};

#define FUNCTION MO(_FUNCTION)
#define SYMBOL MO(_SYMBOL)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

  [_MAIN] = LAYOUT(
  //┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐                 ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐
     KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,                      KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,                      KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,                      KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ESC,  KC_ENT,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                                        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
  //└────────┴────────┼────────┼────────┼────────┼────────┼────────┐                 ┌────────┼────────┼────────┼────────┴────────┴────────┴────────┘
              KC_LCTL, KC_LGUI, KC_LALT, KC_APP,  MO(1),   KC_SPC,                    KC_SPC,  TO(1),   KC_APP,  KC_RALT, KC_RGUI, KC_RCTL
  //         └────────┴────────┴────────┴────────┴────────┴────────┘                 └────────┴────────┴────────┴────────┴────────┴────────┘
  ),

  [_FUNCTION] = LAYOUT(
  //┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐                 ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐
     KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_BSPC,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,                      KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,                      KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ESC,  KC_ENT,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,                                        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
  //└────────┴────────┼────────┼────────┼────────┼────────┼────────┐                 ┌────────┼────────┼────────┼────────┴────────┴────────┴────────┘
              KC_LCTL, KC_LGUI, KC_LALT, KC_APP,  MO(0),   KC_SPC,                    KC_SPC,  TO(2),   KC_APP,  KC_RALT, KC_RGUI, KC_RCTL
  //         └────────┴────────┴────────┴────────┴────────┴────────┘                 └────────┴────────┴────────┴────────┴────────┴────────┘
  ),

  [_SYMBOL] = LAYOUT(
  //┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐                 ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐
     KC_ESC,  KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC, KC_PSCR,                   KC_P1,   KC_P2,   KC_P3,   KC_PPLS, KC_LPRN, KC_RPRN, KC_BSPC,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_TAB,  KC_CIRC, KC_AMPR, KC_UNDS, KC_INS,  KC_HOME, KC_SCRL,                   KC_P4,   KC_P5,   KC_P6,   KC_PMNS, KC_LCBR, KC_RCBR, KC_PDOT,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_BSLS, KC_PIPE, KC_UP,   KC_SCLN, KC_HOME, KC_END,  KC_PAUS,                   KC_P7,   KC_P8,   KC_P9,   KC_PAST, KC_LBRC, KC_RBRC, KC_ENT,
  //├────────┼────────┼────────┼────────┼────────┼────────┼────────┤                 ├────────┼────────┼────────┼────────┼────────┼────────┼────────┤
     KC_LSFT, KC_LEFT, KC_DOWN, KC_RGHT, KC_PGUP, KC_PGDN,                                     KC_P0,   KC_EQL,  KC_PSLS, KC_LT,   KC_GT,   KC_PCMM,
  //└────────┴────────┼────────┼────────┼────────┼────────┼────────┐                 ┌────────┼────────┼────────┼────────┴────────┴────────┴────────┘
              KC_LCTL, KC_LGUI, KC_LALT, KC_APP,  MO(1),   KC_SPC,                    KC_SPC,  TO(0),   KC_COLN, KC_QUOT, KC_DQUO, KC_QUES
  //         └────────┴────────┴────────┴────────┴────────┴────────┘                 └────────┴────────┴────────┴────────┴────────┴────────┘
  ),
};

#ifdef OLED_ENABLE

// oled_rotation_t oled_init_user(oled_rotation_t rotation)
// {
//     if (!is_keyboard_master()) {
//         return OLED_ROTATION_180;  // flips the display 180 degrees if offhand
//     }

//     return rotation;
// }

void render_status(void)
{
    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);

    switch (get_highest_layer(layer_state))
    {
        case _MAIN:
            oled_write_P(PSTR("Main\n"), false);
            break;
        case _FUNCTION:
            oled_write_P(PSTR("Function\n"), false);
            break;
        case _SYMBOL:
            oled_write_P(PSTR("Symbol\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("Undefined"), false);
    }

    // Host Keyboard LED Status
    led_t led_state = host_keyboard_led_state();
    oled_write_P(PSTR("Num Lock: "), false);
    oled_write_ln_P(led_state.num_lock ? PSTR("ON ") : PSTR("OFF"), false);
    oled_write_P(PSTR("Caps Lock: "), false);
    oled_write_ln_P(led_state.caps_lock ? PSTR("ON ") : PSTR("OFF"), false);
    oled_write_P(PSTR("Scroll Lock: "), false);
    oled_write_ln_P(led_state.scroll_lock ? PSTR("ON ") : PSTR("OFF"), false);

    oled_write_P(PSTR("Shift: "), false);
    oled_write_ln_P(get_mods() & MOD_MASK_SHIFT ? PSTR("ON ") : PSTR("OFF "), false);
    oled_write_P(PSTR("Ctrl: "), false);
    oled_write_ln_P(get_mods() & MOD_MASK_CTRL ? PSTR("ON ") : PSTR("OFF "), false);
    oled_write_P(PSTR("Alt: "), false);
    oled_write_ln_P(get_mods() & MOD_MASK_ALT ? PSTR("ON ") : PSTR("OFF "), false);
}

void render_logo(void)
{
    static const char PROGMEM qmk_logo[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x07, 0xE7, 0x87, 0x3F, 0xFF,
    0x7F, 0x0F, 0xE7, 0x07, 0xFF, 0xFF,
    0xFF, 0x5F, 0x5F, 0x5F, 0x1F, 0xFF,
    0xFF, 0x3F, 0x9F, 0xDF, 0x9F, 0x03,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07,
    0x07, 0xC7, 0x1F, 0x7F, 0xFF, 0x1F,
    0xC7, 0x07, 0x07, 0xFF, 0xFF, 0x5F,
    0x5F, 0x5F, 0x1F, 0x3F, 0xFF, 0x1F,
    0x1F, 0xDF, 0xDF, 0x1F, 0x3F, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8,
    0xFF, 0xFF, 0xFC, 0xF8, 0xFC, 0xFF,
    0xFF, 0xF8, 0xFF, 0xFF, 0xFC, 0xF8,
    0xFB, 0xF9, 0xF8, 0xFF, 0xFF, 0xFC,
    0xF9, 0xFB, 0xFD, 0xF8, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xF8, 0xF8, 0xFF,
    0xFE, 0xF8, 0xF8, 0xFE, 0xFF, 0xF8,
    0xF8, 0xFF, 0xFF, 0xF8, 0xFB, 0xFB,
    0xF8, 0xF8, 0xFF, 0xF8, 0xF8, 0xFF,
    0xFF, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0xEF,
    0xEF, 0xCF, 0x1F, 0x7F, 0xFF, 0x27,
    0x27, 0xFF, 0x3F, 0x3F, 0xBF, 0xBF,
    0x3F, 0x7F, 0xFF, 0xFF, 0x7F, 0xBF,
    0xBF, 0x3F, 0x7F, 0xFF, 0x3F, 0x3F,
    0xBF, 0xBF, 0x3F, 0x7F, 0xFF, 0xFF,
    0x3F, 0xBF, 0xBF, 0x07, 0x07, 0xFF,
    0xFF, 0x3F, 0xFF, 0xFF, 0x3F, 0x3F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F,
    0xDF, 0xEF, 0x6F, 0x6F, 0x6F, 0xFF,
    0xFF, 0x07, 0x07, 0xBF, 0xBF, 0x3F,
    0x7F, 0xFF, 0x7F, 0x3F, 0xBF, 0xBF,
    0x3F, 0x7F, 0xFF, 0xFF, 0x3F, 0xBF,
    0xBF, 0xFF, 0xFF, 0x07, 0x07, 0xBF,
    0xBF, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xF0, 0xF0, 0xF7, 0xF7, 0xF3,
    0xF8, 0xFE, 0xFF, 0xF0, 0xF0, 0xFF,
    0xC0, 0xC0, 0xF7, 0xF7, 0xF0, 0xF8,
    0xFF, 0xFC, 0xF0, 0xF6, 0xF6, 0xF6,
    0xF6, 0xFF, 0xF0, 0xF0, 0xFF, 0xFF,
    0xF0, 0xF0, 0xFF, 0xFC, 0xF0, 0xF7,
    0xF7, 0xF8, 0xF0, 0xFF, 0xFF, 0xF0,
    0xF7, 0xF7, 0xF0, 0xF0, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xF8, 0xF3, 0xF7,
    0xF7, 0xF7, 0xF0, 0xFF, 0xFF, 0xF0,
    0xF0, 0xFF, 0xFF, 0xF0, 0xF0, 0xFF,
    0xF8, 0xF0, 0xF7, 0xF7, 0xF3, 0xF8,
    0xFF, 0xFB, 0xF6, 0xF4, 0xF1, 0xFB,
    0xFF, 0xF0, 0xF0, 0xFF, 0xFF, 0xF0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF
    };

    oled_write_P(qmk_logo, false);
}

bool oled_task_user(void)
{

    if (is_keyboard_master())
    {
        render_status();  // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    }
    else
    {
        render_logo();  // Renders a static logo
    }
    return false;
}
#endif
