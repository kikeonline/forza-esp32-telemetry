#include "RpmLeds.h"

#include <math.h>

RpmLeds::RpmLeds(uint8_t dataPin, uint16_t ledCount)
    : strip_(ledCount, dataPin, NEO_GRB + NEO_KHZ800) {}

void RpmLeds::begin() {
  strip_.begin();
  applySettings(settings_);
  off();
}

void RpmLeds::applySettings(const DeviceSettings& settings) {
  settings_ = settings;
  settings_.ledCount = constrain(settings_.ledCount, static_cast<uint16_t>(1),
                                 static_cast<uint16_t>(300));
  settings_.brightness = constrain(settings_.brightness, static_cast<uint8_t>(1),
                                   static_cast<uint8_t>(255));

  strip_.updateLength(settings_.ledCount);
  strip_.setBrightness(settings_.brightness);
  strip_.clear();
  strip_.show();
}

void RpmLeds::update(float rpmPercent, unsigned long nowMs) {
  rpmPercent = constrain(rpmPercent, 0.0f, 1.0f);
  const uint16_t pixelCount = strip_.numPixels();
  const uint32_t rpmColor = colorForRpm(rpmPercent);

  // Near the limiter, blink the active RPM pattern as a shift warning.
  if (rpmPercent >= settings_.redBlinkThreshold) {
    if (nowMs - lastBlinkToggleMs_ >= BlinkIntervalMs) {
      lastBlinkToggleMs_ = nowMs;
      redBlinkState_ = !redBlinkState_;
    }
  } else {
    redBlinkState_ = true;
    lastBlinkToggleMs_ = nowMs;
  }

  strip_.clear();
  if (redBlinkState_) {
    if (settings_.ledMode == RpmLedMode::Solid) {
      const float scale = 0.10f + (rpmPercent * 0.90f);
      strip_.fill(scaledColor(rpmColor, scale));
    } else {
      const uint16_t litPixels = static_cast<uint16_t>(ceilf(rpmPercent * pixelCount));
      if (settings_.ledMode == RpmLedMode::Center) {
        const uint16_t startPixel = (pixelCount - litPixels) / 2;
        for (uint16_t i = startPixel; i < startPixel + litPixels; ++i) {
          strip_.setPixelColor(i, rpmColor);
        }
      } else {
        for (uint16_t i = 0; i < litPixels; ++i) {
          strip_.setPixelColor(i, rpmColor);
        }
      }
    }
  }
  strip_.show();
}

void RpmLeds::off() {
  redBlinkState_ = true;
  lastBlinkToggleMs_ = millis();
  strip_.clear();
  strip_.show();
}

void RpmLeds::startupTest(unsigned int stepDelayMs) {
  strip_.fill(strip_.Color(0, 0, 255));
  strip_.show();
  delay(stepDelayMs);
  strip_.fill(strip_.Color(0, 255, 0));
  strip_.show();
  delay(stepDelayMs);
  strip_.fill(strip_.Color(255, 180, 0));
  strip_.show();
  delay(stepDelayMs);
  strip_.fill(strip_.Color(255, 0, 0));
  strip_.show();
  delay(stepDelayMs);
  off();
}

uint32_t RpmLeds::colorForRpm(float rpmPercent) const {
  if (rpmPercent >= settings_.redThreshold) {
    return strip_.Color(255, 0, 0);
  }
  if (rpmPercent >= settings_.yellowThreshold) {
    return strip_.Color(255, 180, 0);
  }
  if (rpmPercent >= settings_.greenThreshold) {
    return strip_.Color(0, 255, 0);
  }
  return strip_.Color(0, 0, 255);
}

uint32_t RpmLeds::scaledColor(uint32_t color, float scale) const {
  scale = constrain(scale, 0.0f, 1.0f);
  const uint8_t red = static_cast<uint8_t>(((color >> 16) & 0xff) * scale);
  const uint8_t green = static_cast<uint8_t>(((color >> 8) & 0xff) * scale);
  const uint8_t blue = static_cast<uint8_t>((color & 0xff) * scale);
  return strip_.Color(red, green, blue);
}
