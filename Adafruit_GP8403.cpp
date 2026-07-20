// SPDX-FileCopyrightText: 2026 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

/**
 * @file Adafruit_GP8403.cpp
 *
 * @mainpage Adafruit GP8403 Arduino Library
 *
 * @section intro_sec Introduction
 *
 * Arduino driver for the GP8403 dual-channel 12-bit voltage-output DAC.
 *
 * Written by Limor "ladyada" Fried for Adafruit Industries.
 */

#include "Adafruit_GP8403.h"

#include <Adafruit_BusIO_Register.h>

/**
 * @brief Construct a new GP8403 driver.
 */
Adafruit_GP8403::Adafruit_GP8403() {
  _i2c_dev = nullptr;
  _range = GP8403_RANGE_5V;
  _raw[0] = 0;
  _raw[1] = 0;
}

/**
 * @brief Destroy the GP8403 driver and release its BusIO device.
 */
Adafruit_GP8403::~Adafruit_GP8403() {
  delete _i2c_dev;
  _i2c_dev = nullptr;
}

/**
 * @brief Initialize the GP8403 in a known, safe state.
 *
 * The device is probed, placed in the 0 V to 5 V range, and both channels are
 * set to zero. The GP8403 has no readable identification register.
 *
 * @param address 7-bit I2C address.
 * @param wire I2C interface to use.
 * @return true if the probe and all initialization writes succeeded.
 */
bool Adafruit_GP8403::begin(uint8_t address, TwoWire* wire) {
  delete _i2c_dev;
  _i2c_dev = new Adafruit_I2CDevice(address, wire);

  if (_i2c_dev == nullptr || !_i2c_dev->begin()) {
    return false;
  }

  return setOutputRange(GP8403_RANGE_5V);
}

/**
 * @brief Select the output range after first setting both outputs to zero.
 *
 * @param range Desired output range.
 * @return true if both the zero write and range write succeeded.
 */
bool Adafruit_GP8403::setOutputRange(gp8403_output_range_t range) {
  if (!setRawValues(0, 0)) {
    return false;
  }

  return writeOutputRange(range);
}

/**
 * @brief Get the cached output range.
 *
 * @return Cached range; this is not hardware readback.
 */
gp8403_output_range_t Adafruit_GP8403::getOutputRange() {
  return _range;
}

/**
 * @brief Set one channel using a 12-bit raw DAC value.
 *
 * @param channel Output channel to update.
 * @param value Raw value from 0 through 4095.
 * @return true if the value was valid and the I2C write succeeded.
 */
bool Adafruit_GP8403::setRaw(uint8_t channel, uint16_t value) {
  if (channel > 1 || value > GP8403_MAX_RAW_VALUE) {
    return false;
  }

  return writeRaw(channel, value);
}

/**
 * @brief Set both channels in one I2C transaction.
 *
 * @param channel0 Raw channel 0 value from 0 through 4095.
 * @param channel1 Raw channel 1 value from 0 through 4095.
 * @return true if both values were valid and the I2C write succeeded.
 */
bool Adafruit_GP8403::setRawValues(uint16_t channel0, uint16_t channel1) {
  if (channel0 > GP8403_MAX_RAW_VALUE || channel1 > GP8403_MAX_RAW_VALUE) {
    return false;
  }

  return writeRawValues(channel0, channel1);
}

/**
 * @brief Get a channel's cached 12-bit DAC value.
 *
 * @param channel Output channel to query.
 * @return Cached raw value, or zero for an invalid channel.
 */
uint16_t Adafruit_GP8403::getRaw(uint8_t channel) {
  if (channel > 1) {
    return 0;
  }
  return _raw[channel];
}

/**
 * @brief Set one channel to a voltage in volts.
 *
 * @param channel Output channel to update.
 * @param volts Requested voltage in volts.
 * @return true if the request was valid and the I2C write succeeded.
 */
bool Adafruit_GP8403::setVoltage(uint8_t channel, float volts) {
  uint16_t value;
  if (channel > 1 || !voltageToRaw(volts, value)) {
    return false;
  }
  return writeRaw(channel, value);
}

/**
 * @brief Set both channels to voltages in one I2C transaction.
 *
 * @param channel0Volts Requested channel 0 voltage in volts.
 * @param channel1Volts Requested channel 1 voltage in volts.
 * @return true if both requests were valid and the I2C write succeeded.
 */
bool Adafruit_GP8403::setVoltages(float channel0Volts, float channel1Volts) {
  uint16_t channel0;
  uint16_t channel1;
  if (!voltageToRaw(channel0Volts, channel0) ||
      !voltageToRaw(channel1Volts, channel1)) {
    return false;
  }
  return writeRawValues(channel0, channel1);
}

