# Adafruit GP8403 [![Build Status](https://github.com/adafruit/Adafruit_GP8403/workflows/Arduino%20Library%20CI/badge.svg)](https://github.com/adafruit/Adafruit_GP8403/actions)[![Documentation](https://github.com/adafruit/ci-arduino/blob/master/assets/doxygen_badge.svg)](http://adafruit.github.io/Adafruit_GP8403/html/index.html)

<img src="assets/board.png?raw=true" width="500px">

This is the Adafruit GP8403 dual-channel 12-bit voltage-output DAC library.

Tested and works great with the Adafruit GP8403 Breakout Board.

The GP8403 provides two analog voltage outputs with selectable 0-5 V and
0-10 V ranges. This chip uses I2C to communicate, so two pins are required to
interface.

Adafruit invests time and resources providing this open source code. Please
support Adafruit and open-source hardware by purchasing products from Adafruit!

Written by Limor Fried for Adafruit Industries.

MIT license, check `LICENSE` for more information.

All text above must be included in any redistribution.

To install, use the Arduino Library Manager and search for "Adafruit GP8403".

## Dependencies

This library depends on
[Adafruit BusIO](https://github.com/adafruit/Adafruit_BusIO). Arduino Library
Manager will install it automatically.

## Wiring

Connect the GP8403 SDA and SCL pins to the corresponding I2C pins on your
Arduino, and connect the grounds. The default I2C address is `0x58`. Pass a
different address to `begin()` if required by your hardware.

> **Warning:** The GP8403 outputs can reach 10 V. Protect low-voltage ADCs and
> other downstream inputs with a correctly sized voltage divider or another
> suitable interface. The library never selects 10 V mode automatically.

## Basic use

```cpp
#include <Adafruit_GP8403.h>

Adafruit_GP8403 gp8403;

void setup() {
  if (!gp8403.begin()) {
    while (1) {
      delay(10);
    }
  }

  gp8403.setOutputRange(GP8403_RANGE_5V);
  gp8403.setVoltage(0, 2.5);
}

void loop() {}
```

See the `examples` folder for single-output, dual-output, waveform, 5 V, 10 V,
and nonvolatile-memory examples.

`begin()` checks for the selected I2C address, selects the 5 V range, and sets
both outputs to 0 V. Changing the output range also clears both outputs before
sending the range command. This prevents an existing DAC code from unexpectedly
producing twice the voltage when changing from 5 V to 10 V mode.

`getOutputRange()`, `getRaw()`, and `getVoltage()` return the library's cached
requested state. They are not hardware readback because the GP8403 does not
provide readable registers.

Invalid channels, raw values above 4095, non-finite voltages, negative voltages,
and voltages above the selected range are rejected without an I2C write.

## Nonvolatile memory

`saveToNVM(SDA, SCL)` asks the GP8403 to save its current hardware output state.
Saving is never automatic. Do not call it repeatedly or from a loop; the
datasheet does not specify the nonvolatile-memory endurance.

The I2C peripheral must be released before saving and restored afterward:

```cpp
Wire.end();
bool saved = gp8403.saveToNVM(SDA, SCL);
Wire.begin();
```

If your application uses custom pins, speed, or other bus settings, restore the
same configuration instead of using the parameterless `Wire.begin()` shown
above.

The GP8403 uses a special open-drain waveform for nonvolatile-memory programming;
it cannot be sent as a normal `Wire` transaction. The library generates that
waveform on the supplied SDA and SCL pins and uses the datasheet's fixed `0x58`
programming address. A `true` result confirms that the programming waveform and
acknowledgments completed. Verify persistence after an actual power cycle.

The saved output voltage is restored as soon as the GP8403 powers up. Calling
`begin()` afterward intentionally replaces it by selecting the 5 V range and
setting both outputs to 0 V. Applications that need the restored output must use
or inspect it before calling `begin()`.
