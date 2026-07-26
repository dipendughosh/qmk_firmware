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

// Holding the bottom-right key this long replays the boot sequence.
#define RESET_HOLD_MS 1500

// Matrix position of the key that triggers the soft reset.
#define RESET_KEY_ROW 3
#define RESET_KEY_COL 5

// Set when the reset key goes down, cleared on release.
static bool     reset_key_held  = false;
static uint32_t reset_key_timer = 0;

// Set once a hold has triggered the reset, so the key's own release action
// (TG/TO, which fire ON_RELEASE) can be swallowed and not re-toggle a layer.
static bool reset_fired = false;

// Reference point for the boot animation/splash sequence. Rewound on soft
// reset so the whole intro replays without power-cycling the board.
static uint32_t boot_timer = 0;

// Holding this key toggles the OLED on/off, so the user can keep the screen
// dark if they prefer. Unlike the reset key, this one has a real action on
// every layer (KC_ESC, MAC_TASKMG2, ...), so its press is held back and only
// replayed if the press turns out to be a short tap.
#define OLED_TOGGLE_HOLD_MS 1500
#define OLED_TOGGLE_KEY_ROW 3
#define OLED_TOGGLE_KEY_COL 4

static bool     oled_toggle_key_held  = false;
static uint32_t oled_toggle_key_timer = 0;
static bool     oled_toggle_fired     = false;

// User's on/off preference for the display.
static bool oled_user_enabled = true;

// Short-lived confirmation screens shown before a long-press action takes
// effect, so the user gets feedback that the hold registered.
typedef enum {
    MSG_NONE,
    MSG_SCREEN_OFF,
    MSG_SCREEN_ON,
} oled_message_t;

#define SCREEN_MSG_MS 500

static oled_message_t oled_message       = MSG_NONE;
static uint32_t       oled_message_timer = 0;

// Label drawn above the progress bar, wording depending on whether the boot
// sequence came from a power-on or a soft reset.
#define BOOT_MSG_TEXT "Starting"
#define RESET_MSG_TEXT "Reseting"
#define BOOT_MSG_DOTS 5

static bool boot_from_reset = false;

// Turn on raw matrix-scan printing over `qmk console` so ghosting/wiring
// issues can be diagnosed without a multimeter.
void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = true;

    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_sethsv_noeeprom(0, 255, 255);
}

// Step the onboard RGB LED to the next hue once a second, and watch for a
// long press on the reset key.
void housekeeping_task_user(void) {
    static uint32_t rgb_cycle_timer = 0;
    static uint8_t  rgb_hue         = 0;

    if (timer_elapsed32(rgb_cycle_timer) >= 1000) {
        rgb_cycle_timer = timer_read32();
        rgb_hue += 32;
        rgblight_sethsv_noeeprom(rgb_hue, 255, 255);
    }

    // Fire while the key is still down so the reset feels immediate rather
    // than waiting for release. The "Reseting" label rides along with the boot
    // animation rather than getting its own screen first.
    if (reset_key_held && timer_elapsed32(reset_key_timer) >= RESET_HOLD_MS) {
        reset_key_held = false;
        reset_fired    = true;

        boot_from_reset = true;
        layer_clear();
        boot_timer = timer_read32();
    }

    if (oled_toggle_key_held && timer_elapsed32(oled_toggle_key_timer) >= OLED_TOGGLE_HOLD_MS) {
        oled_toggle_key_held = false;
        oled_toggle_fired    = true;

        if (oled_user_enabled) {
            // Announce first, then go dark once the message has been seen.
            oled_message = MSG_SCREEN_OFF;
        } else {
            // Come back on right away so the message is actually visible.
            oled_user_enabled = true;
            oled_message      = MSG_SCREEN_ON;
        }
        oled_message_timer = timer_read32();
    }

    // Apply whatever the expiring message was announcing.
    switch (oled_message) {
        case MSG_SCREEN_OFF:
            if (timer_elapsed32(oled_message_timer) >= SCREEN_MSG_MS) {
                oled_message      = MSG_NONE;
                oled_user_enabled = false;
            }
            break;
        case MSG_SCREEN_ON:
            if (timer_elapsed32(oled_message_timer) >= SCREEN_MSG_MS) {
                oled_message = MSG_NONE;
            }
            break;
        case MSG_NONE:
            break;
    }
}

#ifdef OLED_ENABLE

// A variable to store the name of the last key pressed
char last_key_pressed[20] = "";

