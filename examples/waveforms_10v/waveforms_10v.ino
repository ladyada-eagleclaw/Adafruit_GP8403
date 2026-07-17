/*
 * Adafruit GP8403 Dual Waveforms 10 V Example
 *
 * Generates a 0-10 V sine wave on channel 0 and a 0-10 V triangle wave on
 * channel 1.
 */

#include <Adafruit_GP8403.h>
#include <math.h>

Adafruit_GP8403 gp8403;
uint8_t step = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Adafruit GP8403 Dual Waveforms 10 V Example");

  if (!gp8403.begin()) {
    Serial.println("Could not find GP8403");
    while (1) {
      delay(10);
    }
  }

  if (!gp8403.setOutputRange(GP8403_RANGE_10V)) {
    Serial.println("Could not select the 10 V output range");
    while (1) {
      delay(10);
    }
  }
}

void loop() {
  // Convert steps 0 through 99 into radians for one complete sine cycle.
  // 6.283185307 radians is 2 * pi, or one full circle.
  float phase = step * 6.283185307 / 100.0;

  // sin() produces -1 to +1. Scaling by 5 and adding 5 makes it 0 to 10 V.
  float sineVoltage = 5.0 + 5.0 * sin(phase);

  float triangleVoltage;
  if (step < 50) {
    // Rise from 0 to 10 V during the first 50 steps.
    triangleVoltage = step * 10.0 / 50.0;
  } else {
    // Fall from 10 V back to 0 V during the last 50 steps.
    triangleVoltage = (100 - step) * 10.0 / 50.0;
  }

  if (!gp8403.setVoltages(sineVoltage, triangleVoltage)) {
    Serial.println("Could not update output voltages");
    while (1) {
      delay(10);
    }
  }

  step = (step + 1) % 100;
  delay(10);
}
