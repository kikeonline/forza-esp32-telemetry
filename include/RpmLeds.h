#pragma once

#include <Arduino.h>

class RpmLeds {
 public:
  RpmLeds(uint8_t bluePin, uint8_t greenPin, uint8_t yellowPin, uint8_t redPin);

  void begin();
  void update(float rpmPercent, unsigned long nowMs);
  void off();
  void startupTest(unsigned int stepDelayMs = 180);

 private:
  static constexpr float BlueThreshold = 0.0f;
  static constexpr float GreenThreshold = 0.30f;
  static constexpr float YellowThreshold = 0.65f;
  static constexpr float RedThreshold = 0.80f;
  static constexpr float RedBlinkThreshold = 0.85f;
  static constexpr unsigned long BlinkIntervalMs = 90;

  uint8_t bluePin_;
  uint8_t greenPin_;
  uint8_t yellowPin_;
  uint8_t redPin_;
  unsigned long lastBlinkToggleMs_ = 0;
  bool redBlinkState_ = false;

  void write(bool blue, bool green, bool yellow, bool red);
};
