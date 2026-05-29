#include "ForzaTelemetry.h"

#include <math.h>
#include <string.h>

float ForzaTelemetryPacket::rpmPercent() const {
  const float rpmRange = engineMaxRpm - engineIdleRpm;
  if (rpmRange <= 0.0f) {
    return 0.0f;
  }

  const float normalized = (currentEngineRpm - engineIdleRpm) / rpmRange;
  return constrain(normalized, 0.0f, 1.0f);
}

bool ForzaTelemetry::parse(const uint8_t* packet, size_t length, ForzaTelemetryPacket& telemetry) {
  if (packet == nullptr || length < MinPacketSize) {
    return false;
  }

  // Common Forza Dash/Sled layout:
  // isRaceOn: int32 at byte 0, engineMaxRpm: float at byte 8,
  // engineIdleRpm: float at byte 12, currentEngineRpm: float at byte 16.
  parseWithOffsets(packet, length, 0, 8, 12, 16, "standard", telemetry);
  if (looksValid(telemetry)) {
    return true;
  }

  // Some notes/examples describe these values shifted by 8 bytes. Keep this as
  // a fallback so the firmware can tolerate either layout during setup.
  parseWithOffsets(packet, length, 8, 16, 20, 24, "shifted+8", telemetry);
  if (looksValid(telemetry)) {
    return true;
  }

  telemetry.layoutName = "invalid";

  return false;
}

void ForzaTelemetry::parseWithOffsets(const uint8_t* packet,
                                      size_t length,
                                      size_t raceOffset,
                                      size_t maxRpmOffset,
                                      size_t idleRpmOffset,
                                      size_t currentRpmOffset,
                                      const char* layoutName,
                                      ForzaTelemetryPacket& telemetry) {
  telemetry.isRaceOn = readInt32Le(packet, raceOffset) != 0;
  telemetry.engineMaxRpm = readFloatLe(packet, maxRpmOffset);
  telemetry.engineIdleRpm = readFloatLe(packet, idleRpmOffset);
  telemetry.currentEngineRpm = readFloatLe(packet, currentRpmOffset);
  telemetry.layoutName = layoutName;
  telemetry.packetSize = length;
}

bool ForzaTelemetry::looksValid(const ForzaTelemetryPacket& telemetry) {
  if (!isfinite(telemetry.engineMaxRpm) || !isfinite(telemetry.engineIdleRpm) ||
      !isfinite(telemetry.currentEngineRpm)) {
    return false;
  }

  return telemetry.engineMaxRpm >= 1000.0f && telemetry.engineMaxRpm <= 30000.0f &&
         telemetry.engineIdleRpm >= 0.0f && telemetry.engineIdleRpm < telemetry.engineMaxRpm &&
         telemetry.currentEngineRpm >= 0.0f &&
         telemetry.currentEngineRpm <= telemetry.engineMaxRpm * 1.5f;
}

int32_t ForzaTelemetry::readInt32Le(const uint8_t* packet, size_t offset) {
  return static_cast<int32_t>(static_cast<uint32_t>(packet[offset]) |
                              (static_cast<uint32_t>(packet[offset + 1]) << 8) |
                              (static_cast<uint32_t>(packet[offset + 2]) << 16) |
                              (static_cast<uint32_t>(packet[offset + 3]) << 24));
}

float ForzaTelemetry::readFloatLe(const uint8_t* packet, size_t offset) {
  const uint32_t raw = static_cast<uint32_t>(packet[offset]) |
                       (static_cast<uint32_t>(packet[offset + 1]) << 8) |
                       (static_cast<uint32_t>(packet[offset + 2]) << 16) |
                       (static_cast<uint32_t>(packet[offset + 3]) << 24);

  float value = 0.0f;
  static_assert(sizeof(value) == sizeof(raw), "float and uint32_t must be 4 bytes");
  memcpy(&value, &raw, sizeof(value));
  return value;
}
