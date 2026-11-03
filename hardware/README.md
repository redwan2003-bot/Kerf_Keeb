# kerf_keeb hardware

Both boards were designed in **EasyEDA Pro** (project `kerf_keeb`, boards `LP63_Keyboard` and `Kerf_Dongle`). This folder holds the manufacturing data that is ready now:

| File | What it is |
|---|---|
| `keyboard/kerf_keeb_BOM_JLC.csv` | Bill of materials for JLCPCB assembly (LCSC part numbers) |
| `keyboard/kerf_keeb_CPL_JLC.csv` | Component placement list (bottom side, 95 parts) |
| `dongle/kerf_dongle_BOM_JLC.csv` | Dongle BOM |
| `dongle/kerf_dongle_CPL_JLC.csv` | Dongle placement list (top side, 31 parts; C20 is left unfitted on purpose) |
| `artwork/*.svg` | kerf_keeb logo and silkscreen artwork |

> **Gerbers:** the final Gerber files will be added once they're exported from the current revision. Copper and silkscreen changed on 2026-10-28, so older exports are out of date. The EasyEDA project will also be published on OSHWLab.

---

## Keyboard PCB (`LP63_Keyboard`)

| | |
|---|---|
| Size | 272.8 × 97.3 mm, 2 layers, 1.6 mm, rounded corners |
| Switches | 65 × Gateron KS-33 (Low Profile 2.0) on KS-33 hot-swap sockets, 19.05 mm pitch |
| Matrix | 8 rows × 9 columns, `row2col`, one 1N4148W diode per key |
| Row driver | 74HC595 (SOIC-16) on SPI0, so the 8 rows use only 3 controller pins |
| Controller | nice!nano v2 or a compatible "Pro Micro nRF52840" (U1), mounted on the bottom |
| Display | nice!view on a 5-pin header (J1), sharing SPI0 with the 74HC595 |
| Encoder | Alps EC12E1240301 (SW65): A/B for rotation, pin D for the push switch |
| Underglow | 8 × WS2812B-B/W 5050 (LED1–LED8, bottom) |
| Battery LED | LED9: WS2812B-Mini 3535, top side right of ↑, hand-soldered, last in the LED chain |
| Battery | JST-PH 2-pin SMD connector (J2): pin 1 = BAT+, pin 2 = GND |
| Switches | SW66: slide power switch; SW67: reset button (bottom side) |
| Mounting | 8 × M2 holes for the plate and case |

### Pin map (nice!nano v2 names)

| Function | Pin | Net |
|---|---|---|
| Columns 1–8 | D8, D9, D2, D3, D4, D5, D6, D7 | COL_D… |
| Column 9 (Snip key) | D21 / P0.31 | COL_D10 |
| SPI0 SCK / MOSI (74HC595 + nice!view) | D10 / D16 | SPI_SCK / SPI_MOSI |
| 74HC595 latch | D14 | SR_LATCH |
| nice!view CS | D15 | VIEW_CS |
| WS2812 data (SPI1 MOSI) | D19 / P0.02 | LED_DATA |
| LED power enable | D20 / P0.29 | LED_PWR_EN |
| Encoder A / B | D0 / D1 | ENC_A / ENC_B |
| Encoder push | D18 / P1.15 (active low, internal pull-up) | ENC_SW |
| SPI1 SCK (unused clock) | P1.07 back pad, not connected | — |

### Power design

- **Battery switch:** SW66 pulls the gate of Q3 (AO3401A P-MOSFET) to GND to connect BAT+ to the controller's B+ input. R4 (100k) holds it off otherwise.
- **LED power switch:** D20 drives Q2 (AO3400A), which turns on Q1 (AO3401A) to feed LED_VCC from B+. R2 and R3 keep the LEDs off during boot. When the underglow is off, the LEDs draw nothing.
- **VCC rail:** the controller's switched VCC (P0.13) is held on by firmware, because it powers the 74HC595 and the nice!view.
- **Underglow on battery:** it only runs while USB is plugged in (`ZMK_RGB_UNDERGLOW_AUTO_OFF_USB`). Below about 3.7 V the WS2812B-B/W is out of spec.

### Verification (2026-10-28)

| Check | Result |
|---|---|
| PCB DRC (strict) | 0 errors |
| Schematic DRC | 0 errors, 0 warnings |
| Schematic ⇄ PCB | identical: 108 nets, 389 pins, 164 parts, footprints and LCSC numbers match |
| Matrix | all 65 keys traced from PCB net → firmware transform → physical position |
| BOM ⇄ CPL ⇄ PCB | designators and positions cross-checked |

---

## Dongle PCB (`Kerf_Dongle`, rev r3)

| | |
|---|---|
| Size | 18.0 × 39.4 mm, 4 layers (JLC04161H-7628; the antenna match assumes this stack-up) |
| SoC | nRF52833-QDAA (QFN-40) |
| USB | USB-C **plug** (GCT USB4155-03-C) soldered directly to the board; all four VBUS pins connected; 5.1 kΩ CC pull-down; USBLC6-2SC6 ESD protection |
| Clock | 32 MHz crystal. No 32 kHz crystal, so the firmware uses the internal RC oscillator |
| Power | USB VBUS into VDDH; internal LDO, with REG0 set to 3.3 V by firmware. No DC-DC inductor fitted |
| Antenna | PCB meander antenna with an LC matching network (L1/L2/C3/C4/C5) and a spare tuning shunt (C20, not fitted) |
| Status LED | RGB LED (D1): blue = link state, green = Caps Lock, red = keyboard battery low |
| Programming | SWD test pads on the bottom: TP_V (3.3 V), TP_G, TP_D (SWDIO), TP_C (SWCLK), TP_R (reset) |
| Button | SW1 = reset; double-tap to enter the UF2 bootloader |

**Order options:** 4-layer, ENIG finish, epoxy-filled and capped vias (vias sit in the QFN thermal pad).

### Verification

PCB DRC 0. Schematic DRC 0 errors and 0 warnings. Schematic ⇄ PCB identical (26 nets, 108 pins, 32 parts).
