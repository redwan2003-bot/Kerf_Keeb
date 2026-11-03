# Kerf Hub — Software Plan

> Configurator software for the **kerf_keeb (ByranTK LP63)** wireless keyboard and the future Kerf macro pad and mouse.
> Owner: Redwan · Plan date: 2026-10-22 · Status: approved plan, not started.

Kerf Hub is a web app plus a thin desktop wrapper. It speaks **ZMK Studio's protocol** for the keymap and a small
**Kerf protocol** for everything else: lighting, battery LED, macros, profiles, display, power and updates. The
keyboard already runs ZMK, so about half of what commercial hubs (Razer Synapse, Logitech G HUB, MCHOSE M HUB) do is
free. This plan builds the other half in about **19 weeks of part-time work (10–15 h/week)**.

---

## 1. Hardware and firmware you are building for

### 1.1 kerf_keeb (LP63) keyboard

| Item | Value |
|---|---|
| Layout | 65 keys, column-aligned grid 13 × 5, 2U space, arrow cluster bottom-right, 1 EC12 encoder (no push) |
| Controller | nice!nano v2, or a pin-compatible Pro Micro nRF52840 clone (ZMK board `nice_nano_v2` / `nice_nano//zmk`) |
| Matrix | 8 rows × 9 columns, `diode-direction = "row2col"` |
| Rows | One 74HC595 shift register on SPI0 (`shifter0`), row-gpios order: shifter outputs `0, 2, 4, 5, 7, 6, 1, 3` |
| Columns | Pro Micro pins `D8, D9, D2, D3, D4, D5, D6, D7, D21` (D21 = P0.31 = screenshot key column) |
| Encoder | A = D0 (P0.08), B = D1 (P0.06); UART disabled |
| Display | nice!view, 160 × 68 px, 1-bit Sharp memory LCD (LS011B7DH03), < 10 µA, 30 Hz, SPI0 CS = D15 |
| 74HC595 latch | SPI0 CS = D14 |
| Underglow | 8 × WS2812B (LED1–LED8) + LED9 battery indicator (WS2812B-Mini) = 9-pixel chain on SPI1 MOSI = D19 (P0.02) |
| LED power | LED rail switched by P-MOSFET Q1 via EXT_POWER on D20 (P0.29), active high; LEDs draw 0 mA when off |
| VCC for 595 + nice!view | nice!nano P0.13 held high by a GPIO hog (`vcc_always_on`) |
| Battery | LiPo on JST-PH J2 (pin 1 = BAT+), slide switch SW66, reset button SW67 |
| Special keys | Fn+Esc = battery check on LED9 (`&batled`); bottom-row "Snip" key = Win+Shift+S, Fn+Snip = Print Screen |

### 1.2 Dongles

| Dongle | Chip | Link to keyboard | Firmware target |
|---|---|---|---|
| nice!nano as dongle | nRF52840 | Bluetooth LE split (keyboard = peripheral) | shield `byrantk_lp63_dongle` |
| kerf_dongle (own PCB) | nRF52833 QFN-40, RGB status LED on P0.29–P0.31 (active low) | BLE split, or ESB 2.4 GHz | board `kerf_dongle`, shield `byrantk_lp63_dongle` or `byrantk_lp63_esb_dongle` |

With a dongle, ZMK runs the **keymap on the dongle** (central). Lighting, LED9, battery and the display live on the **keyboard**
(peripheral). The ZMK split link only forwards behavior invocations (key presses), not arbitrary settings.

### 1.3 Firmware repositories (existing)

| Repo | ZMK version | Zephyr | Notes |
|---|---|---|---|
| `zmk-config-byrantk_lp63` | v0.3 (`west.yml` default revision `v0.3`) | 3.5 | BLE: standalone, peripheral, BLE dongle, kerf_dongle |
| `zmk-config-esb` | zmk `753d8b467125fe860eec05ee71cb7ebefa08fba6` + `damex/zmk-feature-split-esb` v0.8.14 | 4.1 | ESB 2.4 GHz split; adds `&rgb_esb` behavior and an ESB status screen |

Existing custom code in both repos (keep it working):

