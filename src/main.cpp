#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "ForzaTelemetry.h"
#include "OledDisplay.h"
#include "RpmLeds.h"
#include "SettingsStore.h"
#include "WebDashboard.h"
#include "WifiConfig.h"

namespace {
constexpr uint8_t LedStripPin = 27;
constexpr uint16_t LedStripCount = 60;
constexpr uint8_t PreviousModeButtonPin = 16;
constexpr uint8_t NextModeButtonPin = 18;
constexpr uint8_t AlternateNextModeButtonPin = 19;
constexpr uint8_t ButtonInputMode = INPUT_PULLUP;
constexpr bool ButtonPressedLevel = LOW;
constexpr unsigned long ButtonDebounceMs = 35;
constexpr unsigned long TelemetryTimeoutMs = 1000;
constexpr unsigned long WifiRetryDelayMs = 500;
constexpr unsigned long SerialPrintIntervalMs = 250;
constexpr unsigned long IdleStatusIntervalMs = 5000;
constexpr unsigned long ModeChangeDisplayMs = 1200;
constexpr size_t PacketBufferSize = 512;

struct ButtonState {
  ButtonState(uint8_t buttonPin, const char* buttonName) : pin(buttonPin), name(buttonName) {}

  uint8_t pin;
  const char* name;
  bool lastReading = HIGH;
  bool stableState = HIGH;
  unsigned long lastChangeMs = 0;
};

WiFiUDP udp;
SettingsStore settingsStore;
DeviceSettings settings;
RpmLeds rpmLeds(LedStripPin, LedStripCount);
OledDisplay oledDisplay;
TelemetryStatus telemetryStatus;
WebDashboard dashboard(settings, telemetryStatus, settingsStore, rpmLeds);
uint8_t packetBuffer[PacketBufferSize];
unsigned long lastPacketTimeMs = 0;
unsigned long lastSerialPrintMs = 0;
unsigned long lastIdleStatusMs = 0;
bool telemetryTimedOut = true;
uint16_t activeUdpPort = 0;
RpmLedMode activeRpmLedMode = RpmLedMode::Fill;
unsigned long modeChangeDisplayUntilMs = 0;
ButtonState previousModeButton{PreviousModeButtonPin, "previous"};
ButtonState nextModeButton{NextModeButtonPin, "next"};
ButtonState alternateNextModeButton{AlternateNextModeButtonPin, "next alternate"};

const char* rpmLedModeName(RpmLedMode mode) {
  switch (mode) {
    case RpmLedMode::Fill:
      return "Fill";
    case RpmLedMode::Solid:
      return "Solid";
    case RpmLedMode::Center:
      return "Center";
  }
  return "Unknown";
}

RpmLedMode cycleRpmLedMode(RpmLedMode mode, int direction) {
  constexpr int ModeCount = static_cast<int>(RpmLedMode::Center) + 1;
  int nextMode = static_cast<int>(mode) + direction;
  if (nextMode < 0) {
    nextMode += ModeCount;
  }
  if (nextMode >= ModeCount) {
    nextMode -= ModeCount;
  }
  return static_cast<RpmLedMode>(nextMode);
}

void beginButton(ButtonState& button) {
  pinMode(button.pin, ButtonInputMode);
  const bool reading = digitalRead(button.pin);
  button.lastReading = reading;
  button.stableState = reading;
  button.lastChangeMs = millis();
  Serial.print("Button ");
  Serial.print(button.name);
  Serial.print(" GPIO");
  Serial.print(button.pin);
  Serial.println(reading == ButtonPressedLevel ? " starts pressed" : " starts released");
}

void beginModeButtons() {
  beginButton(previousModeButton);
  beginButton(nextModeButton);
  beginButton(alternateNextModeButton);
}

bool consumeButtonPress(ButtonState& button, unsigned long nowMs) {
  const bool reading = digitalRead(button.pin);
  if (reading != button.lastReading) {
    button.lastReading = reading;
    button.lastChangeMs = nowMs;
  }

  if (nowMs - button.lastChangeMs < ButtonDebounceMs) {
    return false;
  }

  if (reading != button.stableState) {
    button.stableState = reading;
    Serial.print("Button ");
    Serial.print(button.name);
    Serial.print(" GPIO");
    Serial.print(button.pin);
    Serial.println(button.stableState == ButtonPressedLevel ? " pressed" : " released");
    return button.stableState == ButtonPressedLevel;
  }

  return false;
}

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  oledDisplay.showWifiConnecting();
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(WifiRetryDelayMs);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.localIP());
  oledDisplay.showWifiConnected(WiFi.localIP(), settings.udpPort);
}

void printTelemetry(const ForzaTelemetryPacket& telemetry, float rpmPercent) {
  Serial.print("Packet: ");
  Serial.print(telemetry.packetSize);
  Serial.print(" bytes | Layout: ");
  Serial.print(telemetry.layoutName);
  Serial.print(" | Race: ");
  Serial.print(telemetry.isRaceOn ? "on" : "off");
  Serial.print(" | ");
  Serial.print("RPM: ");
  Serial.print(telemetry.currentEngineRpm, 0);
  Serial.print(" / ");
  Serial.print(telemetry.engineMaxRpm, 0);
  Serial.print(" | ");
  Serial.print(rpmPercent * 100.0f, 1);
  Serial.println("%");
}

void beginUdp(uint16_t udpPort) {
  udp.stop();
  udp.begin(udpPort);
  activeUdpPort = udpPort;
  Serial.print("Listening for Forza telemetry on UDP port ");
  Serial.println(activeUdpPort);
}