// The raw keycode of the last key pressed, formatted as hex
char last_key_code[8] = "";

// How many keys are physically down right now. While this is non-zero the key
// info stays on screen, so holding a key doesn't flip to the layer name (and
// then flash the *next* layer) partway through a long press.
static uint8_t keys_held = 0;

// How long to play the connect animation before the splash text appears.
#define ANIM_DURATION_MS 1500

// How long to show the boot splash before switching to the normal display.
#define SPLASH_DURATION_MS 2000

#define SPLASH_LINE1 "MacroPad"
#define SPLASH_LINE2 "DiGhosh"

// The glyph table backing the OLED_FONT_H set for this board (see lib/glcdfont.c).
// It's declared non-static there so we can blit it manually at larger sizes below.
extern const unsigned char font[];

// Draws one character scaled up by an integer factor, top-left pixel at (x0, y0).
static void oled_write_char_scaled(uint8_t x0, uint8_t y0, char c, uint8_t scale) {
    uint8_t cast_data = (uint8_t)c;
    if (cast_data < OLED_FONT_START || cast_data > OLED_FONT_END) {
        return;
    }

    const uint8_t *glyph = &font[(cast_data - OLED_FONT_START) * OLED_FONT_WIDTH];
    for (uint8_t col = 0; col < OLED_FONT_WIDTH; col++) {
        uint8_t bits = pgm_read_byte(&glyph[col]);
        for (uint8_t row = 0; row < 8; row++) {
            bool on = bits & (1 << row);
            for (uint8_t sx = 0; sx < scale; sx++) {
                for (uint8_t sy = 0; sy < scale; sy++) {
                    oled_write_pixel(x0 + col * scale + sx, y0 + row * scale + sy, on);
                }
            }
        }
    }
}

// Draws a string scaled up by an integer factor, top-left pixel at (x0, y0).
static void oled_write_string_scaled_at(const char *str, uint8_t x0, uint8_t y0, uint8_t scale) {
    uint8_t len = strlen(str);

    for (uint8_t i = 0; i < len; i++) {
        oled_write_char_scaled(x0 + i * OLED_FONT_WIDTH * scale, y0, str[i], scale);
    }
}

// The left edge a string of char_len characters needs to sit centered.
static uint8_t oled_centered_x(uint8_t char_len, uint8_t scale) {
    uint16_t text_width = (uint16_t)char_len * OLED_FONT_WIDTH * scale;
    return (text_width < OLED_DISPLAY_WIDTH) ? (OLED_DISPLAY_WIDTH - text_width) / 2 : 0;
}

// Draws a string scaled up by an integer factor, horizontally centered, starting at pixel row y0.
static void oled_write_string_scaled_centered(const char *str, uint8_t y0, uint8_t scale) {
    oled_write_string_scaled_at(str, oled_centered_x(strlen(str), scale), y0, scale);
}

// Draws a rectangle. When filled is false, only the 1px border is drawn.
static void oled_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool filled) {
    for (uint8_t i = 0; i < w; i++) {
        for (uint8_t j = 0; j < h; j++) {
            bool is_border = (i == 0 || i == w - 1 || j == 0 || j == h - 1);
            if (filled || is_border) {
                oled_write_pixel(x + i, y + j, true);
            }
        }
    }
}