/**
 * @brief Get a channel's cached, quantized voltage in volts.
 *
 * @param channel Output channel to query.
 * @return Cached quantized voltage, or NAN for an invalid channel.
 */
float Adafruit_GP8403::getVoltage(uint8_t channel) {
  if (channel > 1) {
    return NAN;
  }
  return (float)_raw[channel] * fullScaleVoltage() /
         (float)GP8403_MAX_RAW_VALUE;
}

/**
 * @brief Write the output-range command and update the cached range.
 *
 * @param range Output range to write.
 * @return true if the I2C write succeeded.
 */
bool Adafruit_GP8403::writeOutputRange(gp8403_output_range_t range) {
  if (_i2c_dev == nullptr) {
    return false;
  }

  uint8_t rangeData = GP8403_RANGE_DATA_5V;
  if (range == GP8403_RANGE_10V) {
    rangeData = GP8403_RANGE_DATA_10V;
  }
  Adafruit_BusIO_Register rangeRegister(_i2c_dev, GP8403_COMMAND_RANGE);
  if (!rangeRegister.write(rangeData)) {
    return false;
  }

  _range = range;
  return true;
}

/**
 * @brief Write one raw channel value and update its cached value.
 *
 * @param channel Output channel to write.
 * @param value Valid 12-bit raw value.
 * @return true if the I2C write succeeded.
 */
bool Adafruit_GP8403::writeRaw(uint8_t channel, uint16_t value) {
  if (_i2c_dev == nullptr) {
    return false;
  }

  uint16_t wireValue = value << 4;
  uint8_t command = GP8403_COMMAND_CHANNEL_0;
  if (channel == 1) {
    command = GP8403_COMMAND_CHANNEL_1;
  }
  Adafruit_BusIO_Register channelRegister(_i2c_dev, command, 2, LSBFIRST);
  if (!channelRegister.write(wireValue)) {
    return false;
  }

  _raw[channel] = value;
  return true;
}

/**
 * @brief Write both raw channel values and update their cached values.
 *
 * @param channel0 Valid channel 0 raw value.
 * @param channel1 Valid channel 1 raw value.
 * @return true if the I2C write succeeded.
 */
bool Adafruit_GP8403::writeRawValues(uint16_t channel0, uint16_t channel1) {
  if (_i2c_dev == nullptr) {
    return false;
  }

  uint16_t wireChannel0 = channel0 << 4;
  uint16_t wireChannel1 = channel1 << 4;
  uint32_t wireValues = ((uint32_t)wireChannel1 << 16) | wireChannel0;
  Adafruit_BusIO_Register channelRegister(_i2c_dev, GP8403_COMMAND_CHANNEL_0, 4,
                                          LSBFIRST);
  if (!channelRegister.write(wireValues)) {
    return false;
  }

  _raw[0] = channel0;
  _raw[1] = channel1;
  return true;
}

/**
 * @brief Convert a requested voltage to the nearest 12-bit DAC value.
 *
 * @param volts Requested voltage in volts.
 * @param value Destination for the converted value.
 * @return true if the voltage is finite and within the selected range.
 */
bool Adafruit_GP8403::voltageToRaw(float volts, uint16_t& value) {
  float fullScale = fullScaleVoltage();
  if (!isfinite(volts) || volts < 0.0 || volts > fullScale) {
    return false;
  }

  value = (uint16_t)(volts * (float)GP8403_MAX_RAW_VALUE / fullScale + 0.5);
  return true;
}

/**
 * @brief Get the selected full-scale voltage.
 *
 * @return 5.0 or 10.0 volts according to the cached range.
 */
float Adafruit_GP8403::fullScaleVoltage() {
  if (_range == GP8403_RANGE_10V) {
    return 10.0;
  }
  return 5.0;
}

namespace {

constexpr uint8_t GP8403_NVM_HALF_PERIOD_US = 5;

class GP8403NVMWriter {
 public:
  GP8403NVMWriter(uint16_t sdaPin, uint16_t sclPin) {
    _sdaPin = sdaPin;
    _sclPin = sclPin;
  }

  void releaseBus() {
    release(_sdaPin);
    release(_sclPin);
  }

