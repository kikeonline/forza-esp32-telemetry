#pragma once

#include <Arduino.h>
#include <WebServer.h>

#include "DeviceSettings.h"
#include "RpmLeds.h"
#include "SettingsStore.h"

class WebDashboard {
 public:
  WebDashboard(DeviceSettings& settings,
               TelemetryStatus& status,
               SettingsStore& settingsStore,
               RpmLeds& rpmLeds);

  void begin();
  void handleClient();
  bool consumeSettingsChanged();

 private:
  WebServer server_;
  DeviceSettings& settings_;
  TelemetryStatus& status_;
  SettingsStore& settingsStore_;
  RpmLeds& rpmLeds_;
  bool settingsChanged_ = false;

  void handleRoot();
  void handleSettingsJson();
  void handleStatusJson();
  void handleSaveSettings();
  void handleResetSettings();
  void handleLedTest();
  void handleNotFound();

  uint16_t argUInt16(const char* name, uint16_t fallback, uint16_t minValue, uint16_t maxValue);
  uint8_t argUInt8(const char* name, uint8_t fallback, uint8_t minValue, uint8_t maxValue);
  RpmLedMode argRpmLedMode(const char* name, RpmLedMode fallback);
  float argPercent(const char* name, float fallback);
  String settingsJson() const;
  String statusJson() const;
};
