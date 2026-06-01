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
constexpr uint8_t LedStripPin = 5;
constexpr uint16_t LedStripCount = 60;
constexpr unsigned long TelemetryTimeoutMs = 1000;
constexpr unsigned long WifiRetryDelayMs = 500;
constexpr unsigned long SerialPrintIntervalMs = 250;
constexpr unsigned long IdleStatusIntervalMs = 5000;
constexpr size_t PacketBufferSize = 512;

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
  rpmLeds.applySettings(settings);
  if (activeUdpPort != settings.udpPort) {
    beginUdp(settings.udpPort);
  }
  if (telemetryTimedOut) {
    rpmLeds.off();
    oledDisplay.showWaiting(WiFi.localIP(), activeUdpPort);
  }
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
  if (dashboard.consumeSettingsChanged()) {
    applyRuntimeSettings();
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
          oledDisplay.showTelemetry(telemetry, rpmPercent);
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
