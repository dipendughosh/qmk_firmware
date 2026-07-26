# MacroPad24 v1 layout

The full-featured layout: everything in the [default](../default) keymap plus
the OLED display, RGB cycling and two long-press actions.

## Layers

| # | Layer               | Contents                                          |
| - | ------------------- | ------------------------------------------------- |
| 0 | `_FUNCTION`         | F1-F12, Ins/Del, Home/End, PgUp/PgDn, arrows      |
| 1 | `_SPECIAL_FUNCTION` | F13-F24                                           |
| 2 | `_NUMPAD`           | Numpad digits and operators                       |
| 3 | `_MACROS`           | Copy/cut/paste, undo/redo, tab and window control |

The bottom-right key steps through the layers with `TG(1)` -> `TG(2)` ->
`TG(3)`, then `TO(0)` to return to the base layer.

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

1. **Boot animation**, 1.5s: a loading bar with a `Starting...` label, or
   `Reseting...` when triggered by the long-press reset.
2. **Splash**, 2s: `MacroPad` over `DiGhosh`.
3. **Normal operation**:
   - Idle: the current layer name, large and centered.
   - While a key is held: the key's name with its raw keycode underneath.

The screen sleeps after `OLED_TIMEOUT` (60s) of inactivity and wakes on the
next keypress.

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