- `src/kerf_led_split.c` — `kerf,led-strip-split` driver: underglow sees 8 pixels, the real chain has 9; pixel 9 = LED9.
- `src/kerf_batled.c` + `src/behaviors/behavior_batled.c` — `&batled` (event-source locality): tap = 1 s, hold = until release (max 10 s); green > 50 %, yellow 20–50 %, red < 20 %; low-battery blink ≤ 10 % at most every 5 min. Kconfig: `KERF_BATLED_*`.
- `src/dongle_status_led.c` — kerf_dongle RGB status LED.
- `boards/shields/byrantk_lp63/*` — shield, matrix transform (`columns = <9>`, `rows = <8>`), physical layout, keymaps.

---

## 2. What the market does (research summary)

| App | Runs as | Strongest ideas to copy |
|---|---|---|
| Razer Synapse 4 | Windows app | Hypershift (second layer on one key), Snap Tap, macro speed, 1-step DPI, sensitivity matcher, lift-off/landing distance |
| Logitech G HUB | Win/macOS app | Profiles that auto-switch per game, onboard-memory mode, community profiles, firmware auto-update |
| Corsair iCUE | Windows app | Device Memory Mode, Murals (lighting from image/video/audio/screen), surface calibration, sensor dashboard |
| SteelSeries GG | Win/macOS app | Per-key actuation, two functions at two press depths, macro editor, onboard profiles, OLED image upload |
| Wooting Wootility | Browser + app | Per-key actuation, Rapid Trigger, DKS, Snappy Tappy, 3 onboard profiles, RGB layers |
| MCHOSE M HUB | Browser + Windows | One app for keyboard, mouse and audio; onboard profiles; global RGB sync; polling 125/500/1000 Hz |
| ATK / VXE V HUB | Windows app | Mouse: ≤ 8 DPI stages, 8K polling, LOD 0.7–1.2 mm, motion sync, ripple control, debounce, sleep time, receiver pairing, stable/beta firmware |
| Keychron Launcher | Browser | No install, QMK remap, macros, online firmware update |
| RK web driver / Ajazz driver | Browser / Windows | GIF upload to the keyboard's colour screen (USB only) |
| ASUS Armoury Crate | Windows (+ Gear Link web) | OLED animations, custom images, PC temperature/load/time; short vs long press binds |
| Vial | Open source | Live keymap, macros, tap dance, combos, key overrides, firmware settings, matrix tester, unlock |
| ZMK Studio | Open source, browser + desktop | Live keymap/layers over USB or BLE for ZMK boards; no macros or lighting yet |

Not applicable to kerf_keeb: per-key actuation, Rapid Trigger, DKS (they need Hall-effect switches; kerf_keeb uses mechanical Gateron KS-33).

---

## 3. Feature list (scope)

Release tags: **v1.0** = parity with the big hubs for this hardware · **v1.1** = display editor and extras · **Later** = macro pad / mouse / nice-to-have.

