# Forza ESP32 RPM LEDs

ESP32 shift-light firmware for Forza Motorsport and Forza Horizon telemetry. The firmware drives a WS2812/NeoPixel-style LED strip on GPIO27, listens for Forza UDP telemetry over Wi-Fi, and exposes a small web dashboard for live status and runtime settings.

It also supports a common `0.96 inch` I2C OLED display (`128x64`, SSD1306, address `0x3C`) for IP address, waiting status, and live RPM display.

## Behavior

The firmware normalizes RPM with:

```txt
rpmPercent = (currentEngineRpm - engineIdleRpm) / (engineMaxRpm - engineIdleRpm)
```

The value is clamped from `0.0` to `1.0`, then mapped to a single strip color:

| RPM range | Strip color |
|---:|---|
| `0-30%` | Blue |
| `30-65%` | Green |
| `65-80%` | Yellow |
| `80-100%` | Red |
| `>= 85%` | Active RPM pattern blinks |

The dashboard and physical mode buttons can switch between LED RPM modes:

| Mode | Behavior |
|---|---|
| `Fill` | LEDs fill from the start of the strip. Lit LEDs all use the current RPM color. |
| `Solid` | The whole strip uses the current RPM color, with brightness rising with RPM. |
| `Center` | LEDs fill outward from the center. Lit LEDs all use the current RPM color. |

The strip turns off when `isRaceOn` is false or when no telemetry packet has been received for more than `1000 ms`.

The default thresholds are configurable from the web dashboard and are saved on the ESP32.

## Hardware

- ESP32 DevKit-style board
- WS2812/NeoPixel-style addressable LED strip
- Optional `0.96 inch` I2C OLED, `128x64`, SSD1306, usually address `0x3C`
- Optional two momentary buttons for RPM mode selection
- Breadboard and jumper wires
- USB cable

Default LED strip wiring:

| LED Strip Pin | ESP32 Pin |
|---|---|
| DIN/Data | GPIO27 |
| GND | `GND` |
| 5V/VCC | External 5V supply or suitable 5V source |

Default OLED wiring:

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | `3V3` |
| GND | `GND` |
| SDA | GPIO21 |
| SCL | GPIO22 |

Default mode button wiring:

| Button | ESP32 Pin | Other Side |
|---|---|---|
| Previous RPM mode | GPIO16 | `GND` |
| Next RPM mode | GPIO18 | `GND` |
| Alternate next RPM mode | GPIO19 | `GND` |

The buttons use ESP32 internal pullups, so no external resistor is required for basic momentary push buttons. An unpressed or disconnected button reads as `HIGH`; pressing the button connects the GPIO to `GND` and reads as `LOW`.

Connect ESP32 GND and LED strip power-supply GND together. Keep brightness conservative unless the strip has a power supply sized for the number of pixels.

See [docs/wiring.md](docs/wiring.md) for more detail.

## Wi-Fi Setup

Create your local Wi-Fi config, then edit it:

```sh
cp include/WifiConfig.example.h include/WifiConfig.h
```

```cpp
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

Use a 2.4 GHz Wi-Fi network. Many ESP32 boards cannot connect to 5 GHz-only networks.

## Build And Upload

Install PlatformIO, then run:

```sh
pio run -e esp32dev
pio run -e esp32dev --target upload
pio device monitor -e esp32dev
```

The serial monitor runs at `115200` baud. After upload, the ESP32 runs a short LED startup test, connects to Wi-Fi, prints its IP address and dashboard URL, and starts listening on UDP port `5300` unless a different saved port is configured.

If the OLED is connected, it also shows the ESP32 IP address and UDP port after Wi-Fi connects. If the OLED is not found at address `0x3C`, the firmware prints a serial warning and continues with LEDs only.

Expected serial output:

```txt
Wi-Fi connected.
ESP32 IP address: 192.168.x.x
Dashboard: http://192.168.x.x
Listening for Forza telemetry on UDP port 5300
```

To upload the standalone LED strip wiring test instead of the telemetry firmware:

```sh
pio run -e led_strip_test --target upload
pio device monitor -e led_strip_test
```

The test lights the first 60 pixels on GPIO27 at low brightness and cycles red, green, blue, and dim white.

## Web Dashboard

After Wi-Fi connects, open the dashboard URL printed in Serial Monitor:

```txt
Dashboard: http://192.168.x.x
```

The dashboard shows live race/RPM/packet/Wi-Fi status and lets you change:

- UDP telemetry port
- LED strip pixel count
- LED RPM display mode
- LED brightness
- Green/yellow/red RPM thresholds
- Red blink threshold

Settings are saved on the ESP32 and survive reboot. If you change the UDP port, update the port in Forza telemetry settings too. The dashboard also includes buttons for a strip startup test, turning the strip off, and resetting saved settings to defaults.

The physical RPM mode buttons also save the selected mode. Press GPIO16's button to move backward through `Fill -> Center -> Solid -> Fill`, or GPIO18's button to move forward through `Fill -> Solid -> Center -> Fill`. GPIO19 is also accepted as an alternate next-mode input.

## Forza Setup

In Forza, enable Data Out/telemetry and set:

| Setting | Value |
|---|---|
| Data Out | ON |
| IP Address | ESP32 IP from Serial Monitor |
| Port | `5300`, or the saved dashboard UDP port |
| Format | Dash or Sled |

See [docs/forza-setup.md](docs/forza-setup.md) for setup and network checks.

## Telemetry Fields

The parser reads these little-endian fields and ignores packets shorter than 28 bytes:

| Field | Type | Offset |
|---|---|---:|
| `isRaceOn` | int32 | `0`, fallback `8` |
| `engineMaxRpm` | float | `8`, fallback `16` |
| `engineIdleRpm` | float | `12`, fallback `20` |
| `currentEngineRpm` | float | `16`, fallback `24` |

## Troubleshooting

If LEDs do not turn on during startup test:

- Confirm the strip data input is wired to GPIO27.
- Confirm the strip input end is connected, not the output end.
- Confirm ESP32 GND and the strip power-supply GND are connected together.
- Confirm the strip has enough 5V power for the number of lit pixels.
- Try the `led_strip_test` environment to isolate wiring from telemetry setup.

If Wi-Fi connects but no telemetry appears:

- Confirm Forza is using the ESP32 IP printed in Serial Monitor.
- Confirm the Forza port matches the dashboard UDP port, default `5300`.
- Confirm the gaming device and ESP32 are on the same LAN/VLAN.
- Disable guest network or client isolation for this test.
- Use a 2.4 GHz Wi-Fi network for the ESP32.

If the OLED stays blank:

- Confirm VCC goes to `3V3`, not GPIO27 or another signal pin.
- Confirm OLED GND is tied to ESP32 GND.
- Confirm SDA is GPIO21 and SCL is GPIO22.
- Check whether your OLED uses I2C address `0x3C`. Some modules use `0x3D`.

If RPM values look wrong:

- Confirm the selected Forza telemetry packet format.
- Check Serial Monitor for short packet messages.
- Verify the game is sending Dash or Sled format telemetry.
