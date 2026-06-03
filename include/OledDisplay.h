#pragma once

#include <Arduino.h>
#include <IPAddress.h>

#include "DeviceSettings.h"
#include "ForzaTelemetry.h"

class OledDisplay {
 public:
  bool begin();
  bool isAvailable() const;
  void setRpmLedMode(RpmLedMode ledMode);

  void showBoot();
  void showWifiConnecting();
  void showWifiConnected(const IPAddress& ip, uint16_t udpPort);
  void showWaiting(const IPAddress& ip, uint16_t udpPort);
  void showTimeout();
  void showTelemetry(const ForzaTelemetryPacket& telemetry,
                     float rpmPercent,
                     RpmLedMode ledMode);
  void showRpmModeChanged(RpmLedMode ledMode);

 private:
  bool available_ = false;
  RpmLedMode ledMode_ = RpmLedMode::Fill;

  const char* rpmLedModeName(RpmLedMode ledMode) const;
  void printModeLine(int16_t x, int16_t y);
  void drawHeader(const char* title);
  void drawRpmBar(float rpmPercent);
  void display();
};
