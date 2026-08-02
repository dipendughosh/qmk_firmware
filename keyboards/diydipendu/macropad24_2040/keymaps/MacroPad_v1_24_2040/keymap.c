// Keymap for the DiGhosh MacroPad24 (RP2040).
//
// File layout:
//   1. Enums          - layers, custom keycodes, internal state types
//   2. Keymap         - the layer tables
//   3. Configuration  - all tunable constants (timings, key positions, text)
//   4. State          - module-level variables
//   5. Key handling   - key name lookup and macro dispatch
//   6. QMK callbacks  - the entry points QMK calls into
//   7. OLED drawing   - low-level pixel/text primitives
//   8. OLED screens   - one render_* function per screen, plus oled_task_user
//
// Everything OLED-specific lives in one #ifdef OLED_ENABLE block at the end.

#include QMK_KEYBOARD_H
#include <stdio.h> // Include this for sprintf to work correctly
#include "oled_driver.h"

/* -------------------------------------------------------------------------
 * 1. Enums
 * ---------------------------------------------------------------------- */

enum layers {
    _FUNCTION,
    _SPECIAL_FUNCTION,
    _NUMPAD,
    _MACROS
};

// Define custom keycodes for your macros. This is the modern, flexible way.
// Listed in the same order they appear on the _MACROS layer.
enum custom_keycodes {
    // Row 1 - clipboard
    MAC_SELALL = SAFE_RANGE,
    MAC_CPY,
    MAC_CUT,
    MAC_PST,
    // Row 2 - editing
    MAC_FIND,
    MAC_REDO,
    MAC_UNDO,
    MAC_SAVE,
    // Row 3 - windows
    MAC_NEWWIN,
    MAC_WIN,
    MAC_CLOSE,
    MAC_REL,
    // Row 4 - tabs and system
    MAC_NEWTAB,
    MAC_TABS,
    MAC_PRVTAB,
    MAC_SECURE,
    MAC_TASKMG
};

// Short-lived confirmation screens shown before a long-press action takes
// effect, so the user gets feedback that the hold registered.
typedef enum {
    MSG_NONE,
    MSG_SCREEN_OFF,
    MSG_SCREEN_ON,
} oled_message_t;

// Which screen is currently being shown. Used so we can clear once whenever we
// *switch* screens, even if the content on the new screen happens to be
// identical to what it showed last time it was up (e.g. returning to the same
// layer name after a keypress) -- the render_* functions only clear on their
// own when their own content changes, which isn't enough on its own to wipe
// leftovers from a *different* screen that was showing a moment ago.
typedef enum {
    OLED_SCREEN_NONE,
    OLED_SCREEN_MSG,
    OLED_SCREEN_ANIM,
    OLED_SCREEN_SPLASH,
    OLED_SCREEN_LAYER,
    OLED_SCREEN_KEY,
    OLED_SCREEN_RAIN,
} oled_screen_t;

/* -------------------------------------------------------------------------
 * 2. Keymap
 * ---------------------------------------------------------------------- */

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
        MAC_SELALL, MAC_CPY,  MAC_CUT,    MAC_PST,    KC_TRNS,    KC_TRNS,
        MAC_FIND,   MAC_REDO, MAC_UNDO,   MAC_SAVE,   KC_TRNS,    KC_TRNS,
        MAC_NEWWIN, MAC_WIN,  MAC_CLOSE,  MAC_REL,    KC_TRNS,    KC_TRNS,
        MAC_NEWTAB, MAC_TABS, MAC_PRVTAB, MAC_SECURE, MAC_TASKMG, TO(0))
};

/* -------------------------------------------------------------------------
 * 3. Configuration
 * ---------------------------------------------------------------------- */

/* --- Long-press keys ------------------------------------------------------
 * Two keys gain a second function when held. Both are tracked by matrix
 * position rather than keycode, so they behave the same on every layer.
 */

// Bottom-right key: hold to replay the boot sequence (a soft reset).
#define RESET_KEY_ROW 3
#define RESET_KEY_COL 5
#define RESET_HOLD_MS 1500

// Key to its left: hold to turn the OLED off/on.
#define OLED_TOGGLE_KEY_ROW 3
#define OLED_TOGGLE_KEY_COL 4
#define OLED_TOGGLE_HOLD_MS 1500

/* --- Boot sequence -------------------------------------------------------- */

// Loading bar, then the splash text, then normal operation.
#define ANIM_DURATION_MS 1500
#define SPLASH_DURATION_MS 2000

