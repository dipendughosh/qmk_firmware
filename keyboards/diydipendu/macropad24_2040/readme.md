# MacroPad24

A 24-key (6x4) macropad with a 128x64 OLED and an RGB status LED.

* Keyboard Maintainer: [Dipendu Ghosh](https://github.com/diydipendu)
* Hardware Supported: Waveshare RP2040-Zero
* Hardware Availability: DIY / hand-wired

## Hardware

| Function      | Pins                               |
| ------------- | ---------------------------------- |
| Matrix cols   | `GP3` `GP4` `GP5` `GP6` `GP7` `GP8` |
| Matrix rows   | `GP27` `GP26` `GP15` `GP14`        |
| OLED (I2C0)   | `GP0` (SDA), `GP1` (SCL)           |
| WS2812 RGB    | `GP16` (onboard LED)               |

Diode direction is `COL2ROW`.

## Keymaps

| Keymap                 | Description                                                          |
| ---------------------- | -------------------------------------------------------------------- |
| `default`              | Plain baseline: four layers and the macros, no OLED/RGB logic.       |
| `MacroPad_v1_24_2040`  | Full build with the OLED boot animation, per-key display and long-press actions. |

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb diydipendu/macropad24_2040 -km default

Flashing example for this keyboard:

    qmk flash -kb diydipendu/macropad24_2040 -km default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Working with the full keymap

Build the featured keymap explicitly with `-km`:

    qmk compile -kb diydipendu/macropad24_2040 -km MacroPad_v1_24_2040

That only writes a `.uf2` into `.build/`. To put it on the board, use `flash`
instead, with the board already in bootloader mode (see below):

    qmk flash -kb diydipendu/macropad24_2040 -km MacroPad_v1_24_2040

To avoid repeating `-km` on every command, set it as your default keymap once:

    qmk config user.keymap=MacroPad_v1_24_2040

After that a bare `qmk compile` / `qmk flash` targets it. Pair it with
`qmk config user.keyboard=diydipendu/macropad24_2040` to drop `-kb` as well.

To start your own keymap, copy the `default` one into a new folder:

    qmk new-keymap -kb diydipendu/macropad24_2040

You will be prompted for a name (it defaults to your git username); the copy
lands in `keymaps/<name>/`. Pass `-km <name>` to skip the prompt. Build it like
any other keymap:

    qmk compile -kb diydipendu/macropad24_2040 -km <name>

## Bootloader

Flashing always needs the board in bootloader mode first -- this applies to
`qmk flash` as much as to copying the file by hand. Enter it in any of these
ways:

* **Bootmagic reset**: hold the bottom-right key (matrix `[3,5]`) and plug in the board.
* **Physical reset button**: press the `BOOT` button on the RP2040-Zero while plugging it in.
* **Keycode in layout**: press the key mapped to `QK_BOOT`, if the keymap defines one.

The board then appears as a USB mass-storage drive, and you have two options:

* `qmk flash -kb diydipendu/macropad24_2040 -km <keymap>` builds and copies the
  `.uf2` across for you.
* Or drag `.build/diydipendu_macropad24_2040_<keymap>.uf2` onto the drive
  yourself, which is handy if `qmk flash` cannot find the device.

The board reboots into the new firmware as soon as the copy finishes.
