/*
 * Adafruit GP8403 Single Output 5 V Example
 *
 * Selects the 0-5 V range and sets channel 0 to 2.50 V.
 */

#include <Adafruit_GP8403.h>

Adafruit_GP8403 gp8403;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Adafruit GP8403 Single Output 5 V Example");

  if (!gp8403.begin()) {
    Serial.println("Could not find GP8403");
    while (1) {
      delay(10);
    }
  }

  if (!gp8403.setOutputRange(GP8403_RANGE_5V)) {
    Serial.println("Could not select the 5 V output range");
    while (1) {
      delay(10);
    }
  }

  if (!gp8403.setVoltage(0, 2.50)) {
    Serial.println("Could not set channel 0 voltage");
    while (1) {
      delay(10);
    }
  }

  // getVoltage() returns a cached value, not hardware readback.
  Serial.print("Channel 0: ");
  Serial.print(gp8403.getVoltage(0), 2);
  Serial.println(" V");
}

void loop() { delay(1000); }