// Connect animation: a "Starting..." / "Reseting..." label above a bordered
// loading bar that fills up over ANIM_DURATION_MS, with the dots keeping pace
// with the fill.
void render_boot_animation(uint32_t elapsed) {
    const uint8_t bar_width  = 100;
    const uint8_t bar_height = 12;
    uint8_t       bar_x      = (OLED_DISPLAY_WIDTH - bar_width) / 2;

    const char *label = boot_from_reset ? RESET_MSG_TEXT : BOOT_MSG_TEXT;

    const uint8_t gap     = 6;
    uint8_t       max_len = strlen(label) + BOOT_MSG_DOTS;
    uint8_t       scale   = ((uint16_t)max_len * OLED_FONT_WIDTH * 2 <= OLED_DISPLAY_WIDTH) ? 2 : 1;
    uint8_t       label_h = OLED_FONT_HEIGHT * scale;
    uint8_t       block_h = label_h + gap + bar_height;
    uint8_t       label_y = (OLED_DISPLAY_HEIGHT > block_h) ? (OLED_DISPLAY_HEIGHT - block_h) / 2 : 0;

    // One dot per slice of the animation, so text and bar finish together.
    uint8_t dots = elapsed / (ANIM_DURATION_MS / (BOOT_MSG_DOTS + 1));
    if (dots > BOOT_MSG_DOTS) {
        dots = BOOT_MSG_DOTS;
    }

    char    buf[24];
    uint8_t len = 0;
    while (label[len] != '\0' && len < sizeof(buf) - 1) {
        buf[len] = label[len];
        len++;
    }
    for (uint8_t i = 0; i < dots && len < sizeof(buf) - 1; i++) {
        buf[len++] = '.';
    }
    buf[len] = '\0';

    // Anchor to the full-length string so the text doesn't creep sideways
    // as dots appear (and doesn't leave stale pixels behind).
    oled_write_string_scaled_at(buf, oled_centered_x(max_len, scale), label_y, scale);

    uint8_t bar_y = label_y + label_h + gap;

    oled_draw_rect(bar_x, bar_y, bar_width, bar_height, false);

    const uint8_t inset  = 2;
    uint8_t       inner_w = bar_width - (inset * 2);
    uint8_t       inner_h = bar_height - (inset * 2);
    uint16_t      fill_w  = (uint16_t)inner_w * elapsed / ANIM_DURATION_MS;
    if (fill_w > inner_w) {
        fill_w = inner_w;
    }

    oled_draw_rect(bar_x + inset, bar_y + inset, (uint8_t)fill_w, inner_h, true);
}

// Centered splash shown for the first SPLASH_DURATION_MS after power-on:
// "MacroPad" (size 2), a blank line, then "Dipendu Ghosh" (size 1).
void render_splash(void) {
    const uint8_t large_scale  = 2;
    const uint8_t small_scale  = 1;
    uint8_t       large_height = OLED_FONT_HEIGHT * large_scale;
    uint8_t       small_height = OLED_FONT_HEIGHT * small_scale;
    uint8_t       gap_height   = small_height; // the blank line
    // uint8_t       total_height = large_height + gap_height + small_height;
    uint8_t       total_height = large_height + gap_height + large_height;
    uint8_t       start_y      = (OLED_DISPLAY_HEIGHT > total_height) ? (OLED_DISPLAY_HEIGHT - total_height) / 2 : 0;

    oled_write_string_scaled_centered(SPLASH_LINE1, start_y, large_scale);
    // oled_write_string_scaled_centered(SPLASH_LINE2, start_y + large_height + gap_height, small_scale);
    oled_write_string_scaled_centered(SPLASH_LINE2, start_y + large_height + gap_height, large_scale);
}

// Draws the current long-press confirmation message, centered on one line.
void render_message(void) {
    const char *text;

    switch (oled_message) {
        case MSG_SCREEN_OFF:
            text = "Screen Off";
            break;
        case MSG_SCREEN_ON:
            text = "Screen On";
            break;
        default:
            return;
    }

    // Shrink to 1x only if the message won't fit at 2x.
    uint8_t len   = strlen(text);
    uint8_t scale = ((uint16_t)len * OLED_FONT_WIDTH * 2 <= OLED_DISPLAY_WIDTH) ? 2 : 1;

    uint8_t y0 = (OLED_DISPLAY_HEIGHT > OLED_FONT_HEIGHT * scale) ? (OLED_DISPLAY_HEIGHT - OLED_FONT_HEIGHT * scale) / 2 : 0;
    oled_write_string_scaled_centered(text, y0, scale);
}

// Records the name/code of the key just pressed, for the OLED to show.
static void set_last_key_display(uint16_t keycode) {
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
        // For standard keys, display the human-readable keycode name
        snprintf(last_key_pressed, sizeof(last_key_pressed), "%s", get_keycode_string(keycode));
    }

    // Always keep the raw keycode value around to show under the name
    snprintf(last_key_code, sizeof(last_key_code), "0x%04X", keycode);
}

