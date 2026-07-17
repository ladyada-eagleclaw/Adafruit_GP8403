/*
 * Adafruit GP8403 NVM Persistence Example
 *
 * The GP8403 can remember its output voltages after power is removed, but its
 * nonvolatile memory (NVM) programming sequence is unusual. Please read all
 * of the warnings below before running this example.
 *
 * WARNING: Do not put saveToNVM() in loop(). The datasheet does not specify
 * the NVM write endurance, so save only when the output really must change.
 *
 * WARNING: NVM programming uses a special open-drain waveform, not a normal
 * I2C transaction. Wire must be stopped before saveToNVM() and restarted
 * afterward.
 *
 * WARNING: NVM programming always uses the GP8403's hardcoded 0x58 address.
 * It cannot save a device configured at another I2C address.
 *
 * WARNING: A saved voltage appears as soon as the GP8403 powers up. Calling
 * begin() selects the 5 V range and clears both outputs, so inspect any saved
 * output before calling begin(). This example pauses first to allow that.
 *
 * WARNING: A successful saveToNVM() call confirms that the GP8403 acknowledged
 * the programming sequence. The saved output can only be verified by removing
 * power completely, powering the GP8403 again, and measuring its outputs.
 */

#include <Adafruit_GP8403.h>

Adafruit_GP8403 gp8403;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("Adafruit GP8403 NVM Persistence Example"));
  Serial.println();
  Serial.println(F("Any previously saved voltages are active now."));
  Serial.println(F("Measure the outputs before continuing."));
  Serial.println(F("Send C to call begin() and clear the outputs."));
  waitForCommand('C');

  if (!gp8403.begin()) {
    Serial.println(F("Could not find GP8403 at address 0x58"));
    halt();
  }

  // begin() selects the 0-5 V range and sets both outputs to 0 V.
  // Change the range here first if your application needs 0-10 V operation.
  if (!gp8403.setVoltages(1.0, 2.0)) {
    Serial.println(F("Could not set the output voltages"));
    halt();
  }

  Serial.println();
  Serial.println(F("Channel 0 is now 1.0 V and channel 1 is now 2.0 V."));
  Serial.println(F("WARNING: The next step writes nonvolatile memory."));
  Serial.println(F("Send S to save these voltages, or reset to cancel."));
  waitForCommand('S');

  // saveToNVM() temporarily controls SDA and SCL directly as open-drain GPIO.
  // Stop Wire first so the I2C peripheral releases those pins. If your project
  // uses custom pins or bus settings, restore that same configuration below.
  Wire.end();
  bool saved = gp8403.saveToNVM(SDA, SCL);
  Wire.begin();

  if (!saved) {
    Serial.println(F("NVM programming was not acknowledged"));
    Serial.println(F("Check the wiring, pull-ups, and device address."));
    halt();
  }

  Serial.println();
  Serial.println(F("NVM programming was acknowledged."));
  Serial.println(F("Remove power completely, then power up and measure."));
  Serial.println(F("This example will pause before begin() on the next boot."));
  halt();
}

void loop() {}

void waitForCommand(char command) {
  while (true) {
    if (Serial.available() && Serial.read() == command) {
      return;
    }
    delay(10);
  }
}

void halt() {
  while (true) {
    delay(1000);
  }
}
