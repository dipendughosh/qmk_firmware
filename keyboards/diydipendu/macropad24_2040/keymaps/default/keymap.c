// Minimal baseline keymap for the MacroPad24.
//
// Layers only -- the shortcut keys use QMK's built-in modifier wrapping
// (e.g. LCTL(KC_C) for Ctrl+C), so no custom keycodes or process_record_user
// are needed here.
//
// See the MacroPad_v1_24_2040 keymap for the full featured version with the
// OLED boot animation, per-key display and long-press actions.

#include QMK_KEYBOARD_H

enum layers {
    _FUNCTION,
    _SPECIAL_FUNCTION,
    _NUMPAD,
    _MACROS
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_FUNCTION] = LAYOUT_grid(
        KC_F1, KC_F2, KC_F3, KC_F4, KC_INS, KC_DEL,
        KC_F5, KC_F6, KC_F7, KC_F8, KC_HOME, KC_END,
        KC_F9, KC_F10, KC_F11, KC_F12, KC_PGUP, KC_PGDN,
        KC_LEFT, KC_RGHT, KC_UP, KC_DOWN, KC_ESC, TG(1)
    ),
    [_SPECIAL_FUNCTION] = LAYOUT_grid(
        KC_F13, KC_F14, KC_F15, KC_F16, KC_TRNS, KC_TRNS,
        KC_F17, KC_F18, KC_F19, KC_F20, KC_TRNS, KC_TRNS,
        KC_F21, KC_F22, KC_F23, KC_F24, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, TG(2)
    ),
    [_NUMPAD] = LAYOUT_grid(
        KC_P1, KC_P2, KC_P3, KC_PPLS, KC_TRNS, KC_TRNS,
        KC_P4, KC_P5, KC_P6, KC_PMNS, KC_TRNS, KC_TRNS,
        KC_P7, KC_P8, KC_P9, KC_PAST, KC_TRNS, KC_TRNS,
        KC_P0, KC_PDOT, KC_PSLS, KC_NUM, KC_TRNS, TG(3)
    ),
    //        Copy          Cut           Paste         Undo
    //        Redo          Save          Select All    Find
    //        Close         New Tab       New Window    Reload
    //        Cycle Tabs    Cycle Windows Tab Shift     Task Mgr      Task Mgr 2
    [_MACROS] = LAYOUT_grid(
        LCTL(KC_C),   LCTL(KC_X),   LCTL(KC_V),   LCTL(KC_Z),   KC_TRNS,              KC_TRNS,
        LCTL(KC_Y),   LCTL(KC_S),   LCTL(KC_A),   LCTL(KC_F),   KC_TRNS,              KC_TRNS,
        LCTL(KC_W),   LCTL(KC_T),   LCTL(KC_N),   LCTL(KC_R),   KC_TRNS,              KC_TRNS,
        LCTL(KC_TAB), LALT(KC_TAB), LSFT(KC_TAB), LCTL(LALT(KC_DEL)), LCTL(LSFT(KC_ESC)), TO(0)
    )
};
