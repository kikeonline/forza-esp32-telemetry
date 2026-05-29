#pragma once

#include <Arduino.h>
#include <IPAddress.h>

#include "ForzaTelemetry.h"

class OledDisplay {
 public:
  bool begin();
  bool isAvailable() const;

  void showBoot();
  void showWifiConnecting();
  void showWifiConnected(const IPAddress& ip, uint16_t udpPort);
  void showWaiting(const IPAddress& ip, uint16_t udpPort);
  void showTimeout();
  void showTelemetry(const ForzaTelemetryPacket& telemetry, float rpmPercent);

 private:
  bool available_ = false;

  void drawHeader(const char* title);
  void drawRpmBar(float rpmPercent);
  void display();
};
