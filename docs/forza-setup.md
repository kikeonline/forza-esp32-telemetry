# Forza Setup

The ESP32 listens for UDP telemetry packets on port `5300`.

## ESP32

1. Copy `include/WifiConfig.example.h` to `include/WifiConfig.h` and set Wi-Fi credentials.
2. Upload the firmware.
3. Open Serial Monitor at `115200` baud.
4. Wait for the ESP32 IP address.

Expected output:

```txt
Wi-Fi connected.
ESP32 IP address: 192.168.x.x
Listening for Forza telemetry on UDP port 5300
```

If the OLED is connected, it also shows the ESP32 IP address and UDP port after Wi-Fi connects.

## Forza

In Forza telemetry/data output settings:

| Setting | Value |
|---|---|
| Data Out | ON |
| IP Address | ESP32 IP printed in Serial Monitor |
| Port | `5300` |
| Format | Dash or Sled |

The ESP32 and the game device must be on the same network. If the ESP32 is on a guest network, UDP packets from the PC or console may be blocked.

## Runtime Check

When telemetry is received, Serial Monitor prints lines like:

```txt
RPM: 3250 / 7500 | 42.3%
```

If the car is not in an active race, the firmware turns all LEDs off. If telemetry stops for more than `1000 ms`, the firmware also turns all LEDs off.

## No Telemetry Checklist

- Data Out is ON.
- Forza IP address matches the ESP32 IP.
- Port is exactly `5300`.
- ESP32 and PC/Xbox are on the same LAN/VLAN.
- Router guest network or client isolation is disabled.
- ESP32 is connected to a 2.4 GHz network.
- Serial Monitor does not show repeated short packet messages.