// Label above the loading bar, wording depending on whether the boot sequence
// came from a power-on or a soft reset. Kept uppercase on purpose: the 6x8
// font has no room for descenders, so a lowercase "g" renders with its tail
// clipped (see lib/glcdfont.c -- only "y" uses the bottom pixel row).
#define BOOT_MSG_TEXT "STARTING"
#define RESET_MSG_TEXT "RESETTING"
#define BOOT_MSG_DOTS 5

// The label types itself out one letter at a time at this rate, then the dots
// are paced across whatever is left of ANIM_DURATION_MS. Keep this brisk: the
// whole screen only lasts 1.5s, so a slow reveal would leave the finished word
// on screen for barely a moment. Set to 0 to show the label immediately.
#define BOOT_LETTER_MS 50

// Splash text. Both lines are drawn at SPLASH_SCALE with a blank line between.
#define SPLASH_LINE1 "MacroPad"
#define SPLASH_LINE2 "DiGhosh"
#define SPLASH_SCALE 2

/* --- Long-press confirmations --------------------------------------------- */

// How long "Screen Off"/"Screen On" stays up before the toggle takes effect.
#define SCREEN_MSG_MS 500

/* --- Idle layout grid -----------------------------------------------------
 * When idle the display shows the layer's whole layout as a 6x4 grid, one cell
 * per key. 128px / 6 columns / 6px font leaves room for exactly 3 characters
 * per key, so the legends are abbreviated -- see key_abbrev().
 *
 * A layer change shows the layer name large for a moment first, then the grid
 * takes over.
 */
#define LAYER_NAME_MS 1000

// Width of one grid cell. Six of these span 126 of the 128 available pixels.
#define GRID_CELL_W 21
#define GRID_ROWS 4
#define GRID_COLS 6

/* --- Layer persistence ----------------------------------------------------
 * The active layer is remembered across unplugs. The RP2040 has no real
 * EEPROM, so QMK emulates it in flash -- writing on every layer change would
 * mean a flash write per keypress, including each step while cycling through
 * layers. Instead the save waits until the layer has been left alone for
 * LAYER_SAVE_DELAY_MS, and is skipped entirely if the value is unchanged.
 */
#define LAYER_SAVE_DELAY_MS 3000

// Tag stored alongside the layer so untouched/erased EEPROM isn't mistaken for
// a saved value.
#define LAYER_SAVE_MAGIC 0x4C590000u
#define LAYER_SAVE_MASK 0xFFFF0000u

/* --- Idle screensaver -----------------------------------------------------
 * The driver's own blanking is off (OLED_TIMEOUT 0 in config.h). Instead, after
 * SCREENSAVER_START_MS of no key activity the Matrix rain plays for one more
 * full cycle, then the panel sleeps. Any keypress returns to normal.
 */
// Raise SCREENSAVER_START_MS if you want the layer name to stay readable for
// longer; lower it to be kinder to the panel, since the layer name is static
// text at a fixed position and the rain is not.
#define SCREENSAVER_START_MS 30000
#define RAIN_DURATION_MS 30000

// How long each row of the rain takes to fall one step.
#define RAIN_STEP_MS 110

// Trail length in characters, picked at random per column within this range.
#define RAIN_TRAIL_MIN 3
#define RAIN_TRAIL_MAX 7

/* --- Layout spacing -------------------------------------------------------
 * Vertical gaps in pixels. The font is OLED_FONT_HEIGHT (8px) tall, so these
 * are fractions of a text line rather than whole rows.
 */

// Between the "Starting"/"Reseting" label and the loading bar.
#define BOOT_LABEL_GAP 10

// Between a key's name and its raw keycode.
#define KEY_INFO_GAP 8

/* -------------------------------------------------------------------------
 * 4. State
 * ---------------------------------------------------------------------- */

/* --- Long-press tracking --------------------------------------------------
 * *_key_held is set while the key is down; *_fired records that the hold
 * already ran, so the key's own release action can be swallowed.
 */
static bool     reset_key_held  = false;
static uint32_t reset_key_timer = 0;
static bool     reset_fired     = false;

static bool     oled_toggle_key_held  = false;
static uint32_t oled_toggle_key_timer = 0;
static bool     oled_toggle_fired     = false;

/* --- Boot sequence -------------------------------------------------------- */

// Reference point for the boot animation/splash. Rewound on soft reset so the
// whole intro replays without power-cycling the board.
static uint32_t boot_timer = 0;

// Whether the current run of the boot sequence came from a soft reset, which
// only changes the label drawn above the loading bar.
static bool boot_from_reset = false;

