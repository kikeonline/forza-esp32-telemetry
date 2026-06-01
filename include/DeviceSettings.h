#pragma once

#include <Arduino.h>

enum class RpmLedMode : uint8_t {
  Fill = 0,
  Solid = 1,
  Center = 2,
};

struct DeviceSettings {
  uint16_t udpPort = 5300;
  uint16_t ledCount = 60;
  uint8_t brightness = 32;
  RpmLedMode ledMode = RpmLedMode::Fill;
  float greenThreshold = 0.30f;
  float yellowThreshold = 0.65f;
  float redThreshold = 0.80f;
  float redBlinkThreshold = 0.85f;
};

struct TelemetryStatus {
  bool raceOn = false;
  bool timedOut = true;
  float currentRpm = 0.0f;
  float maxRpm = 0.0f;
  float rpmPercent = 0.0f;
  size_t packetSize = 0;
  const char* layoutName = "unknown";
  unsigned long lastPacketAgeMs = 0;
};
