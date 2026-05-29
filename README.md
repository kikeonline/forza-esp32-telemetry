# Forza ESP32 RPM LEDs

ESP32 shift-light firmware for Forza Motorsport and Forza Horizon telemetry. The MVP uses four normal single LEDs on GPIO pins and listens for Forza UDP telemetry over Wi-Fi.

It also supports a common `0.96 inch` I2C OLED display (`128x64`, SSD1306, address `0x3C`) for IP address, waiting status, and live RPM display.

## Behavior

The firmware normalizes RPM with:

```txt
rpmPercent = (currentEngineRpm - engineIdleRpm) / (engineMaxRpm - engineIdleRpm)
```

The value is clamped from `0.0` to `1.0`, then mapped to the LEDs:

| RPM percentage | LEDs on |
|---:|---|
| `< 25%` | none |
| `>= 25%` | Blue |
| `>= 50%` | Blue + Green |
| `>= 75%` | Blue + Green + Yellow |
| `>= 90%` | Blue + Green + Yellow + Red |
| `>= 97%` | Blue + Green + Yellow + blinking Red |

All LEDs turn off when `isRaceOn` is false or when no telemetry packet has been received for more than `1000 ms`.

## Hardware

- ESP32 DevKit-style board
- Blue, green, yellow, and red LEDs
- Optional `0.96 inch` I2C OLED, `128x64`, SSD1306, usually address `0x3C`
- One `220 ohm` to `330 ohm` resistor per LED
- Breadboard and jumper wires
- USB cable

Default GPIO mapping:

| LED | ESP32 GPIO |
|---|---:|
| Blue | GPIO16 |
| Green | GPIO17 |
| Yellow | GPIO18 |
| Red | GPIO19 |

Default OLED wiring:

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | `3V3` |
| GND | `GND` |
| SDA | GPIO21 |
| SCL | GPIO22 |

Wire each LED as:

```txt
ESP32 GPIO -> resistor -> LED long leg/anode
LED short leg/cathode -> ESP32 GND
```

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
pio run
pio run --target upload
pio device monitor
```

The serial monitor runs at `115200` baud. After upload, the ESP32 runs a short LED startup test, connects to Wi-Fi, prints its IP address, and starts listening on UDP port `5300`.

If the OLED is connected, it also shows the ESP32 IP address and UDP port after Wi-Fi connects. If the OLED is not found at address `0x3C`, the firmware prints a serial warning and continues with LEDs only.

Expected serial output:

```txt
Wi-Fi connected.
ESP32 IP address: 192.168.x.x
Listening for Forza telemetry on UDP port 5300
```

## Forza Setup

In Forza, enable Data Out/telemetry and set:

| Setting | Value |
|---|---|
| Data Out | ON |
| IP Address | ESP32 IP from Serial Monitor |
| Port | `5300` |
| Format | Dash or Sled |

See [docs/forza-setup.md](docs/forza-setup.md) for setup and network checks.

## Telemetry Fields

The MVP parser reads these little-endian fields and ignores packets shorter than 28 bytes:

| Field | Type | Offset |
|---|---|---:|
| `isRaceOn` | int32 | `0`, fallback `8` |
| `engineMaxRpm` | float | `8`, fallback `16` |
| `engineIdleRpm` | float | `12`, fallback `20` |
| `currentEngineRpm` | float | `16`, fallback `24` |

## Troubleshooting

If LEDs do not turn on during startup test:

- Confirm every LED has its own resistor.
- Confirm each LED long leg goes to the GPIO through the resistor.
- Confirm each LED short leg goes to the shared ESP32 GND rail.
- Check whether your breadboard power rail is split in the middle.

If Wi-Fi connects but no telemetry appears:

- Confirm Forza is using the ESP32 IP printed in Serial Monitor.
- Confirm the port is exactly `5300`.
- Confirm the gaming device and ESP32 are on the same LAN/VLAN.
- Disable guest network or client isolation for this test.
- Use a 2.4 GHz Wi-Fi network for the ESP32.

If the OLED stays blank:

- Confirm VCC goes to `3V3`, not an LED GPIO.
- Confirm OLED GND is tied to ESP32 GND.
- Confirm SDA is GPIO21 and SCL is GPIO22.
- Check whether your OLED uses I2C address `0x3C`. Some modules use `0x3D`.

If RPM values look wrong:

- Confirm the selected Forza telemetry packet format.
- Check Serial Monitor for short packet messages.
- Verify the game is sending Dash or Sled format telemetry.