// Sends the keystrokes for one of our custom macros. Returns false if the
// keycode isn't a macro, so the caller can fall back to normal handling.
static bool send_macro_action(uint16_t keycode) {
    switch (keycode) {
        case MAC_CPY:
            tap_code16(LCTL(KC_C));
            return true;
        case MAC_CUT:
            tap_code16(LCTL(KC_X));
            return true;
        case MAC_PST:
            tap_code16(LCTL(KC_V));
            return true;
        case MAC_UNDO:
            tap_code16(LCTL(KC_Z));
            return true;
        case MAC_REDO:
            tap_code16(LCTL(KC_Y));
            return true;
        case MAC_SAVE:
            tap_code16(LCTL(KC_S));
            return true;
        case MAC_SELALL:
            tap_code16(LCTL(KC_A));
            return true;
        case MAC_FIND:
            tap_code16(LCTL(KC_F));
            return true;
        case MAC_CLOSE:
            tap_code16(LCTL(KC_W));
            return true;
        case MAC_NEWTAB:
            tap_code16(LCTL(KC_T));
            return true;
        case MAC_NEWWIN:
            tap_code16(LCTL(KC_N));
            return true;
        case MAC_REL:
            tap_code16(LCTL(KC_R));
            return true;
        case MAC_TABS:
            tap_code16(LCTL(KC_TAB));
            return true;
        case MAC_WIN:
            tap_code16(LALT(KC_TAB));
            return true;
        case MAC_TABSFT:
            tap_code16(LSFT(KC_TAB));
            return true;
        case MAC_TASKMG:
            tap_code16(LCTL(LALT(KC_DEL)));
            return true;
        case MAC_TASKMG2:
            tap_code16(LCTL(LSFT(KC_ESC)));
            return true;
    }
    return false;
}

// This function runs when a key is pressed or released
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Keep the held-key count up to date before any early return below. The
    // OLED shows key info for exactly as long as this is non-zero.
    if (record->event.pressed) {
        keys_held++;
    } else if (keys_held > 0) {
        keys_held--;
    }

    // The OLED toggle key: show its name on press like any other key, but hold
    // the action back so a long hold can toggle the screen without also firing
    // it (on some layers that action is Task Manager). A short tap runs the
    // action on release; a long hold turns the display off instead.
    if (record->event.key.row == OLED_TOGGLE_KEY_ROW && record->event.key.col == OLED_TOGGLE_KEY_COL) {
        if (record->event.pressed) {
            oled_toggle_key_held  = true;
            oled_toggle_key_timer = timer_read32();
            set_last_key_display(keycode);
        } else {
            oled_toggle_key_held = false;

            if (oled_toggle_fired) {
                // The hold already toggled the display; swallow the tap action.
                oled_toggle_fired = false;
            } else if (!send_macro_action(keycode)) {
                tap_code16(keycode);
            }
        }
        return false;
    }

    // Track holds on the reset key. A short tap keeps its normal layer action;
    // once a hold has fired the reset we swallow the release so TG()/TO()
    // (which act ON_RELEASE) don't drag us straight back off layer 0.
    if (record->event.key.row == RESET_KEY_ROW && record->event.key.col == RESET_KEY_COL) {
        if (record->event.pressed) {
            reset_key_held  = true;
            reset_key_timer = timer_read32();
        } else {
            reset_key_held = false;
            if (reset_fired) {
                reset_fired = false;
                return false;
            }
        }
    }

    if (record->event.pressed) {
        set_last_key_display(keycode);

        if (send_macro_action(keycode)) {
            return false;
        }
    }
    return true;
}

// This function renders the current layer name, large and centered at the top of the screen
void render_main_info(void) {
    // Only clear when the layer actually changes. OLED_UPDATE_PROCESS_LIMIT
    // defaults to 1 block per frame, so clearing every frame (marking the
    // whole buffer dirty each time) starves the flush and only the first
    // block ever reaches the screen.
    static int8_t last_rendered_layer = -1;
    int8_t        current_layer       = get_highest_layer(layer_state);
    if (current_layer != last_rendered_layer) {
        last_rendered_layer = current_layer;
        oled_clear();
    }

    const char *layer_name;
    switch (current_layer) {
        case _FUNCTION:
            layer_name = "FUNCTION";
            break;
        case _SPECIAL_FUNCTION:
            layer_name = "SPECIAL";
            break;
        case _NUMPAD:
            layer_name = "NUMPAD";
            break;
        case _MACROS:
            layer_name = "MACROS";
            break;
        default:
            layer_name = "UNDEFINED";
            break;
    }

    oled_write_string_scaled_centered(layer_name, 0, 2);

    // Display WPM to enable uncomment he below code and in rules.mk
    // oled_set_cursor(0, 2);
    // char wpm_string[10];
    // sprintf(wpm_string, "WPM: %u", get_current_wpm());
    // oled_write(wpm_string, false);
}

