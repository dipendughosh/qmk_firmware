# MacroPad24 v1 layout

The full-featured layout: everything in the [default](../default) keymap plus
the OLED display, RGB cycling and two long-press actions.

## Layers

Four layers, cycled with the bottom-right key. The pad is a 6x4 grid; the
tables below are laid out the same way you see it.

| # | Layer               | Purpose                                    |
| - | ------------------- | ------------------------------------------ |
| 0 | `_FUNCTION`         | Function keys and navigation (base layer)  |
| 1 | `_SPECIAL_FUNCTION` | Extended function keys F13-F24             |
| 2 | `_NUMPAD`           | Number pad                                 |
| 3 | `_MACROS`           | Editing and window shortcuts               |

Entries in (brackets) are inherited from a lower layer, so they do the same
thing on every layer.

For blank keycaps there is a printable version of these tables at
[keycap-reference.html](../../keycap-reference.html), laid out at actual key
pitch so it can sit next to the pad.

### Layer 0 - Function

| 1          | 2           | 3        | 4          | 5         | 6            |
| ---------- | ----------- | -------- | ---------- | --------- | ------------ |
| F1         | F2          | F3       | F4         | Insert    | Delete       |
| F5         | F6          | F7       | F8         | Home      | End          |
| F9         | F10         | F11      | F12        | Page Up   | Page Down    |
| Arrow Left | Arrow Right | Arrow Up | Arrow Down | Escape    | **-> Layer 1** |

### Layer 1 - Special Function

| 1          | 2           | 3        | 4          | 5         | 6            |
| ---------- | ----------- | -------- | ---------- | --------- | ------------ |
| F13        | F14         | F15      | F16        | (Insert)  | (Delete)     |
| F17        | F18         | F19      | F20        | (Home)    | (End)        |
| F21        | F22         | F23      | F24        | (Page Up) | (Page Down)  |
| (Arrow Left) | (Arrow Right) | (Arrow Up) | (Arrow Down) | (Escape) | **-> Layer 2** |

### Layer 2 - Numpad

| 1     | 2       | 3       | 4        | 5         | 6            |
| ----- | ------- | ------- | -------- | --------- | ------------ |
| Num 1 | Num 2   | Num 3   | Num +    | (Insert)  | (Delete)     |
| Num 4 | Num 5   | Num 6   | Num -    | (Home)    | (End)        |
| Num 7 | Num 8   | Num 9   | Num *    | (Page Up) | (Page Down)  |
| Num 0 | Num .   | Num /   | Num Lock | (Escape)  | **-> Layer 3** |

### Layer 3 - Macros

Grouped by row: clipboard, editing, windows, then tabs and system.

| 1          | 2             | 3            | 4        | 5            | 6              |
| ---------- | ------------- | ------------ | -------- | ------------ | -------------- |
| Select All | Copy          | Cut          | Paste    | (Insert)     | (Delete)       |
| Find       | Redo          | Undo         | Save     | (Home)       | (End)          |
| New Window | Switch Window | Close Window | Reload   | (Page Up)    | (Page Down)    |
| New Tab    | Next Tab      | Prev Tab     | Security | Task Manager | **-> Layer 0** |

The shortcut each one sends:

| Key           | Sends      | Key          | Sends              |
| ------------- | ---------- | ------------ | ------------------ |
| Select All    | `Ctrl+A`   | New Window   | `Ctrl+N`           |
| Copy          | `Ctrl+C`   | Switch Window| `Alt+Tab`          |
| Cut           | `Ctrl+X`   | Close Window | `Ctrl+W`           |
| Paste         | `Ctrl+V`   | Reload       | `Ctrl+R`           |
| Find          | `Ctrl+F`   | New Tab      | `Ctrl+T`           |
| Redo          | `Ctrl+Y`   | Next Tab     | `Ctrl+Tab`         |
| Undo          | `Ctrl+Z`   | Prev Tab     | `Ctrl+Shift+Tab`   |
| Save          | `Ctrl+S`   | Security     | `Ctrl+Alt+Del`     |
|               |            | Task Manager | `Ctrl+Shift+Esc`   |