| Area | Feature | Release | Devices |
|---|---|---|---|
| Connect | Find devices over USB, BLE and both dongles; show model, firmware version, protocol version, link type | v1.0 | All |
| Connect | Unlock before editing (ZMK Studio unlock key, e.g. Fn+Bksp); relock on exit | v1.0 | All |
| Keymap | Live remap of all keys and the encoder, drawn from the physical layout the device reports | v1.0 | Keyboard, macro pad |
| Keymap | Layers: add, rename, reorder, momentary/toggle layer keys | v1.0 | Keyboard, macro pad |
| Keymap | Hold-tap, mod-tap, layer-tap | v1.0 | Keyboard, macro pad |
| Keymap | Combos, tap dance, key overrides at runtime | v1.1 | Keyboard, macro pad |
| Keymap | Snap Tap (SOCD) on chosen key pairs | v1.1 | Keyboard |
| Keymap | Key tester | v1.0 | Keyboard, macro pad |
| Macros | Editor: press / release / tap / delay / text / repeat | v1.0 | All |
| Macros | Recorder (captures keystrokes and timing in the browser) | v1.0 | All |
| Macros | 16 slots × 64 steps stored on device, bound as `&kmacro N` | v1.0 | All |
| Lighting | Underglow on/off, effect, HSB colour, brightness, speed | v1.0 | Keyboard |
| Lighting | Auto-off on USB unplug, sleep timeout | v1.0 | Keyboard |
| Lighting | LED9 battery indicator: brightness (normal + low), show time, low-battery %, blink interval | v1.0 | Keyboard |
| Lighting | One colour synced across all connected Kerf devices | v1.1 | All |
| Lighting | Screen/music reactive lighting (desktop app) | Later | Keyboard, macro pad |
| Display | Widget layout editor: battery, BT profile, layer, caps lock, WPM, output, clock | v1.1 | Keyboard |
| Display | Image upload: crop to 160 × 68, dither to 1-bit, live preview, save | v1.1 | Keyboard |
| Display | GIF animation, ≤ 30 frames, frame rate capped on battery | v1.1 | Keyboard |
| Display | Per-layer images, idle/logo screen | v1.1 | Keyboard |
| Display | Live PC data: clock, CPU/GPU temp, volume, now playing (desktop app running) | v1.1 | Keyboard |
| Display | Invert, rotate 180°, screen off on idle | v1.1 | Keyboard |
| Performance | Debounce press/release ms | v1.0 | All |
| Performance | Idle and deep-sleep timeouts | v1.0 | All |
| Profiles | 3 onboard profiles (keymap + lighting + macros + display), switch by key or app | v1.0 | All |
| Profiles | Export / import profile file (`.kerfprofile`, JSON) | v1.0 | All |
| Profiles | Auto-switch when an app/game starts (desktop app) | v1.1 | All |
| Profiles | Community sharing | Later | All |
| Power | Battery %, charging state, estimated hours left | v1.0 | All |
| Power | BT profiles: name, clear, select; dongle pairing | v1.0 | All |
| Updates | One-click firmware update (reboot to bootloader, write UF2) | v1.0 | All |
| Updates | Stable / beta channels, factory reset | v1.1 | All |
| App | Web app (Chrome, Edge, Opera) + desktop app (Windows, macOS) | v1.0 | — |
| App | Dark/light theme, English + Bangla, in-app help | v1.1 | — |
| Mouse | Button remap, macros, DPI stages (≤ 8), DPI step, polling, LOD, motion sync, ripple control, angle snapping, scroll, sleep | Later | Mouse |
| Macro pad | Per-key RGB editor, encoder actions per layer | Later | Macro pad |

---

## 4. Architecture

```
┌──────────────────────── Kerf Hub ────────────────────────┐
│  apps/web (Chrome/Edge/Opera)   apps/desktop (Tauri 2)   │
│  ─────────────── packages/ui (React) ───────────────────  │
│  packages/core: device sessions, capability routing      │
│   ├─ ZMK Studio client (keymap, layers, behaviors)       │
│   └─ @kerf/protocol (lighting, macros, profiles, display,│
│        power, updates, device info)                       │
└───────────────┬──────────────────────────┬───────────────┘
        USB     │                          │  Bluetooth LE
 Studio: CDC-ACM serial            Studio: GATT service
 Kerf:   vendor raw HID            Kerf:   custom GATT service
                │                          │
   ┌────────────┴───────┐     ┌────────────┴──────────┐
   │ Dongle (BLE / ESB) │────▶│ kerf_keeb LP63        │   Macro pad / mouse (later)
   │ keymap, macros,    │split│ lighting, LED9,       │   same zmk-kerf module
   │ profiles           │link │ battery, display      │
   └────────────────────┘     └───────────────────────┘
        zmk-kerf firmware module runs on every device
```

### 4.1 Transports

| Channel | USB | BLE | Framing |
|---|---|---|---|
| ZMK Studio | CDC-ACM serial (snippet `studio-rpc-usb-uart`) | Studio GATT service | SOF `0xAB`, ESC `0xAC`, EOF `0xAD`; protobuf `Request` / `Response` / `Notification` |
| Kerf | Vendor raw HID interface, 64-byte reports (usage page `0xFF60`, usage `0x61` style) chunked | Custom GATT service, one characteristic (write + indicate) | Same SOF/ESC/EOF framing; protobuf `kerf.Request` / `kerf.Response` / `kerf.Notification` |

