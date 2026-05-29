#pragma once

#include <Arduino.h>

struct ForzaTelemetryPacket {
  bool isRaceOn = false;
  float engineMaxRpm = 0.0f;
  float engineIdleRpm = 0.0f;
  float currentEngineRpm = 0.0f;
  const char* layoutName = "unknown";
  size_t packetSize = 0;

  float rpmPercent() const;
};

class ForzaTelemetry {
 public:
  static constexpr size_t MinPacketSize = 28;

  static bool parse(const uint8_t* packet, size_t length, ForzaTelemetryPacket& telemetry);

 private:
  static void parseWithOffsets(const uint8_t* packet,
                               size_t length,
                               size_t raceOffset,
                               size_t maxRpmOffset,
                               size_t idleRpmOffset,
                               size_t currentRpmOffset,
                               const char* layoutName,
                               ForzaTelemetryPacket& telemetry);
  static bool looksValid(const ForzaTelemetryPacket& telemetry);
  static int32_t readInt32Le(const uint8_t* packet, size_t offset);
  static float readFloatLe(const uint8_t* packet, size_t offset);
};
