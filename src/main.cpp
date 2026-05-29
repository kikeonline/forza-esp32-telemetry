#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "ForzaTelemetry.h"
#include "OledDisplay.h"
#include "RpmLeds.h"
#include "WifiConfig.h"

namespace {
constexpr uint16_t UdpPort = 5300;
constexpr uint8_t LedBluePin = 16;
constexpr uint8_t LedGreenPin = 17;
constexpr uint8_t LedYellowPin = 18;
constexpr uint8_t LedRedPin = 19;
constexpr unsigned long TelemetryTimeoutMs = 1000;
constexpr unsigned long WifiRetryDelayMs = 500;
constexpr unsigned long SerialPrintIntervalMs = 250;
constexpr unsigned long IdleStatusIntervalMs = 5000;
constexpr size_t PacketBufferSize = 512;

WiFiUDP udp;
RpmLeds rpmLeds(LedBluePin, LedGreenPin, LedYellowPin, LedRedPin);
OledDisplay oledDisplay;
uint8_t packetBuffer[PacketBufferSize];
unsigned long lastPacketTimeMs = 0;
unsigned long lastSerialPrintMs = 0;
unsigned long lastIdleStatusMs = 0;
bool telemetryTimedOut = true;

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
  oledDisplay.showWifiConnected(WiFi.localIP(), UdpPort);
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
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(100);

  rpmLeds.begin();
  oledDisplay.begin();
  rpmLeds.startupTest();

  connectWifi();

  udp.begin(UdpPort);
  Serial.print("Listening for Forza telemetry on UDP port ");
  Serial.println(UdpPort);
}

void loop() {
  const unsigned long loopNowMs = millis();
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
    rpmLeds.off();
    Serial.println("Telemetry timeout; LEDs off.");
    oledDisplay.showTimeout();
  }

  if (telemetryTimedOut && loopNowMs - lastIdleStatusMs > IdleStatusIntervalMs) {
    lastIdleStatusMs = loopNowMs;
    Serial.print("Waiting for Forza UDP on ");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.println(UdpPort);
    oledDisplay.showWaiting(WiFi.localIP(), UdpPort);
  }
}
