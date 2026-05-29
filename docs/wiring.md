# Wiring

Use four separate GPIO outputs and four separate resistors. Do not share one resistor across multiple LEDs.

The optional OLED is a common `0.96 inch` I2C SSD1306 display with `128x64` pixels and address `0x3C`.

## Pin Map

| LED | ESP32 GPIO |
|---|---:|
| Blue | GPIO16 |
| Green | GPIO17 |
| Yellow | GPIO18 |
| Red | GPIO19 |

## OLED Wiring

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | `3V3` |
| GND | `GND` |
| SDA | GPIO21 |
| SCL | GPIO22 |

Use the ESP32 `3V3` pin for the OLED unless your specific module documentation says otherwise. The OLED ground and LED ground must share ESP32 GND.

## LED Wiring

For each LED:

```txt
ESP32 GPIO -> resistor -> LED long leg/anode
LED short leg/cathode -> breadboard negative rail -> ESP32 GND
```

Shared ground is expected:

```txt
ESP32 GND -> breadboard negative rail
Blue LED short leg -> negative rail
Green LED short leg -> negative rail
Yellow LED short leg -> negative rail
Red LED short leg -> negative rail
```

## Resistors

Use one resistor per LED, typically `220 ohm` to `330 ohm`.

## Pins To Avoid

Avoid GPIO34, GPIO35, GPIO36, and GPIO39 for LEDs because they are input-only on many ESP32 boards.

Avoid GPIO1/TX0 and GPIO3/RX0 unless necessary because they are used for serial communication.

Keep GPIO21 and GPIO22 available for I2C if you are using the OLED.

## Startup Test

On boot, the firmware runs:

```txt
Blue on -> Green on -> Yellow on -> Red on -> all off
```

If this does not work, check LED polarity, resistor placement, GPIO wiring, and the shared ground rail before debugging Forza telemetry.
