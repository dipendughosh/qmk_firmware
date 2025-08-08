#include QMK_KEYBOARD_H
#include <stdio.h> // Include this for sprintf to work correctly
#include "oled_driver.h"

enum layers {
  _FUNCTION,
  _SPECIAL_FUNCTION,
  _NUMPAD,
  _MACROS
};

// Define custom keycodes for your macros. This is the modern, flexible way.
enum custom_keycodes {
    MAC_CPY = SAFE_RANGE,
    MAC_CUT,
    MAC_PST,
    MAC_UNDO,
    MAC_REDO,
    MAC_SAVE,
    MAC_SELALL,
    MAC_FIND,
    MAC_CLOSE,
    MAC_NEWTAB,
    MAC_NEWWIN,
    MAC_REL,
    MAC_TABS,
    MAC_WIN,
    MAC_TABSFT,
    MAC_TASKMG,
    MAC_TASKMG2
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
    [_MACROS] = LAYOUT_grid(
        MAC_CPY, MAC_CUT, MAC_PST, MAC_UNDO, KC_TRNS, KC_TRNS,
        MAC_REDO, MAC_SAVE, MAC_SELALL, MAC_FIND, KC_TRNS, KC_TRNS,
        MAC_CLOSE, MAC_NEWTAB, MAC_NEWWIN, MAC_REL, KC_TRNS, KC_TRNS,
        MAC_TABS, MAC_WIN, MAC_TABSFT, MAC_TASKMG, MAC_TASKMG2, TO(0))
};

#ifdef OLED_ENABLE

// A variable to store the name of the last key pressed
char last_key_pressed[20] = "";

// A timer to track when the keyboard is idle
static uint32_t oled_timer = 0;

// This function runs when a key is pressed or released
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // Reset the timer on every key press
        oled_timer = timer_read();

        // Check if the keycode is a standard key or a custom macro
        if (keycode >= SAFE_RANGE) {
            // For custom keycodes, display a special message
            switch (keycode) {
                case MAC_CPY:
                    sprintf(last_key_pressed, "Copy");
                    break;
                case MAC_CUT:
                    sprintf(last_key_pressed, "Cut");
                    break;
                case MAC_PST:
                    sprintf(last_key_pressed, "Paste");
                    break;
                case MAC_UNDO:
                    sprintf(last_key_pressed, "Undo");
                    break;
                case MAC_REDO:
                    sprintf(last_key_pressed, "Redo");
                    break;
                case MAC_SAVE:
                    sprintf(last_key_pressed, "Save");
                    break;
                case MAC_SELALL:
                    sprintf(last_key_pressed, "Select All");
                    break;
                case MAC_FIND:
                    sprintf(last_key_pressed, "Find");
                    break;
                case MAC_CLOSE:
                    sprintf(last_key_pressed, "Close");
                    break;
                case MAC_NEWTAB:
                    sprintf(last_key_pressed, "New Tab");
                    break;
                case MAC_NEWWIN:
                    sprintf(last_key_pressed, "New Window");
                    break;
                case MAC_REL:
                    sprintf(last_key_pressed, "Reload");
                    break;
                case MAC_TABS:
                    sprintf(last_key_pressed, "Cycle Tabs");
                    break;
                case MAC_WIN:
                    sprintf(last_key_pressed, "Cycle Windows");
                    break;
                case MAC_TABSFT:
                    sprintf(last_key_pressed, "Tab Shift");
                    break;
                case MAC_TASKMG:
                    sprintf(last_key_pressed, "Task Manager");
                    break;
                case MAC_TASKMG2:
                    sprintf(last_key_pressed, "Task Manager");
                    break;
                default:
                    sprintf(last_key_pressed, "Macro");
                    break;
            }
        } else {
            // For standard keys, display the keycode number (useful for debugging)
            sprintf(last_key_pressed, "0x%02X", keycode);
        }

        switch (keycode) {
            case MAC_CPY:
                tap_code16(LCTL(KC_C));
                return false;
            case MAC_CUT:
                tap_code16(LCTL(KC_X));
                return false;
            case MAC_PST:
                tap_code16(LCTL(KC_V));
                return false;
            case MAC_UNDO:
                tap_code16(LCTL(KC_Z));
                return false;
            case MAC_REDO:
                tap_code16(LCTL(KC_Y));
                return false;
            case MAC_SAVE:
                tap_code16(LCTL(KC_S));
                return false;
            case MAC_SELALL:
                tap_code16(LCTL(KC_A));
                return false;
            case MAC_FIND:
                tap_code16(LCTL(KC_F));
                return false;
            case MAC_CLOSE:
                tap_code16(LCTL(KC_W));
                return false;
            case MAC_NEWTAB:
                tap_code16(LCTL(KC_T));
                return false;
            case MAC_NEWWIN:
                tap_code16(LCTL(KC_N));
                return false;
            case MAC_REL:
                tap_code16(LCTL(KC_R));
                return false;
            case MAC_TABS:
                tap_code16(LCTL(KC_TAB));
                return false;
            case MAC_WIN:
                tap_code16(LALT(KC_TAB));
                return false;
            case MAC_TABSFT:
                tap_code16(LSFT(KC_TAB));
                return false;
            case MAC_TASKMG:
                tap_code16(LCTL(LALT(KC_DEL)));
                return false;
            case MAC_TASKMG2:
                tap_code16(LCTL(LSFT(KC_ESC)));
                return false;
        }
    }
    return true;
}

// This function renders the layer and WPM info
void render_main_info(void) {
    oled_set_cursor(0, 0);
    // oled_clear();

    // Display current layer
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state)) {
        case _FUNCTION:
            oled_write_P(PSTR("FUNCTION"), false);
            break;
        case _SPECIAL_FUNCTION:
            oled_write_P(PSTR("SPECIAL"), false);
            break;
        case _NUMPAD:
            oled_write_P(PSTR("NUMPAD"), false);
            break;
        case _MACROS:
            oled_write_P(PSTR("MACROS"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined!!!!"), false);
            break;
    }
    oled_write_P(PSTR("               "), false);

    // Display WPM
    oled_set_cursor(0, 2);
    char wpm_string[10];
    sprintf(wpm_string, "WPM: %u", get_current_wpm());
    oled_write(wpm_string, false);
}

// This function renders the last pressed key info
void render_key_info(void) {
    oled_set_cursor(0, 0);

    oled_write_P(PSTR("Pressed: "), false);
    oled_write(last_key_pressed, false);
    oled_write_P(PSTR("               "), false);
}

bool oled_task_user(void) {
    // The conditional logic to choose what to display
    if (timer_elapsed(oled_timer) > 1500) { // 1.5 second timeout
        // If idle, show layer and WPM
        render_main_info();
    } else {
        // If typing, show the last key pressed
        render_key_info();
    }

    return true;
}

#endif
