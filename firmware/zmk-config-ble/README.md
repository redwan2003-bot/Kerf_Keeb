# kerf_keeb (ByranTK LP63): ZMK config, Bluetooth / USB / BLE dongle

65-key column-aligned grid (13 × 5, all 1U except a 2U space), Gateron KS-33 low-profile
switches, a nice!nano v2-compatible nRF52840, a 74HC595 row driver, a nice!view, an Alps EC12 encoder
with push and 8 × WS2812 underglow. Bluetooth + USB, with an
optional USB dongle (the custom nRF52833 kerf dongle, or a spare nice!nano).

> **Current: firmware v12 (2026-11-04).** The numbered list at the end of this file is the change
> history. Where it disagrees with this section, this section is current.

## Layout

```
Esc   1    2    3    4    5    6    7    8    9    0    [    ]
Tab   Q    W    E    R    T    Y    U    I    O    P    -    Bksp
Caps  A    S    D    F    G    H    J    K    L    ;    '    Enter
Shift Z    X    C    V    B    N    M    ,    .    /    Shift Up
Ctrl  Win  Alt  Fn  [Space ]  =    \    `    Del  Snip Left Down Right      + knob (key position 66)
```

Snip = Win + Shift + S. The knob turns for volume (Fn + turn: underglow brightness); a press is Mute
(Fn + press: Play/Pause).

## Fn layer (hold Fn)

| Keys | Action |
|---|---|
| 1 … 0, [, ] | F1 … F12 |
| Q W E R T | Bluetooth host profile 1–5 |
| A / S / D | Output: USB / Bluetooth / toggle |
| Z / X / C / V | Underglow on-off / next effect / hue up / hue down |
| G / H, J / K, , / . | Saturation − / +, effect speed − / +, brightness − / + |
| B / N | Bootloader / reset |
| M | Mute |
| P / - / Bksp | Print Screen / Home / End |
| ↑ ← ↓ → | Page Up / Home / Page Down / End |
| Del | Clear the current Bluetooth pairing |
| Snip | Print Screen |
| Knob | turn: underglow brightness; press: Play/Pause |

## Hardware pin map (nice!nano v2 pin names)

| Function | Pin |
|---|---|
| Matrix columns (inputs) | D8, D9, D2, D3, D4, D5, D6, D7, D21 (D21 = Snip key column) |
| Matrix rows | 74HC595 outputs (driven over spi0), `row2col` |
| spi0 SCK / MOSI | D10 (P0.09) / D16 (P0.10) |
| nice!view CS | D15 (active high) |
| 74HC595 latch (RCLK) | D14 |
| WS2812 data (spi1 MOSI) | D19 (P0.02). spi1 SCK goes to P1.07, a back pad not connected on this PCB |
| Underglow power enable | D20 (P0.29), active high (Q2 → Q1 feeds LED_VCC from B+). P0.13 (VCC) is held on by a GPIO hog |
| Encoder A / B | D0 (P0.08) / D1 (P0.06); UART on these pins is disabled |
| Encoder push | D18 (P1.15), active low with pull-up, read by a `zmk,kscan-gpio-direct` inside a `zmk,kscan-composite` (matrix + direct), row 8 col 0 = key position 66 |

## Builds (`build.yaml`)

| Artifact | Flash to | Use |
|---|---|---|
| `byrantk_lp63_standalone` | keyboard | Normal mode: keyboard talks to your PC directly over Bluetooth or USB. **Flash this first and test everything.** |
| `settings_reset` | keyboard and dongle | Clears pairings. Use before switching between standalone and dongle mode. |
| `byrantk_lp63_dongle` | second nRF52840 Pro Micro | USB receiver. |
| `byrantk_lp63_peripheral` | keyboard | Dongle mode: keyboard connects only to the dongle. |

Flashing: double-tap reset (or Fn + B), a USB drive appears, copy the `.uf2` onto it.

## Changes from the Wizard output

1. Real keymap and Fn layer (Wizard only generates `&kp A B C …` placeholders).
2. Encoder bindings: volume on Base, RGB brightness on Fn.
3. Keyboard-only Kconfig added to `Kconfig.defconfig` under `SHIELD_BYRANTK_LP63`:
   EC11 driver + trigger, `ZMK_RGB_UNDERGLOW=y`, `ZMK_RGB_UNDERGLOW_EXT_POWER=y`
   (RGB off also cuts power to the LEDs through Q1; see item 8),
   lights off at start, brightness capped at 60 %.
4. `config/byrantk_lp63_dongle.keymap` added (the dongle runs the keymap).
5. `config/byrantk_lp63_dongle.conf` added.
6. `build.yaml`: standalone, peripheral, dongle and settings_reset all enabled.
7. Encoder moved from P1.01/P1.02 to D0/D1 so it uses the main header pins (works on nice!nano clones too); `uart0` disabled.
8. Underglow power switch: EXT_POWER drives D20 (P0.29, active high) -> Q2 (AO3400A) -> Q1 (AO3401A) feeding LED_VCC from B+ (battery rail), R2/R3 100k keep it off at boot. The nice!nano VCC switch (P0.13) is held on with a GPIO hog because VCC powers the 74HC595 and the nice!view. Turning underglow off removes the LEDs' ~5-8 mA idle draw.

Important: ZMK merges `config/byrantk_lp63.conf` into the dongle build too (the dongle's name
starts with `byrantk_lp63`). Keep keyboard-only settings in `Kconfig.defconfig`, not in that file.

Keep `byrantk_lp63.keymap` and `byrantk_lp63_dongle.keymap` identical when you edit the keymap.

## Verified

All four targets compiled successfully against ZMK v0.3 (Zephyr 3.5, SDK 0.16.8, board
`nice_nano_v2`). Compiling proves the config is valid. It does not prove the PCB is wired to
match. Test every key on real hardware before building the case.

9. (Superseded by 11.) Encoder changed to the low-profile Alps EC12E1240301 (8.5 mm tall, 12 pulses / 12 detents): encoder `steps = 48`, `triggers-per-rotation = 12`. The push kscan/composite was removed; the matrix is 8 x 8 and keymaps have 64 bindings per layer. Mute moved to Fn + M (underglow on/off stays on Fn + Z).
10. Underglow saturation (Fn + G/H) and effect-speed (Fn + J/K) keys added. `ZMK_RGB_UNDERGLOW_AUTO_OFF_USB` enabled: underglow switches off when USB is unplugged and restores its previous state when plugged back in, so the LEDs only run while their supply is within the WS2812B-B/W 3.7-5.3 V range.

## kerf dongle (custom nRF52833 USB-C receiver)

Board: `kerf_dongle` (in `boards/arm/kerf_dongle`). Build target: `kerf_dongle` + shield `byrantk_lp63_dongle`.

### Status LED (one colour per job)
| Colour | Meaning |
|---|---|
| Blue fast blink | searching for the keyboard (since power-up) |
| Blue solid 2 s, then off | keyboard connected |
| Blue slow blink | keyboard link lost |
| Green solid | Caps Lock on |
| Red short flash every 2 s | keyboard battery at or below 15 % (`CONFIG_KERF_DONGLE_LOW_BATTERY_PERCENT`) |

Pins: red P0.31, green P0.30, blue P0.29 (active low). If a colour on your LED batch is swapped, swap the `gpios` in `kerf_dongle.dts`.

### Latency / radio settings (dongle: `config/byrantk_lp63_dongle.conf`, keyboard: peripheral snippet)
- 7.5 ms Bluetooth connection interval (the fastest BLE allows); keypresses go out at the next connection event.
- 1 ms USB polling (1000 Hz) to the computer.
- +8 dBm transmit power on both the keyboard and the dongle.

### First-time flashing (once per new dongle)
The bare nRF52833 ships blank. Flash the UF2 bootloader once over SWD using the bottom test pads
(TP_V = 3.3 V, TP_G = GND, TP_D = SWDIO, TP_C = SWCLK, TP_R = reset) and any CMSIS-DAP probe
(e.g. a Raspberry Pi Pico running debugprobe, or a J-Link):

    pyocd flash -t nrf52833 kerf_dongle_bootloader-0.11.0_s140_7.3.0.hex
    # or: nrfjprog --program kerf_dongle_bootloader-0.11.0_s140_7.3.0.hex --chiperase --reset

The bootloader also sets the regulator to 3.3 V. After that, double-tap the reset button (SW1) and a
`KERFBOOT` drive appears: copy `kerf_dongle.uf2` onto it. Later updates: double-tap SW1 (or press
Fn + B on the keyboard while connected) and copy the new UF2.

## Later changes

11. Encoder push restored (2026-10-28): the PCB routes EC12 pin D to D18 (net ENC_SW). The matrix and a
    direct-GPIO kscan are combined in `kscan0` (composite, 9 rows), the transform gains `RC(8,0)`, and
    every keymap layer has 66 bindings. spi1 SCK moved from D18 to P1.07 (not connected), so the LED
    driver no longer drives the button pin.
12. Snip key (SW68, column D21): Win + Shift + S; Fn + Snip = Print Screen.
13. LED9 battery indicator (`&batled` on Fn + Esc), driven by `src/kerf_batled.c` through the same
    WS2812 chain; underglow sees only LED1-8 (`src/kerf_led_split.c`).
14. Battery LED removed (2026-11-04): LED9 is no longer on the PCB. `&batled`, `kerf_batled.c` and
    `kerf_led_split.c` are gone, the WS2812 chain is back to 8 pixels, and Fn + Esc is plain Esc.
    Battery level is still reported to the host and the dongle.
