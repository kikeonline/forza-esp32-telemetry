# Forza Telemetry RPM LED Project — Codex VS Code Handoff

## Goal

Build a simple Forza Motorsport / Forza Horizon telemetry shift-light using an ESP32 and four single LEDs.

The first MVP should:

1. Connect the ESP32 to Wi-Fi.
2. Listen for Forza UDP telemetry packets.
3. Read engine RPM values from telemetry.
4. Light four discrete LEDs based on RPM percentage.
5. Turn LEDs off when telemetry stops or the race is not active.

This project intentionally starts with normal single LEDs instead of an addressable LED strip.

---

## Hardware available

- ESP32 development board with Wi-Fi and Bluetooth
- Breadboard
- Single LEDs:
  - Blue
  - Green
  - Yellow
  - Red
- Resistors, ideally `220Ω` to `330Ω`
- Jumper wires
- USB cable for ESP32 programming/power

Bluetooth is not needed for this version. Use Wi-Fi because Forza sends telemetry over UDP.

---

## LED behavior

Use four progressive RPM stages:

| RPM percentage | LEDs on |
|---:|---|
| `< 25%` | none |
| `>= 25%` | Blue |
| `>= 50%` | Blue + Green |
| `>= 75%` | Blue + Green + Yellow |
| `>= 90%` | Blue + Green + Yellow + Red |
| `>= 97%` | Red blinks |

Use normalized RPM:

```txt
rpmPercent = (currentEngineRpm - engineIdleRpm) / (engineMaxRpm - engineIdleRpm)
```

Clamp the value between `0.0` and `1.0`.

---

## Wiring

Recommended GPIO mapping:

| LED | ESP32 GPIO |
|---|---:|
| Blue | GPIO16 |
| Green | GPIO17 |
| Yellow | GPIO18 |
| Red | GPIO19 |

Each LED needs its own resistor.

Wire each LED like this:

```txt
ESP32 GPIO → resistor → LED long leg/anode
LED short leg/cathode → GND
```

Use a shared ground rail on the breadboard:

```txt
ESP32 GND → breadboard negative rail
Blue LED short leg   → negative rail
Green LED short leg  → negative rail
Yellow LED short leg → negative rail
Red LED short leg    → negative rail
```

Important rules:

- It is okay for all LEDs to share the same ESP32 GND.
- Do not share one resistor across all LEDs.
- Use one resistor per LED.
- Avoid GPIO34, GPIO35, GPIO36, and GPIO39 for LEDs because they are input-only on many ESP32 boards.
- Avoid using TX0/GPIO1 and RX0/GPIO3 unless necessary, because they are used for serial communication.

---

## Forza UDP telemetry assumptions

The firmware should listen on UDP port:

```txt
5300
```

In Forza, enable telemetry/data output and configure:

```txt
Data Out: ON
IP Address: ESP32 IP address printed in Serial Monitor
Port: 5300
Format: Dash or Sled
```

The ESP32 and game device must be on the same network.

The ESP32 usually needs a 2.4 GHz Wi-Fi network.

Known useful Forza packet offsets for this MVP:

| Field | Type | Offset |
|---|---|---:|
| `isRaceOn` | int32 little-endian | `8` |
| `engineMaxRpm` | float little-endian | `16` |
| `engineIdleRpm` | float little-endian | `20` |
| `currentEngineRpm` | float little-endian | `24` |

The implementation should validate that the packet is at least 28 bytes before reading these values.

---

## Recommended repository structure

For Arduino IDE, a minimal structure is enough:

```txt
forza-esp32-rpm-led/
  forza-esp32-rpm-led.ino
  README.md
  docs/
    wiring.md
    forza-setup.md
```

For PlatformIO in VS Code, prefer:

```txt
forza-esp32-rpm-led/
  platformio.ini
  src/
    main.cpp
  include/
    ForzaTelemetry.h
    RpmLeds.h
  lib/
  test/
  docs/
    wiring.md
    forza-setup.md
  README.md
```

Recommended approach: use PlatformIO if continuing in VS Code.

---

## Codex task prompt

Use this as the main Codex prompt in VS Code:

