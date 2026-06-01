#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "DeviceSettings.h"

class SettingsStore {
 public:
  void begin();
  DeviceSettings load();
  void save(const DeviceSettings& settings);
  void reset();

 private:
  Preferences preferences_;
};
