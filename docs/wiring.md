# Wiring

Use a WS2812/NeoPixel-style addressable LED strip connected to GPIO5. The firmware defaults to the first 60 pixels, and the pixel count can be changed from the web dashboard.

The optional OLED is a common `0.96 inch` I2C SSD1306 display with `128x64` pixels and address `0x3C`.

## Pin Map

| LED Strip Pin | ESP32 Pin |
|---|---|
| DIN/Data | GPIO5 |
| GND | `GND` |
| 5V/VCC | External 5V supply or suitable 5V source |

## OLED Wiring

| OLED Pin | ESP32 Pin |
|---|---|
| VCC | `3V3` |
| GND | `GND` |
| SDA | GPIO21 |
| SCL | GPIO22 |

Use the ESP32 `3V3` pin for the OLED unless your specific module documentation says otherwise. The OLED ground, ESP32 ground, and LED strip power-supply ground must be connected together.

## LED Wiring

Connect the strip data input to GPIO5. Connect ESP32 GND and strip power-supply GND together, otherwise the data signal may be unreliable.

Keep brightness conservative unless the strip has a power supply sized for the number of pixels. The firmware defaults to brightness `32` out of `255`.

## Resistors

For longer LED strips, a resistor in series with the data line and a capacitor across strip power can improve reliability. Update the dashboard LED count if you use a strip length other than 60 pixels.

## Pins To Avoid

Avoid GPIO34, GPIO35, GPIO36, and GPIO39 for LED strip data because they are input-only on many ESP32 boards.

Avoid GPIO1/TX0 and GPIO3/RX0 unless necessary because they are used for serial communication.

Keep GPIO21 and GPIO22 available for I2C if you are using the OLED.

## Startup Test

On boot, the firmware runs:

```txt
Blue strip -> Green strip -> Yellow strip -> Red strip -> all off
```

If this does not work, check strip direction, GPIO wiring, power, and the shared ground before debugging Forza telemetry.

You can also upload the standalone strip test:

```sh
pio run -e led_strip_test --target upload
pio device monitor -e led_strip_test
```

The strip test uses GPIO5, lights the first 60 pixels at low brightness, and cycles red, green, blue, and dim white.