```txt
You are helping me build an ESP32 Forza telemetry RPM shift-light project.

Create a PlatformIO project for an ESP32 DevKit board. The firmware must connect to Wi-Fi, listen for UDP packets on port 5300, parse Forza telemetry fields for isRaceOn, engineMaxRpm, engineIdleRpm, and currentEngineRpm, and drive four single LEDs connected to GPIO16, GPIO17, GPIO18, and GPIO19.

Hardware:
- ESP32 DevKit-style board
- Blue LED on GPIO16
- Green LED on GPIO17
- Yellow LED on GPIO18
- Red LED on GPIO19
- One resistor per LED
- All LED cathodes share ESP32 GND through a breadboard ground rail

Telemetry parsing assumptions:
- isRaceOn: int32 little-endian at offset 8
- engineMaxRpm: float little-endian at offset 16
- engineIdleRpm: float little-endian at offset 20
- currentEngineRpm: float little-endian at offset 24
- Ignore packets shorter than 28 bytes

LED behavior:
- Blue on at 25% RPM
- Green on at 50% RPM
- Yellow on at 75% RPM
- Red on at 90% RPM
- Red blinks at 97% RPM and above
- All LEDs off if race is not active
- All LEDs off if no telemetry packet has been received for more than 1000 ms

Implementation requirements:
- Use clean, modular C++.
- Put telemetry parsing into a small ForzaTelemetry module.
- Put LED logic into a small RpmLeds module.
- Keep main.cpp readable and minimal.
- Print the ESP32 IP address to Serial after Wi-Fi connects.
- Print current RPM, max RPM, and RPM percentage to Serial while telemetry is received.
- Add comments explaining the packet offsets and LED thresholds.
- Add a README explaining wiring, Forza setup, build/upload steps, and troubleshooting.

Do not use Bluetooth. Do not use addressable LED strip libraries for this MVP.
```

---

## Suggested PlatformIO config

Codex can create this, but the project will likely need something close to:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

---

## Firmware skeleton

Codex should generate the actual files, but the implementation should roughly follow this flow:

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const uint16_t UDP_PORT = 5300;
WiFiUDP udp;

const int LED_BLUE = 16;
const int LED_GREEN = 17;
const int LED_YELLOW = 18;
const int LED_RED = 19;

uint8_t packetBuffer[512];
unsigned long lastPacketTime = 0;
const unsigned long TELEMETRY_TIMEOUT_MS = 1000;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Connect Wi-Fi
  // Start UDP listener
  // Print ESP32 IP address
  // Run startup LED test
}

void loop() {
  // Read UDP packet
  // Validate packet size
  // Parse Forza values
  // If isRaceOn, update LEDs from RPM
  // Otherwise clear LEDs
  // Clear LEDs if telemetry timeout expires
}
```

---

## Testing plan

### 1. Hardware test first

Before using Forza telemetry, create or run a simple LED test:

```txt
Blue on → Green on → Yellow on → Red on → all off
```

This confirms:

- GPIO pins are correct
- LED polarity is correct
- Resistors are connected properly
- Ground rail is connected properly

### 2. Wi-Fi test

Serial Monitor should show:

```txt
Wi-Fi connected.
ESP32 IP address: xxx.xxx.xxx.xxx
Listening for Forza telemetry on UDP port 5300
```

Use that IP address in Forza telemetry settings.

### 3. Telemetry test

When the car is running, Serial Monitor should print something like:

```txt
RPM: 3250 / 7500 | 42.3%
```

If nothing appears:

- Check that Forza Data Out is enabled.
- Check the ESP32 IP address.
- Check the UDP port is `5300`.
- Make sure the ESP32 and gaming device are on the same network.
- Try disabling guest network/client isolation on the router.
- Make sure the ESP32 is connected to 2.4 GHz Wi-Fi.

---

## Common troubleshooting

### LEDs do not turn on during startup test

Check:

- LED short leg goes to GND.
- LED long leg goes through resistor to GPIO.
- The GPIO number in code matches the physical pin.
- Breadboard rail is actually connected to ESP32 GND.
- Some breadboards split power rails in the middle; bridge the rail if necessary.

### Only one LED works

Check:

- Each LED has its own resistor.
- Each LED has its own GPIO wire.
- All LED cathodes are connected to the common GND rail.

### ESP32 connects to Wi-Fi but receives no telemetry

Check:

- Forza telemetry IP equals the ESP32 IP printed in Serial Monitor.
- Port is exactly `5300`.
- Data Out is ON.
- Router does not block device-to-device UDP traffic.
- PC/Xbox and ESP32 are on the same LAN/VLAN.

### RPM values look wrong

Possible causes:

- Forza packet format mismatch.
- Wrong offsets for the selected telemetry format.
- Reading from a packet shorter than expected.

Add Serial debug output for packet size first.

---

## Future upgrades

After the four-LED MVP works:

1. Add configurable thresholds.
2. Add gear display using a 7-segment display or small OLED.
3. Add a WS2812B LED strip.
4. Add brake/throttle LEDs.
5. Add ABS/traction-control indicators.
6. Add a small web config page hosted by the ESP32.
7. Add EEPROM/NVS storage for Wi-Fi credentials and thresholds.

---

## Definition of done for MVP

The MVP is complete when:

- ESP32 connects to Wi-Fi.
- ESP32 receives Forza UDP packets.
- Serial Monitor prints RPM data.
- Blue, green, yellow, and red LEDs light progressively as RPM rises.
- Red blinks near limiter.
- LEDs turn off when race is inactive or telemetry stops.
- README explains wiring and setup clearly.