void applyRuntimeSettings() {
  oledDisplay.setRpmLedMode(settings.ledMode);
  rpmLeds.applySettings(settings);
  if (activeUdpPort != settings.udpPort) {
    beginUdp(settings.udpPort);
  }
  if (telemetryTimedOut) {
    rpmLeds.off();
    oledDisplay.showWaiting(WiFi.localIP(), activeUdpPort);
  }
}

void showRpmModeChanged(unsigned long nowMs) {
  oledDisplay.showRpmModeChanged(settings.ledMode);
  modeChangeDisplayUntilMs = nowMs + ModeChangeDisplayMs;
}

void setRpmLedMode(RpmLedMode mode) {
  if (settings.ledMode == mode) {
    return;
  }

  settings.ledMode = mode;
  oledDisplay.setRpmLedMode(settings.ledMode);
  settingsStore.save(settings);
  applyRuntimeSettings();
  activeRpmLedMode = settings.ledMode;
  showRpmModeChanged(millis());

  Serial.print("LED RPM mode changed to ");
  Serial.println(rpmLedModeName(settings.ledMode));
}

void handleModeButtons(unsigned long nowMs) {
  const bool previousPressed = consumeButtonPress(previousModeButton, nowMs);
  const bool nextPressed = consumeButtonPress(nextModeButton, nowMs) ||
                           consumeButtonPress(alternateNextModeButton, nowMs);
  if (previousPressed == nextPressed) {
    return;
  }

  const int direction = nextPressed ? 1 : -1;
  setRpmLedMode(cycleRpmLedMode(settings.ledMode, direction));
}

void updateTelemetryStatus(const ForzaTelemetryPacket& telemetry, float rpmPercent) {
  telemetryStatus.raceOn = telemetry.isRaceOn;
  telemetryStatus.timedOut = false;
  telemetryStatus.currentRpm = telemetry.currentEngineRpm;
  telemetryStatus.maxRpm = telemetry.engineMaxRpm;
  telemetryStatus.rpmPercent = rpmPercent;
  telemetryStatus.packetSize = telemetry.packetSize;
  telemetryStatus.layoutName = telemetry.layoutName;
  telemetryStatus.lastPacketAgeMs = 0;
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  settingsStore.begin();
  settings = settingsStore.load();
  activeRpmLedMode = settings.ledMode;
  oledDisplay.setRpmLedMode(settings.ledMode);
  beginModeButtons();

  rpmLeds.begin();
  rpmLeds.applySettings(settings);
  oledDisplay.begin();
  rpmLeds.startupTest();

  connectWifi();
  dashboard.begin();

  beginUdp(settings.udpPort);
}

void loop() {
  const unsigned long loopNowMs = millis();
  dashboard.handleClient();
  handleModeButtons(loopNowMs);
  if (dashboard.consumeSettingsChanged()) {
    const RpmLedMode previousRpmLedMode = activeRpmLedMode;
    applyRuntimeSettings();
    activeRpmLedMode = settings.ledMode;
    if (settings.ledMode != previousRpmLedMode) {
      showRpmModeChanged(loopNowMs);
      Serial.print("LED RPM mode changed to ");
      Serial.println(rpmLedModeName(settings.ledMode));
    }
  }

  telemetryStatus.timedOut = telemetryTimedOut;
  telemetryStatus.lastPacketAgeMs = lastPacketTimeMs == 0 ? 0 : loopNowMs - lastPacketTimeMs;

  const int packetSize = udp.parsePacket();
  if (packetSize > 0) {
    const size_t bytesToRead = min(static_cast<size_t>(packetSize), PacketBufferSize);
    const int bytesRead = udp.read(packetBuffer, bytesToRead);

    if (bytesRead >= static_cast<int>(ForzaTelemetry::MinPacketSize)) {
      ForzaTelemetryPacket telemetry;
      if (ForzaTelemetry::parse(packetBuffer, static_cast<size_t>(bytesRead), telemetry)) {
        lastPacketTimeMs = loopNowMs;
        telemetryTimedOut = false;

        const float rpmPercent = telemetry.rpmPercent();
        updateTelemetryStatus(telemetry, rpmPercent);
        if (loopNowMs - lastSerialPrintMs >= SerialPrintIntervalMs) {
          lastSerialPrintMs = loopNowMs;
          printTelemetry(telemetry, rpmPercent);
          if (loopNowMs >= modeChangeDisplayUntilMs) {
            oledDisplay.showTelemetry(telemetry, rpmPercent, settings.ledMode);
          }
        }

        if (telemetry.isRaceOn) {
          rpmLeds.update(rpmPercent, loopNowMs);
        } else {
          rpmLeds.off();
        }
      } else {
        Serial.print("Ignoring UDP packet with unexpected telemetry values: ");
        Serial.print(bytesRead);
        Serial.println(" bytes");
      }
    } else {
      Serial.print("Ignoring short UDP packet: ");
      Serial.print(bytesRead);
      Serial.println(" bytes");
    }

    while (udp.available() > 0) {
      udp.read();
    }
  }

  if (!telemetryTimedOut && loopNowMs - lastPacketTimeMs > TelemetryTimeoutMs) {
    telemetryTimedOut = true;
    telemetryStatus.timedOut = true;
    telemetryStatus.raceOn = false;
    telemetryStatus.rpmPercent = 0.0f;
    rpmLeds.off();
    Serial.println("Telemetry timeout; LEDs off.");
    oledDisplay.showTimeout();
  }

  if (telemetryTimedOut && loopNowMs - lastIdleStatusMs > IdleStatusIntervalMs) {
    lastIdleStatusMs = loopNowMs;
    Serial.print("Waiting for Forza UDP on ");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(activeUdpPort);
    oledDisplay.showWaiting(WiFi.localIP(), activeUdpPort);
  }
}
