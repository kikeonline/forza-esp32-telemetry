#include "SettingsStore.h"

namespace {
constexpr const char* Namespace = "forza";

float getThreshold(Preferences& preferences, const char* key, float fallback) {
  return preferences.getFloat(key, fallback);
}

float sanitizeThreshold(float value, float fallback) {
  if (!isfinite(value) || value < 0.0f || value > 1.0f) {
    return fallback;
  }
  return value;
}

RpmLedMode sanitizeLedMode(uint8_t value, RpmLedMode fallback) {
  if (value <= static_cast<uint8_t>(RpmLedMode::Center)) {
    return static_cast<RpmLedMode>(value);
  }
  return fallback;
}
}  // namespace

void SettingsStore::begin() {
  preferences_.begin(Namespace, false);
}

DeviceSettings SettingsStore::load() {
  DeviceSettings settings;
  settings.udpPort = preferences_.getUShort("udp", settings.udpPort);
  settings.ledCount = preferences_.getUShort("leds", settings.ledCount);
  settings.brightness = preferences_.getUChar("bright", settings.brightness);
  settings.ledMode =
      sanitizeLedMode(preferences_.getUChar("mode", static_cast<uint8_t>(settings.ledMode)),
                      settings.ledMode);
  settings.greenThreshold =
      sanitizeThreshold(getThreshold(preferences_, "green", settings.greenThreshold),
                        settings.greenThreshold);
  settings.yellowThreshold =
      sanitizeThreshold(getThreshold(preferences_, "yellow", settings.yellowThreshold),
                        settings.yellowThreshold);
  settings.redThreshold =
      sanitizeThreshold(getThreshold(preferences_, "red", settings.redThreshold),
                        settings.redThreshold);
  settings.redBlinkThreshold =
      sanitizeThreshold(getThreshold(preferences_, "blink", settings.redBlinkThreshold),
                        settings.redBlinkThreshold);

  settings.udpPort = constrain(settings.udpPort, static_cast<uint16_t>(1024),
                               static_cast<uint16_t>(65535));
  settings.ledCount = constrain(settings.ledCount, static_cast<uint16_t>(1),
                                static_cast<uint16_t>(300));
  settings.brightness = constrain(settings.brightness, static_cast<uint8_t>(1),
                                  static_cast<uint8_t>(255));
  return settings;
}

void SettingsStore::save(const DeviceSettings& settings) {
  preferences_.putUShort("udp", settings.udpPort);
  preferences_.putUShort("leds", settings.ledCount);
  preferences_.putUChar("bright", settings.brightness);
  preferences_.putUChar("mode", static_cast<uint8_t>(settings.ledMode));
  preferences_.putFloat("green", settings.greenThreshold);
  preferences_.putFloat("yellow", settings.yellowThreshold);
  preferences_.putFloat("red", settings.redThreshold);
  preferences_.putFloat("blink", settings.redBlinkThreshold);
}

void SettingsStore::reset() {
  preferences_.clear();
}