Reason for a separate Kerf channel: ZMK's public docs describe Studio's framing and messages but not third-party
subsystems. If ZMK adds supported custom Studio subsystems, move Kerf messages there and keep the same `.proto` files.

### 4.2 Kerf protocol (first draft of `kerf.proto`)

```proto
syntax = "proto3";
package kerf;

message Request {
  uint32 request_id = 1;
  oneof subsystem {
    core.Request     core     = 10;  // device info, lock state, reboot to bootloader, factory reset
    lighting.Request lighting = 11;  // underglow state + LED9 battery indicator config
    macros.Request   macros   = 12;  // list/get/set/clear macro slots
    profiles.Request profiles = 13;  // list, activate, save-as, export blob, import blob
    display.Request  display  = 14;  // layout, images (chunked), animation, live data push
    power.Request    power    = 15;  // debounce, idle/sleep timeouts, battery, BT profiles
    pointer.Request  pointer  = 16;  // mouse only (later): DPI stages, polling, LOD, filters
  }
}
message Response     { uint32 request_id = 1; oneof subsystem { /* mirrors Request */ } ErrorCode error = 99; }
message Notification { oneof subsystem { /* battery_changed, profile_changed, lock_changed, layer_changed */ } }

enum ErrorCode { OK = 0; LOCKED = 1; UNSUPPORTED = 2; INVALID = 3; NO_SPACE = 4; BUSY = 5; }
```

`core.GetDeviceInfoResponse` must include: `model_id`, `usb_vid`, `usb_pid`, `fw_version`, `protocol_version`,
`capabilities` (bitmap: KEYMAP, LIGHTING_UNDERGLOW, BATLED, MACROS, PROFILES, DISPLAY_NICEVIEW, POINTER, ENCODER),
`role` (STANDALONE, CENTRAL_DONGLE, PERIPHERAL), `battery_percent`, `charging`, `link` (USB, BLE, ESB).

Encoding: `ts-proto` on the host, `nanopb` in firmware (same `.proto` files from `kerf-protocol`).

### 4.3 Firmware storage (Zephyr settings)

| Key | Content |
|---|---|
| `kerf/ver` | settings schema version |
| `kerf/light` | underglow config, auto-off rules |
| `kerf/batled` | LED9 brightness, low brightness, show/max ms, low %, repeat minutes (replaces the Kconfig defaults at runtime) |
| `kerf/power` | debounce press/release ms, idle/sleep timeouts |
| `kerf/macro/<n>` | macro slot n (≤ 64 steps) |
| `kerf/prof/<n>` | profile n snapshot |
| `kerf/disp/layout`, `kerf/disp/img/<n>`, `kerf/disp/anim` | display layout, 1-bit images (1,360 B per 160 × 68 frame), animation table |

Writes are debounced (ZMK already delays settings saves) to limit flash wear.

### 4.4 Dongle rule (v1)

Keymap, macros and profiles are edited on the device the app is connected to (the dongle when one is used). Keyboard-side
settings (lighting, LED9, display, keyboard battery) need the keyboard connected by USB or BLE directly. Through a
dongle, v1 only sends lighting *commands* as behavior invocations (`&rgb_ug` on BLE, `&rgb_esb` on ESB). A custom split
message for full settings relay is a later task.

---

## 5. Repositories and layout

```
zmk-kerf/                      # ZMK module, added to every device's west.yml
  zephyr/module.yml
  Kconfig, CMakeLists.txt
  dts/bindings/...             # kerf,kmacro behavior, display, transport nodes
  include/kerf/*.h
  src/transport/usb_hid.c      # vendor raw HID
  src/transport/ble_gatt.c     # custom GATT service
  src/rpc/{core,lighting,macros,profiles,display,power}.c
  src/behaviors/behavior_kmacro.c
  src/display/kerf_status_screen.c   # custom ZMK status screen for nice!view
  proto/ -> git submodule of kerf-protocol

kerf-protocol/
  proto/kerf/*.proto
  ts/   (npm package @kerf/protocol, generated by ts-proto)
  c/    (nanopb options + generated sources)

kerf-hub/                      # pnpm + Turborepo monorepo
  apps/web/                    # Vite + React + TypeScript + Tailwind
  apps/desktop/                # Tauri 2 (Rust: hidapi, serialport, btleplug)
  packages/core/               # sessions, transports (WebHID, WebSerial, Web Bluetooth), Studio client wrapper
  packages/ui/                 # keyboard renderer, editors (keymap, macro, lighting, display)
  packages/image/              # crop + dither (Floyd–Steinberg / Atkinson) to 160×68 1-bit, GIF frame extraction
  devices/*.json               # one descriptor per product (see 5.1)

zmk-config-byrantk_lp63/, zmk-config-esb/   # existing; add zmk-kerf to west.yml
```