  bool start() {
    releaseBus();
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    if (digitalRead(_sdaPin) != HIGH || digitalRead(_sclPin) != HIGH) {
      return false;
    }
    driveLow(_sdaPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    driveLow(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    return true;
  }

  bool stop() {
    driveLow(_sdaPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    release(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    bool clockHigh = digitalRead(_sclPin) == HIGH;
    release(_sdaPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    return clockHigh && digitalRead(_sdaPin) == HIGH;
  }

  bool writeBit(bool value) {
    driveLow(_sclPin);
    if (value) {
      release(_sdaPin);
    } else {
      driveLow(_sdaPin);
    }
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    release(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    bool clockHigh = digitalRead(_sclPin) == HIGH;
    bool dataHigh = !value || digitalRead(_sdaPin) == HIGH;
    driveLow(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    return clockHigh && dataHigh;
  }

  bool writeByte(uint8_t value) {
    bool clocksHigh = true;
    for (int8_t bit = 7; bit >= 0; bit--) {
      clocksHigh = writeBit(value & (1U << bit)) && clocksHigh;
    }
    return clocksHigh;
  }

  bool writeAcknowledgedByte(uint8_t value) {
    bool clocksHigh = writeByte(value);
    driveLow(_sclPin);
    release(_sdaPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    release(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    bool acknowledged =
        digitalRead(_sclPin) == HIGH && digitalRead(_sdaPin) == LOW;
    driveLow(_sclPin);
    delayMicroseconds(GP8403_NVM_HALF_PERIOD_US);
    return clocksHigh && acknowledged;
  }

  bool writePreamble() {
    if (!start()) {
      return false;
    }
    bool waveformOK = writeBit(false);
    waveformOK = writeBit(true) && waveformOK;
    waveformOK = writeBit(false) && waveformOK;
    return stop() && waveformOK;
  }

 private:
  static void driveLow(uint16_t pin) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }

  static void release(uint16_t pin) {
    pinMode(pin, INPUT);
  }

  uint16_t _sdaPin;
  uint16_t _sclPin;
};

} // namespace

/**
 * @brief Persist the current GP8403 output voltage data to NVM.
 *
 * The datasheet specifies a nonstandard bus waveform rather than an ordinary
 * I2C register write. This implementation uses portable open-drain GPIO
 * signaling on the supplied SDA and SCL pins. The caller must release the I2C
 * peripheral before this call and restore its configuration afterward.
 *
 * A true return value confirms that the waveform was emitted and the documented
 * entry and exit acknowledgments were observed. It cannot verify persistence
 * without a power cycle.
 *
 * @param sdaPin SDA pin used by the configured Wire interface.
 * @param sclPin SCL pin used by the configured Wire interface.
 * @return true if the complete waveform and acknowledged bytes succeeded;
 *         false if uninitialized, unsupported, or not acknowledged.
 */
bool Adafruit_GP8403::saveToNVM(uint16_t sdaPin, uint16_t sclPin) {
  if (_i2c_dev == nullptr || sdaPin == sclPin) {
    return false;
  }

  GP8403NVMWriter writer(sdaPin, sclPin);

  bool entryPreambleOK = writer.writePreamble();
  bool entryStartOK = entryPreambleOK && writer.start();
  bool entryCommandACK = false;
  bool entryDataACK = false;
  if (entryStartOK) {
    entryCommandACK = writer.writeAcknowledgedByte(0x10);
    entryDataACK = writer.writeAcknowledgedByte(0x03);
  }
  bool entryStopOK = entryStartOK && writer.stop();
  bool entered = entryPreambleOK && entryStartOK && entryCommandACK &&
                 entryDataACK && entryStopOK;

  bool dataWaveformOK = false;
  if (entered) {
    dataWaveformOK = writer.start();
    if (dataWaveformOK) {
      dataWaveformOK = writer.writeByte(0xB0) && dataWaveformOK;
      dataWaveformOK = writer.writeBit(true) && dataWaveformOK;
      for (uint8_t byte = 0; byte < 8; byte++) {
        dataWaveformOK = writer.writeByte(0x00) && dataWaveformOK;
        dataWaveformOK = writer.writeBit(true) && dataWaveformOK;
      }
      dataWaveformOK = writer.stop() && dataWaveformOK;
    }
  }

  if (entered) {
    delay(8);
  }

  // Attempt the complete exit pair even if entry acknowledgment failed, so a
  // partially accepted entry command receives its documented cleanup sequence.
  // Do not drive further if another device is holding the bus busy.
  bool exitPreambleOK = entryStartOK && writer.writePreamble();
  bool exitStartOK = false;
  bool exitCommandACK = false;
  bool exitDataACK = false;
  bool exitStopOK = false;
  if (exitPreambleOK) {
    exitStartOK = writer.start();
    if (exitStartOK) {
      exitCommandACK = writer.writeAcknowledgedByte(0x10);
      exitDataACK = writer.writeAcknowledgedByte(0x00);
      exitStopOK = writer.stop();
    }
  }
  bool exited = exitPreambleOK && exitStartOK && exitCommandACK &&
                exitDataACK && exitStopOK;
  writer.releaseBus();
  return entered && dataWaveformOK && exited;
}