/* --- Display state -------------------------------------------------------- */

// User's on/off preference for the display, flipped by the long-press toggle.
static bool oled_user_enabled = true;

static oled_message_t oled_message       = MSG_NONE;
static uint32_t       oled_message_timer = 0;

// How many keys are physically down right now. While this is non-zero the key
// info stays on screen, so holding a key doesn't flip to the layer name (and
// then flash the *next* layer) partway through a long press.
static uint8_t keys_held = 0;

// Name and raw code of the last key pressed, for the OLED to show.
static char last_key_pressed[20] = "";
static char last_key_code[8]     = "";

// When the last key event happened, which drives the screensaver and sleep.
static uint32_t last_activity_timer = 0;

// When the active layer last changed, so the layer name can be shown large for
// LAYER_NAME_MS before the layout grid replaces it.
static uint32_t layer_change_timer = 0;

// A layer change is waiting to be written to EEPROM once it has settled.
static bool     layer_save_pending = false;
static uint32_t layer_save_timer   = 0;

/* -------------------------------------------------------------------------
 * 5. Key handling
 * ---------------------------------------------------------------------- */

// Records the name/code of the key just pressed, for the OLED to show.
static void set_last_key_display(uint16_t keycode) {
    // Check if the keycode is a standard key or a custom macro
    if (keycode >= SAFE_RANGE) {
        // For custom keycodes, display a special message
        switch (keycode) {
            case MAC_SELALL:
                sprintf(last_key_pressed, "Select All");
                break;
            case MAC_CPY:
                sprintf(last_key_pressed, "Copy");
                break;
            case MAC_CUT:
                sprintf(last_key_pressed, "Cut");
                break;
            case MAC_PST:
                sprintf(last_key_pressed, "Paste");
                break;
            case MAC_FIND:
                sprintf(last_key_pressed, "Find");
                break;
            case MAC_REDO:
                sprintf(last_key_pressed, "Redo");
                break;
            case MAC_UNDO:
                sprintf(last_key_pressed, "Undo");
                break;
            case MAC_SAVE:
                sprintf(last_key_pressed, "Save");
                break;
            case MAC_NEWWIN:
                sprintf(last_key_pressed, "New Window");
                break;
            case MAC_WIN:
                sprintf(last_key_pressed, "Switch Win");
                break;
            case MAC_CLOSE:
                sprintf(last_key_pressed, "Close");
                break;
            case MAC_REL:
                sprintf(last_key_pressed, "Reload");
                break;
            case MAC_NEWTAB:
                sprintf(last_key_pressed, "New Tab");
                break;
            case MAC_TABS:
                sprintf(last_key_pressed, "Next Tab");
                break;
            case MAC_PRVTAB:
                sprintf(last_key_pressed, "Prev Tab");
                break;
            case MAC_SECURE:
                sprintf(last_key_pressed, "Security");
                break;
            case MAC_TASKMG:
                sprintf(last_key_pressed, "Task Mgr");
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
        // Row 1 - clipboard
        case MAC_SELALL:
            tap_code16(LCTL(KC_A));
            return true;
        case MAC_CPY:
            tap_code16(LCTL(KC_C));
            return true;
        case MAC_CUT:
            tap_code16(LCTL(KC_X));
            return true;
        case MAC_PST:
            tap_code16(LCTL(KC_V));
            return true;
        // Row 2 - editing
        case MAC_FIND:
            tap_code16(LCTL(KC_F));
            return true;
        case MAC_REDO:
            tap_code16(LCTL(KC_Y));
            return true;
        case MAC_UNDO:
            tap_code16(LCTL(KC_Z));
            return true;
        case MAC_SAVE:
            tap_code16(LCTL(KC_S));
            return true;
        // Row 3 - windows
        case MAC_NEWWIN:
            tap_code16(LCTL(KC_N));
            return true;
        case MAC_WIN:
            tap_code16(LALT(KC_TAB));
            return true;
        case MAC_CLOSE:
            tap_code16(LCTL(KC_W));
            return true;
        case MAC_REL:
            tap_code16(LCTL(KC_R));
            return true;
        // Row 4 - tabs and system
        case MAC_NEWTAB:
            tap_code16(LCTL(KC_T));
            return true;
        case MAC_TABS:
            tap_code16(LCTL(KC_TAB));
            return true;
        case MAC_PRVTAB:
            tap_code16(LCTL(LSFT(KC_TAB)));
            return true;
        case MAC_SECURE:
            tap_code16(LCTL(LALT(KC_DEL)));
            return true;
        case MAC_TASKMG:
            tap_code16(LCTL(LSFT(KC_ESC)));
            return true;
    }
    return false;
}

/* -------------------------------------------------------------------------
 * 6. QMK callbacks
 * ---------------------------------------------------------------------- */

// The layer saved in EEPROM, or 0 if nothing valid is stored there.
static uint8_t load_saved_layer(void) {
    uint32_t raw = eeconfig_read_user();
    if ((raw & LAYER_SAVE_MASK) != LAYER_SAVE_MAGIC) {
        return 0;
    }
    uint8_t layer = (uint8_t)(raw & 0xFF);
    // Guard against a stored layer that no longer exists, e.g. after the
    // keymap is rebuilt with fewer layers.
    return (layer <= _MACROS) ? layer : 0;
}

void keyboard_post_init_user(void) {
    // Raw matrix-scan printing over `qmk console`, so ghosting/wiring issues
    // can be diagnosed without a multimeter.
    debug_enable = true;
    debug_matrix = true;

    rgblight_enable_noeeprom();
    rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
    rgblight_sethsv_noeeprom(0, 255, 255);

    // Come back on whichever layer was in use when the board was unplugged.
    layer_move(load_saved_layer());
}

// Restart the "show the layer name large" window whenever the layer changes,
// and queue the new layer to be persisted once it settles.
layer_state_t layer_state_set_user(layer_state_t state) {
    layer_change_timer = timer_read32();
    layer_save_pending = true;
    layer_save_timer   = timer_read32();
    return state;
}

// Steps the onboard RGB LED, and drives both long-press actions.
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

    // Persist the layer once it has been left alone, and only if it actually
    // differs from what is already stored -- flash writes are not free.
    if (layer_save_pending && timer_elapsed32(layer_save_timer) >= LAYER_SAVE_DELAY_MS) {
        layer_save_pending = false;

        uint8_t current = get_highest_layer(layer_state);
        if (current != load_saved_layer()) {
            eeconfig_update_user(LAYER_SAVE_MAGIC | current);
        }
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

// This function runs when a key is pressed or released
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // Keep the held-key count up to date before any early return below. The
    // OLED shows key info for exactly as long as this is non-zero.
    if (record->event.pressed) {
        keys_held++;
    } else if (keys_held > 0) {
        keys_held--;
    }

    // Any key event counts as activity, which dismisses the screensaver and
    // wakes the panel if it had gone to sleep.
    last_activity_timer = timer_read32();

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

#ifdef OLED_ENABLE

/* -------------------------------------------------------------------------
 * 7. OLED drawing primitives
 *
 * QMK's OLED driver has no runtime font scaling, so these read the glyph
 * bitmaps directly and blit each pixel as an NxN block.
 * ---------------------------------------------------------------------- */

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

// The largest scale at which a string of char_len characters still fits across
// the display, capped at 2x.
static uint8_t oled_fitting_scale(uint8_t char_len) {
    return ((uint16_t)char_len * OLED_FONT_WIDTH * 2 <= OLED_DISPLAY_WIDTH) ? 2 : 1;
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

/* -------------------------------------------------------------------------
 * 8. OLED screens
 * ---------------------------------------------------------------------- */

// Connect animation: a "Starting..." / "Reseting..." label above a bordered
// loading bar that fills up over ANIM_DURATION_MS, with the dots keeping pace
// with the fill.
static void render_boot_animation(uint32_t elapsed) {
    const uint8_t bar_width  = 100;
    const uint8_t bar_height = 12;
    const uint8_t gap        = BOOT_LABEL_GAP;

    const char *label   = boot_from_reset ? RESET_MSG_TEXT : BOOT_MSG_TEXT;
    uint8_t     max_len = strlen(label) + BOOT_MSG_DOTS;
    uint8_t     scale   = oled_fitting_scale(max_len);
    uint8_t     label_h = OLED_FONT_HEIGHT * scale;
    uint8_t     block_h = label_h + gap + bar_height;
    uint8_t     label_y = (OLED_DISPLAY_HEIGHT > block_h) ? (OLED_DISPLAY_HEIGHT - block_h) / 2 : 0;

    // Type the label out first, then pace the dots across the time that's left
    // so the last dot lands as the bar fills.
    uint8_t  label_len = strlen(label);
    uint32_t typed_ms  = (uint32_t)label_len * BOOT_LETTER_MS;

    uint8_t shown = (BOOT_LETTER_MS > 0) ? elapsed / BOOT_LETTER_MS : label_len;
    if (shown > label_len) {
        shown = label_len;
    }

    uint8_t dots = 0;
    if (elapsed >= typed_ms && ANIM_DURATION_MS > typed_ms) {
        uint32_t dot_ms = (ANIM_DURATION_MS - typed_ms) / (BOOT_MSG_DOTS + 1);
        if (dot_ms > 0) {
            dots = (elapsed - typed_ms) / dot_ms;
            if (dots > BOOT_MSG_DOTS) {
                dots = BOOT_MSG_DOTS;
            }
        }
    }

    char    buf[24];
    uint8_t len = 0;
    for (uint8_t i = 0; i < shown && len < sizeof(buf) - 1; i++) {
        buf[len++] = label[i];
    }
    for (uint8_t i = 0; i < dots && len < sizeof(buf) - 1; i++) {
        buf[len++] = '.';
    }
    buf[len] = '\0';

    // Anchor to the full-length string so the text doesn't creep sideways as
    // dots appear (and doesn't leave stale pixels behind).
    oled_write_string_scaled_at(buf, oled_centered_x(max_len, scale), label_y, scale);

    uint8_t bar_x = (OLED_DISPLAY_WIDTH - bar_width) / 2;
    uint8_t bar_y = label_y + label_h + gap;

    oled_draw_rect(bar_x, bar_y, bar_width, bar_height, false);

    const uint8_t inset   = 2;
    uint8_t       inner_w = bar_width - (inset * 2);
    uint8_t       inner_h = bar_height - (inset * 2);
    uint16_t      fill_w  = (uint16_t)inner_w * elapsed / ANIM_DURATION_MS;
    if (fill_w > inner_w) {
        fill_w = inner_w;
    }

    oled_draw_rect(bar_x + inset, bar_y + inset, (uint8_t)fill_w, inner_h, true);
}

// Centered splash shown after the animation: SPLASH_LINE1, a blank line, then
// SPLASH_LINE2.
static void render_splash(void) {
    uint8_t line_height  = OLED_FONT_HEIGHT * SPLASH_SCALE;
    uint8_t gap_height   = OLED_FONT_HEIGHT; // the blank line
    uint8_t total_height = line_height * 2 + gap_height;
    uint8_t start_y      = (OLED_DISPLAY_HEIGHT > total_height) ? (OLED_DISPLAY_HEIGHT - total_height) / 2 : 0;

    oled_write_string_scaled_centered(SPLASH_LINE1, start_y, SPLASH_SCALE);
    oled_write_string_scaled_centered(SPLASH_LINE2, start_y + line_height + gap_height, SPLASH_SCALE);
}

// Draws the current long-press confirmation message, centered on one line.
static void render_message(void) {
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

    uint8_t scale = oled_fitting_scale(strlen(text));
    uint8_t y0    = (OLED_DISPLAY_HEIGHT > OLED_FONT_HEIGHT * scale) ? (OLED_DISPLAY_HEIGHT - OLED_FONT_HEIGHT * scale) / 2 : 0;

    oled_write_string_scaled_centered(text, y0, scale);
}

/* --- Layout grid -----------------------------------------------------------
 * The grid reads the keymap at runtime rather than keeping its own copy of the
 * layout, so rearranging a layer needs no changes here.
 */

// Arrow glyphs live in the font's control-code range (see lib/glcdfont.c).
#define GLYPH_UP "\x18"
#define GLYPH_DOWN "\x19"
#define GLYPH_RIGHT "\x1A"
#define GLYPH_LEFT "\x1B"

// At most three characters per key. Necessarily terse on the macro layer; the
// printed reference card in the keyboard folder carries the full names.
static const char *key_abbrev(uint16_t kc) {
    switch (kc) {
        case KC_F1 ... KC_F12: {
            static char buf[4];
            snprintf(buf, sizeof(buf), "F%u", (unsigned)(kc - KC_F1 + 1));
            return buf;
        }
        case KC_F13 ... KC_F24: {
            static char buf[4];
            snprintf(buf, sizeof(buf), "F%u", (unsigned)(kc - KC_F13 + 13));
            return buf;
        }
        // Note the range order: the keypad digits run KC_P1..KC_P9 and then
        // KC_P0, so KC_P0 is the *end* of the range, not the start.
        case KC_P1 ... KC_P0: {
            if (!host_keyboard_led_state().num_lock) {
                // With Num Lock off the keypad sends its navigation functions
                // instead of digits. Keypad 5 does nothing, hence the blank.
                static const char *const nav[] = {"End", GLYPH_DOWN, "PgD",   GLYPH_LEFT, "",
                                                  GLYPH_RIGHT, "Hom", GLYPH_UP, "PgU", "Ins"};
                return nav[kc - KC_P1];
            }
            static char buf[2];
            buf[0] = (char)('0' + ((kc - KC_P1 + 1) % 10));
            buf[1] = '\0';
            return buf;
        }

        case KC_INS:  return "Ins";
        case KC_DEL:  return "Del";
        case KC_HOME: return "Hom";
        case KC_END:  return "End";
        case KC_PGUP: return "PgU";
        case KC_PGDN: return "PgD";
        case KC_ESC:  return "Esc";
        case KC_UP:   return GLYPH_UP;
        case KC_DOWN: return GLYPH_DOWN;
        case KC_RGHT: return GLYPH_RIGHT;
        case KC_LEFT: return GLYPH_LEFT;

        case KC_PPLS: return "+";
        case KC_PMNS: return "-";
        case KC_PAST: return "*";
        case KC_PSLS: return "/";
        case KC_PDOT: return host_keyboard_led_state().num_lock ? "." : "Del";
        case KC_NUM:  return "NUM";

        case MAC_SELALL: return "SEL";
        case MAC_CPY:    return "CPY";
        case MAC_CUT:    return "CUT";
        case MAC_PST:    return "PST";
        case MAC_FIND:   return "FND";
        case MAC_REDO:   return "RDO";
        case MAC_UNDO:   return "UND";
        case MAC_SAVE:   return "SAV";
        case MAC_NEWWIN: return "NWN";
        case MAC_WIN:    return "SWN";
        case MAC_CLOSE:  return "CLS";
        case MAC_REL:    return "RLD";
        case MAC_NEWTAB: return "NTB";
        case MAC_TABS:   return "NXT";
        case MAC_PRVTAB: return "PRV";
        case MAC_SECURE: return "SEC";
        case MAC_TASKMG: return "TSK";
    }

    // The layer-step key: TG() on layers 0-2, TO() on layer 3.
    if (IS_QK_TO(kc) || IS_QK_TOGGLE_LAYER(kc)) {
        return "Lyr";
    }
    return "";
}

// The keycode a position resolves to on `layer`, following transparency down.
static uint16_t resolved_keycode(uint8_t layer, uint8_t row, uint8_t col) {
    keypos_t pos = {.row = row, .col = col};
    for (int8_t l = (int8_t)layer; l >= 0; l--) {
        uint16_t kc = keymap_key_to_keycode((uint8_t)l, pos);
        if (kc != KC_TRNS) {
            return kc;
        }
    }
    return KC_NO;
}

// Draws the whole layer as a 6x4 grid, one cell per physical key.
static void render_layout_grid(uint8_t layer) {
    // 4 rows of 8px with 8px between them = 56px, centred in 64px.
    const uint8_t row_pitch = OLED_FONT_HEIGHT * 2;
    const uint8_t top       = (OLED_DISPLAY_HEIGHT - (GRID_ROWS * row_pitch - OLED_FONT_HEIGHT)) / 2;

    for (uint8_t r = 0; r < GRID_ROWS; r++) {
        for (uint8_t c = 0; c < GRID_COLS; c++) {
            const char *txt = key_abbrev(resolved_keycode(layer, r, c));
            uint8_t     len = strlen(txt);
            if (len == 0) {
                continue;
            }

            // Centre the legend inside its cell.
            uint8_t cell_x = c * GRID_CELL_W;
            uint8_t x      = cell_x + (GRID_CELL_W - len * OLED_FONT_WIDTH) / 2;

            oled_write_string_scaled_at(txt, x, top + r * row_pitch, 1);
        }
    }
}

static const char *layer_name(uint8_t layer) {
    switch (layer) {
        case _FUNCTION:
            return "FUNCTION";
        case _SPECIAL_FUNCTION:
            return "SPECIAL";
        case _NUMPAD:
            return "NUMPAD";
        case _MACROS:
            return "MACROS";
    }
    return "UNDEFINED";
}

// The idle screen. For LAYER_NAME_MS after a layer change it shows the layer
// name large and centred; after that it switches to the layout grid.
//
// `force` must be set when arriving from a different screen, because the
// content-change check below would otherwise skip the draw and leave the
// screen blank after oled_task_user() cleared it.
static void render_main_info(bool force) {
    uint8_t current_layer = get_highest_layer(layer_state);
    bool    naming        = timer_elapsed32(layer_change_timer) < LAYER_NAME_MS;
    // The numpad legends depend on Num Lock, which the host can change without
    // any key on this board being pressed, so it has to be watched here.
    bool num_lock = host_keyboard_led_state().num_lock;

    // Redraw only when the content actually changes. Two reasons: clearing
    // every frame would starve the flush (OLED_UPDATE_PROCESS_LIMIT is 1 block
    // per frame), and redrawing the 24-cell grid every scan would burn a lot of
    // cycles for an identical result.
    static int8_t last_layer    = -1;
    static int8_t last_naming   = -1;
    static int8_t last_num_lock = -1;
    if (!force && current_layer == last_layer && (int8_t)naming == last_naming && (int8_t)num_lock == last_num_lock) {
        return;
    }
    last_layer    = current_layer;
    last_naming   = naming;
    last_num_lock = num_lock;

    oled_clear();

    if (naming) {
        uint8_t y = (OLED_DISPLAY_HEIGHT - OLED_FONT_HEIGHT * 2) / 2;
        oled_write_string_scaled_centered(layer_name(current_layer), y, 2);
    } else {
        render_layout_grid(current_layer);
    }

    // Display WPM to enable uncomment he below code and in rules.mk
    // oled_set_cursor(0, 2);
    // char wpm_string[10];
    // sprintf(wpm_string, "WPM: %u", get_current_wpm());
    // oled_write(wpm_string, false);
}

// The last pressed key: its name (large) above its raw code (normal size),
// both centered on the screen as a block.
static void render_key_info(void) {
    // Only clear when the key actually changes (see render_main_info for why).
    static char last_rendered_key[sizeof(last_key_pressed)] = "";
    if (strcmp(last_key_pressed, last_rendered_key) != 0) {
        strcpy(last_rendered_key, last_key_pressed);
        oled_clear();
    }

    // Both lines are positioned by pixel rather than by character row, so the
    // gap between them can be finer than a whole 8px line.
    uint8_t name_scale = oled_fitting_scale(strlen(last_key_pressed));
    uint8_t name_h     = OLED_FONT_HEIGHT * name_scale;
    uint8_t code_h     = OLED_FONT_HEIGHT; // the code is always 1x
    uint8_t block_h    = name_h + KEY_INFO_GAP + code_h;
    uint8_t start_y    = (OLED_DISPLAY_HEIGHT > block_h) ? (OLED_DISPLAY_HEIGHT - block_h) / 2 : 0;

    // Line 1: key name, larger font, centered
    oled_write_string_scaled_centered(last_key_pressed, start_y, name_scale);

    // Line 2: raw keycode, normal (smaller) font, centered
    oled_write_string_scaled_centered(last_key_code, start_y + name_h + KEY_INFO_GAP, 1);
}

/* --- Matrix rain screensaver ----------------------------------------------
 * Columns of characters falling down the screen. Each column keeps its own
 * head position, trail length and fall rate so they don't move in lockstep.
 * The panel is 1-bit, so the fade-out of a real Matrix trail is faked by
 * thinning the tail out towards its end.
 */

#define RAIN_COLS (OLED_DISPLAY_WIDTH / OLED_FONT_WIDTH)
#define RAIN_ROWS (OLED_DISPLAY_HEIGHT / OLED_FONT_HEIGHT)

static int8_t  rain_head[RAIN_COLS]; // row of the leading character, may be negative
static uint8_t rain_len[RAIN_COLS];  // trail length in characters
static uint8_t rain_rate[RAIN_COLS]; // advance one row every N steps
static uint8_t rain_tick[RAIN_COLS]; // steps since this column last advanced

// 16-bit xorshift. A local generator keeps the effect self-contained and
// avoids pulling rand() in from libc.
static uint16_t rain_rng = 0xACE1u;

static uint16_t rain_rand(void) {
    rain_rng ^= rain_rng << 7;
    rain_rng ^= rain_rng >> 9;
    rain_rng ^= rain_rng << 8;
    return rain_rng;
}

// Character shown at a cell. Hashing the position keeps a cell's glyph stable
// as the trail passes over it; the head is salted with the step so it flickers.
static char rain_glyph(uint8_t col, int8_t row, uint16_t salt) {
    static const char charset[] = "0123456789ABCDEFXYZ<>*+=-/\\|:;";

    uint16_t h = (uint16_t)col * 31u + (uint16_t)(uint8_t)row * 131u + salt * 7919u;
    h ^= h >> 5;
    h *= 0x2545u;
    h ^= h >> 7;
    return charset[h % (sizeof(charset) - 1)];
}

// Restart a column above the top of the screen with fresh random properties.
static void rain_respawn(uint8_t col) {
    rain_head[col] = -(int8_t)(rain_rand() % RAIN_ROWS);
    rain_len[col]  = RAIN_TRAIL_MIN + (rain_rand() % (RAIN_TRAIL_MAX - RAIN_TRAIL_MIN + 1));
    rain_rate[col] = 1 + (rain_rand() % 3);
    rain_tick[col] = 0;
}

static void rain_init(void) {
    for (uint8_t c = 0; c < RAIN_COLS; c++) {
        rain_respawn(c);
        // Stagger the initial heads so the screen doesn't start off empty.
        rain_head[c] -= rain_rand() % RAIN_ROWS;
    }
}

static void render_matrix_rain(void) {
    static uint32_t step_timer = 0;
    static uint16_t step       = 0;

    // Only touch the buffer when the animation actually advances. Redrawing
    // every frame would re-dirty all 32 blocks faster than the driver can
    // flush them (OLED_UPDATE_PROCESS_LIMIT is 1 block per frame).
    if (timer_elapsed32(step_timer) < RAIN_STEP_MS) {
        return;
    }
    step_timer = timer_read32();
    step++;

    oled_clear();

    for (uint8_t c = 0; c < RAIN_COLS; c++) {
        if (++rain_tick[c] >= rain_rate[c]) {
            rain_tick[c] = 0;
            rain_head[c]++;
            if (rain_head[c] - (int8_t)rain_len[c] > RAIN_ROWS) {
                rain_respawn(c);
            }
        }

        for (uint8_t i = 0; i < rain_len[c]; i++) {
            int8_t row = rain_head[c] - (int8_t)i;
            if (row < 0 || row >= RAIN_ROWS) {
                continue;
            }

            // Thin out the last third of the trail so it appears to fade.
            if (i > (rain_len[c] * 2) / 3 && (rain_glyph(c, row, 1) & 1)) {
                continue;
            }

            char ch = rain_glyph(c, row, (i == 0) ? step : 0);
            oled_write_char_scaled(c * OLED_FONT_WIDTH, (uint8_t)row * OLED_FONT_HEIGHT, ch, 1);
        }
    }
}

// Picks which screen to show. Each branch clears once on entry so leftovers
// from the previous screen don't survive.
bool oled_task_user(void) {
    static oled_screen_t last_screen      = OLED_SCREEN_NONE;
    static bool          was_user_enabled = true;
    static bool          was_asleep       = false;

    // User has held the toggle key to turn the screen off. Keep asserting the
    // off state here rather than switching it off once, because any dirty
    // buffer makes the driver switch the panel back on.
    if (!oled_user_enabled) {
        oled_off();
        was_user_enabled = false;
        // Force a full repaint whenever the screen comes back.
        last_screen = OLED_SCREEN_NONE;
        return false;
    }

    if (!was_user_enabled) {
        was_user_enabled = true;
        oled_on();
    }

    // Idle handling is ours to do: OLED_TIMEOUT is 0, so the driver neither
    // blanks the panel nor wakes it on keypress.
    uint32_t idle = timer_elapsed32(last_activity_timer);

    if (idle >= SCREENSAVER_START_MS + RAIN_DURATION_MS) {
        // The screensaver has had its cycle; sleep for real.
        oled_off();
        was_asleep  = true;
        last_screen = OLED_SCREEN_NONE;
        return false;
    }

    if (was_asleep) {
        was_asleep = false;
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
    } else if (idle >= SCREENSAVER_START_MS) {
        // Idle long enough: run the screensaver until it's time to sleep
        if (last_screen != OLED_SCREEN_RAIN) {
            last_screen = OLED_SCREEN_RAIN;
            rain_init();
            oled_clear();
        }
        render_matrix_rain();
    } else if (keys_held == 0) {
        // Nothing held: show the layer name, then its layout grid
        bool entering = (last_screen != OLED_SCREEN_LAYER);
        if (entering) {
            if (last_screen == OLED_SCREEN_SPLASH) {
                // The boot sequence outlasts the naming window that started at
                // power-on, so restart it here and let the layer name have its
                // moment before the grid appears. Deliberately not done when
                // arriving from the key screen, or the name would flash on
                // every key release.
                layer_change_timer = timer_read32();
            }
            last_screen = OLED_SCREEN_LAYER;
            oled_clear();
        }
        render_main_info(entering);
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

#endif // OLED_ENABLE