### 5.1 Device descriptor (`devices/kerf-lp63.json`)

```json
{
  "modelId": "kerf-lp63",
  "name": "kerf_keeb LP63",
  "usb": { "vid": "0x1D50", "pid": "0x615E" },
  "capabilities": ["KEYMAP", "ENCODER", "LIGHTING_UNDERGLOW", "BATLED", "MACROS", "PROFILES", "DISPLAY_NICEVIEW"],
  "display": { "type": "nice!view", "width": 160, "height": 68, "bpp": 1 },
  "lighting": { "underglowPixels": 8, "batledPixel": 9 },
  "firmware": { "bootloader": "adafruit-uf2", "uf2Family": "0xADA52840" }
}
```

USB VID/PID above are placeholders: ZMK's default IDs are used until the project gets its own (for example from pid.codes).
Rule: the app routes pages by `capabilities`, never by product name.

---

## 6. Technology choices

| Layer | Choice |
|---|---|
| Keymap protocol | ZMK's official TypeScript Studio client (`@zmkfirmware/zmk-studio-ts-client`), version pinned to the shipped ZMK |
| Kerf protocol | protobuf, ts-proto (host) + nanopb (firmware) |
| UI | React 18, Vite, TypeScript strict, Tailwind, Zustand for state, React Query for device calls |
| Web transports | WebSerial (Studio USB), WebHID (Kerf USB), Web Bluetooth (both BLE services) — Chromium only |
| Desktop | Tauri 2; Rust plugins: `hidapi`, `serialport`, `btleplug`; same UI bundle |
| Firmware update | Web: File System Access API writes the UF2 to the bootloader drive; desktop: copies directly |
| Hosting | Cloudflare Pages or GitHub Pages (static) |
| Tests | Vitest (TS), Playwright (UI), Rust unit tests, Zephyr `native_sim` / twister for firmware logic |
| CI | GitHub Actions: build all firmware targets (BLE + ESB), web build, desktop builds (Windows + macOS) |

---

## 7. Roadmap (≈ 19 weeks part-time)

| # | Weeks | Phase | Deliverables | Done when |
|---|---|---|---|---|
| 1 | 1–2 | Studio on kerf_keeb | Move BLE and ESB configs to **one** ZMK version; `CONFIG_ZMK_STUDIO=y`, `studio-rpc-usb-uart` snippet, `&studio_unlock` binding; test with ZMK's own Studio app | A remap survives a power cycle, direct and through each dongle |
| 2 | 3–5 | Kerf Hub shell | Monorepo, connect flow, keyboard drawn from physical layout, remap + layers via Studio client, branding | It replaces Studio for daily use |
| 3 | 6–9 | Kerf protocol + settings | `zmk-kerf` transports (USB HID + BLE GATT), core/lighting/batled/power services, app pages | Every lighting/LED9/debounce setting round-trips and survives reboot |
| 4 | 10–12 | Macros | `&kmacro`, slot editor, recorder, text macros | A 64-step macro plays correctly over USB, BLE and both dongles |
| 5 | 13–14 | Profiles + updates | 3 profile slots, export/import, one-click UF2 update | A failed update is always recoverable by double-tapping reset |
| 6 | 15–16 | Desktop + v1.0 | Tauri builds (Windows installer, macOS), error reporting, user guide, GitHub release | v1.0 tagged and published with the OSHWLab kerf_keeb page |
| 7 | 17–19 | v1.1 display editor | Custom nice!view status screen, `display` service, layout editor, image/GIF upload, per-layer + idle screens, live PC data | An uploaded image survives a power cycle and idle current stays < 10 µA for the display |
| — | Later | Family | Macro pad descriptor + per-key RGB page; mouse `pointer` service + DPI page; colour sync; community profiles | — |

