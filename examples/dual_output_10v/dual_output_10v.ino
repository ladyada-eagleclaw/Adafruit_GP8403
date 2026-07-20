/*
 * Adafruit GP8403 Dual Output 10 V Example
 *
 * Selects the 0-10 V range and sets both output channels.
 */

#include <Adafruit_GP8403.h>

Adafruit_GP8403 gp8403;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Adafruit GP8403 Dual Output 10 V Example");

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

  if (!gp8403.setVoltages(2.50, 7.50)) {
    Serial.println("Could not set both output voltages");
    while (1) {
      delay(10);
    }
  }

  // getVoltage() returns cached values, not hardware readback.
  Serial.print("Channel 0: ");
  Serial.print(gp8403.getVoltage(0), 2);
  Serial.println(" V");
  Serial.print("Channel 1: ");
  Serial.print(gp8403.getVoltage(1), 2);
  Serial.println(" V");
}

void loop() { delay(1000); }
