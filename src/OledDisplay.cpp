#include "OledDisplay.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

namespace {
constexpr int ScreenWidth = 128;
constexpr int ScreenHeight = 64;
constexpr int OledResetPin = -1;
constexpr uint8_t OledAddress = 0x3C;
constexpr uint8_t SdaPin = 21;
constexpr uint8_t SclPin = 22;
constexpr uint32_t I2cClockHz = 400000;

Adafruit_SSD1306 displayDriver(ScreenWidth, ScreenHeight, &Wire, OledResetPin);
}  // namespace

bool OledDisplay::begin() {
  Wire.begin(SdaPin, SclPin);
  Wire.setClock(I2cClockHz);

  available_ = displayDriver.begin(SSD1306_SWITCHCAPVCC, OledAddress);
  if (!available_) {
    Serial.println("OLED not found at I2C address 0x3C; continuing without display.");
    return false;
  }

  displayDriver.clearDisplay();
  displayDriver.setTextColor(SSD1306_WHITE);
  showBoot();
  return true;
}

bool OledDisplay::isAvailable() const {
  return available_;
}

void OledDisplay::showBoot() {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader("Forza RPM");
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 24);
  displayDriver.println("LED shift-light");
  displayDriver.setCursor(0, 40);
  displayDriver.println("Booting...");
  display();
}

void OledDisplay::showWifiConnecting() {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader("Wi-Fi");
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 28);
  displayDriver.println("Connecting...");
  display();
}

void OledDisplay::showWifiConnected(const IPAddress& ip, uint16_t udpPort) {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader("Wi-Fi OK");
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 20);
  displayDriver.print("IP ");
  displayDriver.println(ip);
  displayDriver.setCursor(0, 34);
  displayDriver.print("UDP ");
  displayDriver.println(udpPort);
  displayDriver.setCursor(0, 50);
  displayDriver.println("Waiting for Forza");
  display();
}

void OledDisplay::showWaiting(const IPAddress& ip, uint16_t udpPort) {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader("Waiting");
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 20);
  displayDriver.print(ip);
  displayDriver.print(":");
  displayDriver.println(udpPort);
  displayDriver.setCursor(0, 42);
  displayDriver.println("No telemetry yet");
  display();
}

void OledDisplay::showTimeout() {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader("Telemetry");
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 28);
  displayDriver.println("Signal timeout");
  displayDriver.setCursor(0, 44);
  displayDriver.println("LEDs off");
  display();
}

void OledDisplay::showTelemetry(const ForzaTelemetryPacket& telemetry, float rpmPercent) {
  if (!available_) {
    return;
  }

  displayDriver.clearDisplay();
  drawHeader(telemetry.isRaceOn ? "Race On" : "Race Off");

  displayDriver.setTextSize(2);
  displayDriver.setCursor(0, 18);
  displayDriver.print(static_cast<int>(telemetry.currentEngineRpm + 0.5f));

  displayDriver.setTextSize(1);
  displayDriver.setCursor(86, 20);
  displayDriver.println("RPM");
  displayDriver.setCursor(86, 32);
  displayDriver.print(static_cast<int>(rpmPercent * 100.0f + 0.5f));
  displayDriver.println("%");

  drawRpmBar(rpmPercent);
  display();
}

void OledDisplay::drawHeader(const char* title) {
  displayDriver.setTextSize(1);
  displayDriver.setCursor(0, 0);
  displayDriver.println(title);
  displayDriver.drawLine(0, 10, ScreenWidth - 1, 10, SSD1306_WHITE);
}

void OledDisplay::drawRpmBar(float rpmPercent) {
  rpmPercent = constrain(rpmPercent, 0.0f, 1.0f);

  constexpr int x = 0;
  constexpr int y = 52;
  constexpr int width = 128;
  constexpr int height = 12;
  const int filledWidth = static_cast<int>((width - 2) * rpmPercent + 0.5f);

  displayDriver.drawRect(x, y, width, height, SSD1306_WHITE);
  if (filledWidth > 0) {
    displayDriver.fillRect(x + 1, y + 1, filledWidth, height - 2, SSD1306_WHITE);
  }
}

void OledDisplay::display() {
  displayDriver.display();
}