// This function renders the last pressed key: its name (large) above its raw
// code (normal size), both centered on the screen as a block.
void render_key_info(void) {
    // Only clear when the key actually changes (see render_main_info for why).
    static char last_rendered_key[sizeof(last_key_pressed)] = "";
    if (strcmp(last_key_pressed, last_rendered_key) != 0) {
        strcpy(last_rendered_key, last_key_pressed);
        oled_clear();
    }

    // Drop to 1x if the name is too long to fit the screen at 2x width.
    uint8_t name_scale = 2;
    if (strlen(last_key_pressed) * OLED_FONT_WIDTH * name_scale > OLED_DISPLAY_WIDTH) {
        name_scale = 1;
    }
    const uint8_t name_rows  = name_scale; // scaled name height, in OLED_FONT_HEIGHT-tall row units
    const uint8_t code_rows  = 1;
    const uint8_t total_rows = name_rows + code_rows;

    uint8_t max_chars = oled_max_chars();
    uint8_t max_lines = oled_max_lines();
    uint8_t start_row = (max_lines > total_rows) ? (max_lines - total_rows) / 2 : 0;

    // Line 1: key name, larger font, centered
    oled_write_string_scaled_centered(last_key_pressed, start_row * OLED_FONT_HEIGHT, name_scale);

    // Line 2: raw keycode, normal (smaller) font, centered
    uint8_t code_len = strlen(last_key_code);
    oled_set_cursor((max_chars - code_len) / 2, start_row + name_rows);
    oled_write(last_key_code, false);
}

// Which of the three screens is currently being shown. Used so we can clear
// once whenever we *switch* screens, even if the content on the new screen
// happens to be identical to what it showed last time it was up (e.g.
// returning to the same layer name after a keypress) -- render_main_info()
// and render_key_info() only clear on their own when their own content
// changes, which isn't enough on its own to wipe leftovers from a *different*
// screen that was showing a moment ago.
typedef enum {
    OLED_SCREEN_NONE,
    OLED_SCREEN_MSG,
    OLED_SCREEN_ANIM,
    OLED_SCREEN_SPLASH,
    OLED_SCREEN_LAYER,
    OLED_SCREEN_KEY,
} oled_screen_t;

bool oled_task_user(void) {
    static oled_screen_t last_screen = OLED_SCREEN_NONE;

    // User has held the toggle key to turn the screen off. QMK re-wakes the
    // OLED on any key activity, so keep asserting the off state here rather
    // than switching it off once.
    static bool was_user_enabled = true;

    if (!oled_user_enabled) {
        oled_off();
        was_user_enabled = false;
        // Force a full repaint whenever the screen comes back.
        last_screen = OLED_SCREEN_NONE;
        return false;
    }

    if (!was_user_enabled) {
        // Just switched back on. Wake it once here; after this, leave the
        // display alone so the normal OLED_TIMEOUT sleep still works.
        was_user_enabled = true;
        oled_on();
    }

    // A long-press confirmation outranks everything else, including the boot
    // sequence it may be about to kick off.
    if (oled_message != MSG_NONE) {
        if (last_screen != OLED_SCREEN_MSG) {
            last_screen = OLED_SCREEN_MSG;
            oled_clear();
        }
        render_message();
        return true;
    }

    // The conditional logic to choose what to display.
    // Use the 32-bit timer here: timer_read() is 16-bit and wraps every ~65.5s,
    // which would make the boot sequence reappear every time it wrapped.
    uint32_t boot_elapsed = timer_elapsed32(boot_timer);

    if (boot_elapsed < ANIM_DURATION_MS) {
        // Still within the connect animation window
        if (last_screen != OLED_SCREEN_ANIM) {
            last_screen = OLED_SCREEN_ANIM;
            oled_clear();
        }
        render_boot_animation(boot_elapsed);
    } else if (boot_elapsed < ANIM_DURATION_MS + SPLASH_DURATION_MS) {
        // Animation finished, now within the boot splash window
        if (last_screen != OLED_SCREEN_SPLASH) {
            last_screen = OLED_SCREEN_SPLASH;
            oled_clear();
        }
        render_splash();
    } else if (keys_held == 0) {
        // Nothing held: show the layer name
        if (last_screen != OLED_SCREEN_LAYER) {
            last_screen = OLED_SCREEN_LAYER;
            oled_clear();
        }
        render_main_info();
    } else {
        // A key is down: show it for as long as it's held
        if (last_screen != OLED_SCREEN_KEY) {
            last_screen = OLED_SCREEN_KEY;
            oled_clear();
        }
        render_key_info();
    }

    return true;
}

#endif
