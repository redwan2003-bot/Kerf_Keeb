# kerf_keeb firmware

There are two firmware sets. Pick one per setup, and use the matching file on both the keyboard and the dongle.

| Folder | Based on | Links | Best for |
|---|---|---|---|
| [`zmk-config-ble`](zmk-config-ble) | ZMK v0.3 (Zephyr 3.5) | Bluetooth to up to 5 hosts, USB, or a dongle over BLE | Everyday use, phones and tablets |
| [`zmk-config-esb`](zmk-config-esb) | ZMK main `753d8b4` (Zephyr 4.1) + [damex/zmk-feature-split-esb](https://github.com/damex/zmk-feature-split-esb) v0.8.14 | Dongle only, over Nordic Enhanced ShockBurst | Lowest latency (about 1–2 ms keypress → PC) |

Each folder is a complete ZMK user config. Push either one on its own as a GitHub repo, and its `.github/workflows/build.yml` builds every target in its `build.yaml`.

## Ready-made files (`prebuilt/`)

Built on 2026-10-28 (BLE v11, ESB v7, including the encoder push).

**`prebuilt/ble/`**

| File | Flash to | Use |
|---|---|---|
| `byrantk_lp63_standalone.uf2` | keyboard | Normal mode: Bluetooth or USB straight to your computer. **Start here.** |
| `byrantk_lp63_peripheral.uf2` | keyboard | Dongle mode: the keyboard talks only to the dongle |
| `kerf_dongle.uf2` | kerf dongle | BLE receiver on the custom nRF52833 dongle |
| `byrantk_lp63_dongle_nice_nano.uf2` | a spare nice!nano | Use a nice!nano as the receiver instead |
| `settings_reset_nice_nano.uf2` | keyboard (or a nice!nano dongle) | Clears pairings |
| `kerf_dongle_settings_reset.uf2` | kerf dongle | Clears pairings |

**`prebuilt/esb/`**

| File | Flash to |
|---|---|
| `byrantk_lp63_esb.uf2` | keyboard |
| `kerf_dongle_esb.uf2` | kerf dongle |
| `nice_nano_esb_dongle.uf2` | a spare nice!nano used as the dongle |
| `settings_reset_nice_nano.uf2`, `kerf_dongle_settings_reset.uf2` | flash once on each device when switching from BLE to ESB |

**`prebuilt/dongle-bootloader/`:** the UF2 bootloader for a blank kerf dongle (see below).

## How to flash

1. Double-tap the reset button. On the keyboard you can also press **Fn + B**.
2. A USB drive appears (`NICENANO` on the keyboard, `KERFBOOT` on the dongle).
3. Drag the `.uf2` file onto it. The device reboots by itself.

**Order:**

- **Standalone:** flash `byrantk_lp63_standalone`, then test every key, the knob, the display and the LEDs.
- **BLE dongle:** flash `settings_reset` on both devices, then `kerf_dongle` on the dongle and `byrantk_lp63_peripheral` on the keyboard.
- **ESB dongle:** flash the settings resets, then `kerf_dongle_esb`, then `byrantk_lp63_esb`. The keyboard finds the dongle on its own; there's no pairing.

## First-time dongle bootloader

A new nRF52833 is blank, so it needs the bootloader once over SWD. Use the bottom test pads (TP_V 3.3 V, TP_G, TP_D SWDIO, TP_C SWCLK, TP_R reset) with any CMSIS-DAP probe, such as a Raspberry Pi Pico running debugprobe:

```bash
pyocd flash -t nrf52833 prebuilt/dongle-bootloader/kerf_dongle_bootloader-0.11.0_s140_7.3.0.hex
# or
nrfjprog --program prebuilt/dongle-bootloader/kerf_dongle_bootloader-0.11.0_s140_7.3.0.hex --chiperase --reset
```

After that, double-tap SW1 and a `KERFBOOT` drive appears.

## Custom code in these configs

| File | What it does |
|---|---|
| `src/kerf_led_split.c` | Splits the 9-LED chain: underglow sees LED1–8, and LED9 is kept for the battery indicator |
| `src/kerf_batled.c`, `src/behaviors/behavior_batled.c` | The `&batled` key (Fn + Esc): shows the battery colour on LED9 and blinks red when low |
| `src/dongle_status_led.c` | kerf dongle RGB LED: link state, Caps Lock, keyboard battery |
| `src/behaviors/behavior_rgb_esb.c` (ESB) | `&rgb_esb`: underglow keys pressed on the dongle side are sent over ESB to the keyboard |
| `src/esb_status_screen.c` (ESB) | nice!view screen with ESB link quality |
| `boards/…/kerf_dongle` | Board definition for the custom dongle (sets the regulator to 3.3 V, RC low-frequency clock, LDO mode) |

## Building locally

```bash
west init -l config && west update
west build -s zmk/app -b nice_nano_v2 -- -DSHIELD="byrantk_lp63 nice_view" \
  -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD
```

Every target in both `build.yaml` files was built and checked on 2026-10-28.