**Security** is `Ctrl+Alt+Del` -- it opens the Windows security screen (Lock,
Switch user, Sign out, Task Manager). The key beside it skips that menu and
goes straight to Task Manager.

### Remembering the layer

The active layer survives an unplug: on connect the board returns to whichever
layer it was on. The soft reset still drops back to layer 0, and that is
remembered too.

The RP2040 has no real EEPROM, so QMK emulates it in flash. To avoid a flash
write on every layer key press -- including each intermediate step while
cycling -- the save is deferred until the layer has been left alone for
`LAYER_SAVE_DELAY_MS` (3s), and is skipped if the value has not changed. Switch
layers and unplug within that window and the change will not have been stored.

### Cycling layers

The bottom-right key advances one layer per press and wraps back to the base
layer from the last one:

    Layer 0 --> Layer 1 --> Layer 2 --> Layer 3 --> back to Layer 0

The current layer name is shown on the OLED whenever no key is held.

## Long-press actions

Two keys gain a second function when held for 1.5s. Both are matched by matrix
position, so they behave the same on every layer.

| Key            | Tap                  | Hold 1.5s                                                      |
| -------------- | -------------------- | -------------------------------------------------------------- |
| `[3,5]` bottom-right | Layer step (`TG`/`TO`) | Soft reset: replays the boot sequence and returns to layer 0 |
| `[3,4]`        | Its normal keycode   | Toggles the OLED off/on, with a "Screen Off"/"Screen On" confirmation |

The soft reset is display-only plus a layer reset; it does **not** reboot the
MCU or enter the bootloader.

## Display

On connect (and on soft reset) the OLED plays a short sequence:

1. **Boot animation**, 1.5s: a loading bar with a `STARTING...` label, or
   `RESETTING...` when triggered by the long-press reset. The label types
   itself out a letter at a time (`BOOT_LETTER_MS`, 50ms each) and finishes
   after ~0.4s, then the dots are paced across the remaining time so the last
   one lands as the bar fills. The words are uppercase because the 6x8 font has
   no room for descenders -- a lowercase `g` renders with its tail clipped.
2. **Splash**, 2s: `MacroPad` over `DiGhosh`.
3. **Normal operation**:
   - Idle: the current layer name, large and centered.
   - While a key is held: the key's name with its raw keycode underneath.

## Idle screensaver

Left alone, the display works through three stages:

| Idle time  | What is shown                                |
| ---------- | -------------------------------------------- |
| 0 - 30s    | Normal operation (layer name)                |
| 30s - 60s  | Matrix rain -- columns of characters falling |
| 60s onward | Panel off                                    |

Any keypress at any point returns straight to normal operation and wakes the
panel if it had slept.

The rain gives each of the 21 columns its own head position, trail length
(`RAIN_TRAIL_MIN`..`RAIN_TRAIL_MAX`) and fall rate, so the columns do not move
in lockstep. Because the panel is 1-bit there is no brightness gradient to fade
the trail with, so the tail is thinned out towards its end instead. Fall speed
is `RAIN_STEP_MS` per row.

Note that `OLED_TIMEOUT` is set to `0` in `config.h`: the driver's own idle
blanking is disabled so the screensaver can run first. That also stops QMK
waking the panel on keypress, so the keymap drives `oled_on()`/`oled_off()`
itself.

## RGB

The onboard WS2812 on `GP16` steps to the next hue once a second.

## Tuning

The timings, key positions and on-screen text are all `#define`s grouped in the
Configuration section near the top of `keymap.c`.

## Building

    qmk compile -kb diydipendu/macropad24_2040 -km MacroPad_v1_24_2040

To write it to the board, put the board in bootloader mode first -- hold the
bottom-right key while plugging it in -- then:

    qmk flash -kb diydipendu/macropad24_2040 -km MacroPad_v1_24_2040

Note that the bottom-right key does double duty: held *while plugging in* it
triggers the bootloader, but held *while the board is running* it only does the
display soft reset described above.
