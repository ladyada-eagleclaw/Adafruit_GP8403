// SPDX-FileCopyrightText: 2026 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

/**
 * @file Adafruit_GP8403.h
 *
 * @brief Arduino driver for the GP8403 dual-channel voltage-output DAC.
 */

#ifndef ADAFRUIT_GP8403_H
#define ADAFRUIT_GP8403_H

#include <Adafruit_I2CDevice.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define GP8403_COMMAND_RANGE 0x01     ///< Output-range command
#define GP8403_COMMAND_CHANNEL_0 0x02 ///< Channel 0 data command
#define GP8403_COMMAND_CHANNEL_1 0x04 ///< Channel 1 data command
#define GP8403_RANGE_DATA_5V 0x00     ///< 0 V to 5 V range data
#define GP8403_RANGE_DATA_10V 0x11    ///< 0 V to 10 V range data
#define GP8403_DEFAULT_ADDRESS 0x58   ///< Default GP8403 I2C address
#define GP8403_MAX_RAW_VALUE 4095     ///< Maximum 12-bit DAC value

/** @brief GP8403 output voltage ranges. */
enum gp8403_output_range_t {
  GP8403_RANGE_5V,  ///< 0 V to 5 V output range
  GP8403_RANGE_10V, ///< 0 V to 10 V output range
};

/** @brief Driver for the GP8403 dual-channel 12-bit DAC. */
class Adafruit_GP8403 {
public:
  Adafruit_GP8403();
  ~Adafruit_GP8403();

  bool begin(uint8_t address = GP8403_DEFAULT_ADDRESS,
             TwoWire *wire = &Wire);

  bool setOutputRange(gp8403_output_range_t range);
  gp8403_output_range_t getOutputRange();

  bool setRaw(uint8_t channel, uint16_t value);
  bool setRawValues(uint16_t channel0, uint16_t channel1);
  uint16_t getRaw(uint8_t channel);

  bool setVoltage(uint8_t channel, float volts);
  bool setVoltages(float channel0Volts, float channel1Volts);
  float getVoltage(uint8_t channel);

  bool saveToNVM(uint16_t sdaPin, uint16_t sclPin);

private:
  bool writeOutputRange(gp8403_output_range_t range);
  bool writeRaw(uint8_t channel, uint16_t value);
  bool writeRawValues(uint16_t channel0, uint16_t channel1);
  bool voltageToRaw(float volts, uint16_t &value);
  float fullScaleVoltage();

  Adafruit_I2CDevice *_i2c_dev; ///< BusIO I2C device
  gp8403_output_range_t _range; ///< Cached output range
  uint16_t _raw[2];             ///< Cached channel DAC values
};

#endif
