# Default MacroPad24 layout

A plain baseline layout: the four layers and the clipboard/window macros, with
no OLED or RGB code. Use this as a starting point for your own keymap.

For the full build with the OLED boot animation, per-key display and
long-press actions, see the [MacroPad_v1_24_2040](../MacroPad_v1_24_2040)
keymap.

## Layers

| # | Layer               | Contents                                          |
| - | ------------------- | ------------------------------------------------- |
| 0 | `_FUNCTION`         | F1-F12, Ins/Del, Home/End, PgUp/PgDn, arrows      |
| 1 | `_SPECIAL_FUNCTION` | F13-F24                                           |
| 2 | `_NUMPAD`           | Numpad digits and operators                       |
| 3 | `_MACROS`           | Clipboard, editing, window and tab shortcuts      |

The bottom-right key steps through the layers with `TG(1)` -> `TG(2)` ->
`TG(3)`, then `TO(0)` to return to the base layer.

## Building

    qmk compile -kb diydipendu/macropad24_2040 -km default

To write it to the board, put the board in bootloader mode first (hold the
bottom-right key while plugging it in), then:

    qmk flash -kb diydipendu/macropad24_2040 -km default
