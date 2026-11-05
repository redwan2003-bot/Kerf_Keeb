# ByranTK LP63 — ESB firmware (experimental, lowest latency)

This repo builds the LP63 keyboard and its USB dongle with **Nordic Enhanced ShockBurst (ESB)**
instead of Bluetooth for the keyboard → dongle link, using the community module
[damex/zmk-feature-split-esb](https://github.com/damex/zmk-feature-split-esb) (v0.8.14) on
ZMK main (Zephyr 4.1, pinned commit `753d8b4`).

The regular Bluetooth firmware stays in the other repo (`zmk-config-byrantk_lp63`, ZMK v0.3).
You can always go back to it: flash `settings_reset`, then the BLE files.

## Why ESB

| | Bluetooth split (BLE) | ESB (this repo) |
|---|---|---|
| Keyboard → dongle air latency | waits for the next connection event: 0–7.5 ms, ~3.75 ms average | packet sent the moment the key is scanned, ACK'd within roughly 0.3 ms |
| Dongle → PC | 1 ms USB poll | 1 ms USB poll |
| Retries | next connection event (+7.5 ms each) | hardware auto-retransmit in ~0.25 ms |
| Interference | BLE channel map | adaptive hopping on 2474–2483 MHz (above Wi-Fi ch 11), bad channels masked automatically |
| Pairing | bonding, can get lost | none: both sides share an address compiled in |
| Keyboard to phone/PC directly over BT | yes | **no** (dongle or USB cable only) |

Radio settings (`boards/shields/byrantk_lp63/esb_link.dtsi`), tuned for range + low latency:

| Setting | Value | Why |
|---|---|---|
| Bitrate | 1 Mbps | ~3–4 dB better sensitivity than 2 Mbps (~40–60 % more open-air range); key + ACK still ~0.4 ms on air |
| TX power | +8 dBm, both ends | radio maximum |
| Ramp-up | fast (40 µs) | shorter turnaround on every packet |
| Address / CRC | 5 bytes / 16-bit | best rejection of noise and other 2.4 GHz devices |
| Retries | up to 15, adaptive | clean link uses ~2 (low tail latency); edge of range uses all 15 instead of dropping keys |
| Retry spacing | 600 µs | room for an ACK carrying a full 48-byte payload at 1 Mbps |
| Channels | 74–83 (2474–2483 MHz), hopping | above Wi-Fi ch 11; boot survey + automatic masking of bad channels |

Total keypress → PC latency stays about 1–2 ms (the 1 ms USB poll dominates).
For pure minimum latency at short range, set `bitrate-kbps = <2000>` (saves ~0.15 ms, loses range).

Change `base-address` to 4 random bytes of your own so a second LP63 nearby can never talk to
your dongle. Both sides must be rebuilt after any change in this file.

## Firmware files (GitHub Actions → artifacts)

| Artifact | Flash to |
|---|---|
| `byrantk_lp63_esb` | the keyboard (nice!nano) |
| `kerf_dongle_esb` | the kerf dongle (nRF52833 PCB) |
| `nice_nano_esb_dongle` | a spare nice!nano used as dongle (instead of the kerf dongle) |
| `settings_reset_nice_nano` | keyboard, once, when switching from the BLE firmware |
| `kerf_dongle_settings_reset` | kerf dongle, once, when switching from the BLE firmware |

Order: settings reset on each device → `kerf_dongle_esb` → `byrantk_lp63_esb`. The keyboard
finds the dongle on its own within a second; there is no pairing step.

Flashing is the same as before: double-tap reset (dongle: SW1) → `KERFBOOT` / `NICENANO` drive
appears → drag the `.uf2` on.

## What works

- All 65 keys, the encoder (sensor events are cumulative on air, so a lost packet never loses a
  step), all layers, combos, hold-taps (they run on the dongle)
- Underglow keys on the Fn layer — they use `&rgb_esb` (in this repo), which routes the command
  over ESB to the keyboard. The stock `&rgb_ug` can't do this on an ESB dongle (ESB has no
  "global" broadcast), so keep `&rgb_esb` in the dongle keymap.
- Fn+B bootloader and reset act on the keyboard (key pressed there).
- Keyboard battery level is reported to the dongle (red LED below 15 %).
- nice!view: a new ESB status screen — link state, RSSI (dBm), radio channel, average
  transmissions per packet (`x1.0` = every packet got through first time), battery %.
- kerf dongle RGB LED, same meaning as the BLE firmware:
  - **blue** fast blink = searching, solid 2 s = keyboard linked, slow blink = link lost
  - **green** = Caps Lock
  - **red** short flash every 2 s = keyboard battery ≤ 15 %

## What's different / limits

- No Bluetooth at all in these builds (no direct BT to a phone, no BT profiles). The Fn-layer
  BT/output keys are mapped to nothing here.
- The keyboard still works over its own USB cable only as a charger; typing over the cable
  needs the BLE firmware.
- Experimental module: if something misbehaves, the BLE repo is the fallback.
- Battery life is not measured yet. The keyboard sends a keepalive every 128 ms when idle
  (BLE with these settings wakes about every 225 ms), so expect it to be a bit shorter than BLE.

## Building locally

```sh
west init -l config && west update      # pulls ZMK main, Zephyr 4.1, sdk-nrf v3.1.1, nrfxlib
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-0.17.0
# keyboard
west build -s zmk/app -d build/kb -p -b nice_nano//zmk -S byrantk-lp63-esb -- \
  -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DSHIELD="byrantk_lp63 nice_view"
# kerf dongle
west build -s zmk/app -d build/dg -p -b kerf_dongle -- \
  -DZMK_CONFIG=$PWD/config -DZMK_EXTRA_MODULES=$PWD -DSHIELD=byrantk_lp63_esb_dongle
```

## Repo layout (what's new vs. the BLE repo)

- `config/west.yml` — ZMK main @753d8b4 + damex/zmk-feature-split-esb v0.8.14
- `boards/kerf/kerf_dongle/` — kerf dongle board in the Zephyr 4 board format (`kerf_dongle`)
- `boards/shields/byrantk_lp63/esb_link.dtsi` — shared radio identity (both sides include it)
- `boards/shields/byrantk_lp63/byrantk_lp63_esb_dongle.overlay` + `config/byrantk_lp63_esb_dongle.*`
- `snippets/byrantk-lp63-esb/` — turns the keyboard build into the ESB peripheral
- `src/esb_status_screen.c` — nice!view ESB screen
- `src/behaviors/behavior_rgb_esb.c`, `dts/behaviors/rgb_esb.dtsi` — `&rgb_esb`
- `src/dongle_status_led.c` — LED logic, now works with BLE and ESB
