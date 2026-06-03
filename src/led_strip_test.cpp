#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

namespace {
constexpr uint8_t LedStripPin = 27;
constexpr uint16_t LedCount = 60;
constexpr uint8_t Brightness = 32;
constexpr unsigned long StepDelayMs = 20;
constexpr unsigned long HoldDelayMs = 700;

Adafruit_NeoPixel strip(LedCount, LedStripPin, NEO_GRB + NEO_KHZ800);

void fillStrip(uint32_t color) {
  for (uint16_t i = 0; i < strip.numPixels(); ++i) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void wipe(uint32_t color) {
  strip.clear();
  for (uint16_t i = 0; i < strip.numPixels(); ++i) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(StepDelayMs);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  strip.begin();
  strip.setBrightness(Brightness);
  strip.clear();
  strip.show();

  Serial.println("LED strip test on GPIO27");
  Serial.println("Lighting first 60 pixels at low brightness.");

  fillStrip(strip.Color(24, 24, 24));
  delay(1500);
}

void loop() {
  wipe(strip.Color(255, 0, 0));
  delay(HoldDelayMs);
  wipe(strip.Color(0, 255, 0));
  delay(HoldDelayMs);
  wipe(strip.Color(0, 0, 255));
  delay(HoldDelayMs);
  fillStrip(strip.Color(24, 24, 24));
  delay(1500);
}
