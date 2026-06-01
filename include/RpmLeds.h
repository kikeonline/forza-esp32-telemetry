#pragma once

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "DeviceSettings.h"

class RpmLeds {
 public:
  RpmLeds(uint8_t dataPin, uint16_t ledCount);

  void begin();
  void applySettings(const DeviceSettings& settings);
  void update(float rpmPercent, unsigned long nowMs);
  void off();
  void startupTest(unsigned int stepDelayMs = 180);

 private:
  static constexpr unsigned long BlinkIntervalMs = 90;

  Adafruit_NeoPixel strip_;
  DeviceSettings settings_;
  unsigned long lastBlinkToggleMs_ = 0;
  bool redBlinkState_ = true;

  uint32_t colorForRpm(float rpmPercent) const;
  uint32_t scaledColor(uint32_t color, float scale) const;
};