---

## 8. Risks and mitigations

| Risk | Effect | Mitigation |
|---|---|---|
| Two ZMK versions (v0.3 vs newer main) | Module breaks on one line | Phase 1 aligns both to one release before any module work |
| Split link can't carry settings | Limited lighting control through a dongle | v1 commands only; full settings over direct link; custom split message later |
| Web APIs are Chromium-only | No Firefox/Safari | Desktop app covers them |
| Studio protocol changes upstream | Remap breaks after a ZMK update | Pin the Studio client to the shipped ZMK version; protocol version check in device info |
| Flash wear | Corruption over years | Debounced saves, small records, schema version key |
| BLE transfer speed | Slow image upload | Images over USB first; BLE uploads chunked with progress and resume |
| Licences | Can't copy code freely | ZMK is MIT; check the Studio app/client licences before reusing UI code; follow ZMK's clean-room note for protocol work |
| Clone controllers (Pro Micro nRF52840) | Battery reads 100 % while charging (VDDH), batch-dependent sleep current | Show "charging" instead of % on USB; document known clone quirks |

---

## 9. Open decisions (owner: Redwan)

- [ ] Product name: "Kerf Hub" or another
- [ ] Web first (recommended) or desktop first
- [ ] ESB dongle path in v1, or BLE + USB only
- [ ] Open source the app with kerf_keeb (fits the OSHWLab plan) or keep it closed
- [ ] Own USB VID/PID (pid.codes) before v1.0

---

## 10. Sources

- MCHOSE M HUB — https://support.mchose.store/hc/en-us/articles/49078340935700-Getting-Started-with-MCHOSE-M-HUB
- ZMK Studio RPC protocol — https://zmk.dev/docs/development/studio-rpc-protocol
- ZMK Studio announcement — https://zmk.dev/blog/2024/11/11/zmk-studio-mvp-ga
- ZMK displays — https://zmk.dev/docs/config/displays
- ZMK dongle display module — https://github.com/englmaxi/zmk-dongle-display
- nice!view FAQ — https://docs.splitkb.com/product-guides/nice-view/frequently-asked-questions
- Razer Synapse 4 — https://www.razer.com/newsroom/product-news/introducing-razer-synapse-4
- Logitech G HUB — https://prosettings.net/blog/what-is-logitech-g-hub/
- Corsair iCUE — https://www.corsair.com/explorer/diy-builder/blogs/corsair-icue-everything-you-need-to-know/
- SteelSeries GG (Apex Pro Gen 3) — https://www.igorslab.de/en/steelseries-apex-pro-gen-3-gaming-keyboard-in-detail-hall-effect-technology-and-rapid-trigger-for-uncompromising-control/3/
- Wootility — https://prosettings.net/blog/what-is-wootility/
- ATK V HUB — https://www.atk.store/en-de/blogs/news/atk-v-hub-instructions
- Keychron Launcher — https://keychron.in/blogs/news/advantages-of-the-keychron-launcher-web-app
- RK GIF upload — https://rkgamingstore.com/it/blogs/community/how-to-upload-gifs-to-your-keyboard
- Ajazz AK820 Pro screen — https://www.whatgeek.com/es/blogs/news/how-to-change-the-display-gif-of-the-ajazz-ak820-pro-keyboard?page=1
- ASUS ROG Azoth X — https://www.igorslab.de/en/asus-rog-azoth-x-gaming-keyboard-review-a-stylish-powerhouse-with-many-features/3
- Vial — https://docs.splitkb.com/resources/firmware/vial/usage-guide
- Mouse sensor features — https://www.corsair.com/explorer/gamer/mice/understanding-advanced-mouse-sensor-features-motion-sync-lod-angle-snapping-and-ripple-control/
- Pro Micro nRF52840 clone notes — https://github.com/joric/nrfmicro/wiki/ALternatives
