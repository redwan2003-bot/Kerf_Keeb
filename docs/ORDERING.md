# Ordering the PCBs from JLCPCB

## Files to upload

| Board | Gerber | BOM | CPL |
|---|---|---|---|
| Keyboard | export from `LP63_PCB` in EasyEDA Pro | `hardware/keyboard/kerf_keeb_BOM_JLC.csv` | `hardware/keyboard/kerf_keeb_CPL_JLC.csv` |
| Dongle | export from `Dongle_PCB` | `hardware/dongle/kerf_dongle_BOM_JLC.csv` | `hardware/dongle/kerf_dongle_CPL_JLC.csv` |

Use the `_JLC` BOM/CPL files from this repo, not EasyEDA's own BOM export. Never export from a PCB named `ARCHIVE_DO_NOT_ORDER_…`.

## Keyboard PCB

- 2 layers, 272.8 × 97.3 mm, 1.6 mm, HASL lead-free (ENIG optional). Any mask colour.
- **PCB Assembly: bottom side only, 95 parts.** These are the diodes, 8 × WS2812B, the 74HC595, the MOSFETs, capacitors, resistors, the JST battery connector, the slide switch and the reset button.
- In "Confirm parts placement", check the orientation of the diodes, the 5050 LEDs (pin-1 corner), the SOT-23 MOSFETs and the 74HC595.
- **You solder these yourself:**
  - 65 × Gateron KS-33 hot-swap sockets
  - the nice!nano v2 (U1), with the nice!view on its header (J1)
  - the EC12 encoder (SW65)
  - LED9: WS2812B-Mini 3535, LCSC C114583, top side right of ↑

## Dongle PCB

- **4 layers**, 18.0 × 39.4 mm, 1.6 mm, stack-up **JLC04161H-7628** (the antenna matching depends on it).
- Vias: **epoxy-filled and capped**. Surface finish: **ENIG** (0.4 mm pitch QFN).
- **PCB Assembly: top side, 31 parts.** C20 is intentionally not fitted; it's a spare for antenna tuning.
- Check stock when you order: J1 (USB4155-03-C) and X1 (NX2016SA 32 MHz) have been low at LCSC.
- If JLC lists J1 as a through-hole part (it has shell posts), accept the THT option.
- In the placement preview, check the U1 pin-1 dot, the D1 RGB LED, X1 and U2.

## After you pay

JLC sends **Confirm Production** (Gerbers after their CAM edits) and **Confirm Parts Placement** emails. Placement is approved automatically after 72 hours if you don't respond. Before you confirm, compare their files against what you uploaded, and check every polarity.

## After the boards arrive

1. Check the battery polarity at J2 (pin 1 = BAT+) against your LiPo's JST-PH lead.
2. Flash the firmware as described in [../firmware/README.md](../firmware/README.md).
3. Print the plate fit coupon before the full plate.
