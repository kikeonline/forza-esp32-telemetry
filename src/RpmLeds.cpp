#include "RpmLeds.h"

RpmLeds::RpmLeds(uint8_t bluePin, uint8_t greenPin, uint8_t yellowPin, uint8_t redPin)
    : bluePin_(bluePin), greenPin_(greenPin), yellowPin_(yellowPin), redPin_(redPin) {}

void RpmLeds::begin() {
  pinMode(bluePin_, OUTPUT);
  pinMode(greenPin_, OUTPUT);
  pinMode(yellowPin_, OUTPUT);
  pinMode(redPin_, OUTPUT);
  off();
}

void RpmLeds::update(float rpmPercent, unsigned long nowMs) {
  rpmPercent = constrain(rpmPercent, 0.0f, 1.0f);

  const bool blue = rpmPercent >= BlueThreshold;
  const bool green = rpmPercent >= GreenThreshold;
  const bool yellow = rpmPercent >= YellowThreshold;
  bool red = rpmPercent >= RedThreshold;

  // Near the limiter, blink the red LED while keeping the lower stages on.
  if (rpmPercent >= RedBlinkThreshold) {
    if (nowMs - lastBlinkToggleMs_ >= BlinkIntervalMs) {
      lastBlinkToggleMs_ = nowMs;
      redBlinkState_ = !redBlinkState_;
    }
    red = redBlinkState_;
  } else {
    redBlinkState_ = true;
    lastBlinkToggleMs_ = nowMs;
  }

  write(blue, green, yellow, red);
}

void RpmLeds::off() {
  redBlinkState_ = false;
  lastBlinkToggleMs_ = millis();
  write(false, false, false, false);
}

void RpmLeds::startupTest(unsigned int stepDelayMs) {
  write(true, false, false, false);
  delay(stepDelayMs);
  write(false, true, false, false);
  delay(stepDelayMs);
  write(false, false, true, false);
  delay(stepDelayMs);
  write(false, false, false, true);
  delay(stepDelayMs);
  off();
}

void RpmLeds::write(bool blue, bool green, bool yellow, bool red) {
  digitalWrite(bluePin_, blue ? HIGH : LOW);
  digitalWrite(greenPin_, green ? HIGH : LOW);
  digitalWrite(yellowPin_, yellow ? HIGH : LOW);
  digitalWrite(redPin_, red ? HIGH : LOW);
}
