# Production readiness

As of 2026-11-05, both boards pass every automated check with **0 errors and 0 warnings**. What's left is the final Gerber export and testing on real hardware.

## Final audit

| Check | Keyboard | Dongle |
|---|---|---|
| PCB DRC (strict) | 0 errors | 0 errors |
| Schematic DRC | 0 errors, 0 warnings | 0 errors, 0 warnings |
| Schematic ⇄ PCB connections | identical: 106 nets, 384 pins | identical: 26 nets, 108 pins |
| Parts, footprints and LCSC numbers | 163 parts, no mismatches | 32 parts, no mismatches |
| BOM/CPL check | pass | pass |
| Firmware | all targets build | all targets build |

## Changes made during the audit

| Change | Board |
|---|---|
| USB-C plug: VBUS pins A9, B4 and B9 joined to A4 (all four VBUS contacts now carry power) | Dongle |
| No-connect flags on every unused pin; duplicate net labels removed | Both |
| SWD test pads (TP1–TP5) documented on the schematic | Dongle |
| Silkscreen ID and version (`KERF kerf_keeb LP63 v1.0`, `DONGLE r3`), plus a `PWR` label | Both |
| Encoder push removed: the EC12E1240301 has no push switch, and its D/E pads are the mounting lugs (now both GND, so no ENC_SW short); D18 unused | Keyboard |
| Dead copper removed: BAT+ and RST stubs left from moved parts, a GND stub and an orphan via | Keyboard |
| Schematic notes corrected (65 keys, D20 active high, LED rail from B+) | Keyboard |
| Spare/backup PCBs renamed `ARCHIVE_DO_NOT_ORDER_…` | Project |
| Top-side battery LED (LED9) removed from the schematic, PCB, firmware and plate; the LED_VCC trunk to LED7 kept, LED8 DOUT marked no-connect | Keyboard |

## Still to do before ordering

- [ ] Save a project version (v1.0) in EasyEDA Pro.
- [x] Gerbers exported for both boards on 2026-11-05 (still to be copied into `hardware/`).
- [ ] Check the stock of the USB4155-03-C plug and the 32 MHz crystal.
- [ ] Order the dongle as 4-layer JLC04161H-7628, ENIG, epoxy-filled vias.

## First-article test (on 1 of the first 5 boards)

- [ ] Battery polarity, sleep current, charging.
- [ ] Dongle: SWD bootloader flash, USB enumeration, 3.3 V rail.
- [ ] All 65 keys, the knob (turn), the nice!view and the underglow.
- [ ] BLE and 2.4 GHz range (C20 is spare for antenna tuning).
- [ ] Slide switch: confirm which position is ON, then add an ON arrow to the silkscreen in the next revision.
- [ ] Plate fit.

## What's next

- **Case and keycaps:** a parametric 3D-printable case, split to fit the Bambu A1 bed (tray mount, dovetail joint, 6° typing angle), plus printed KS-33 low-profile keycaps.
- **Kerf Hub:** a web/desktop configurator for remapping, macros, lighting, profiles and the display editor. See [software/KERF_HUB_SOFTWARE_PLAN.md](software/KERF_HUB_SOFTWARE_PLAN.md).
- **More Kerf devices:** a macro pad and a mouse.
