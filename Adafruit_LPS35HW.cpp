/*!
 *  @file Adafruit_LPS35HW.cpp
 *
 *  @mainpage Adafruit LPS35HW I2C water resistant barometric pressure sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the LPS35HW I2C water resistant barometric pressure sensor
 *
 * 	This is a library for the Adafruit LPS35HW breakout:
 * 	http://www.adafruit.com/products
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section dependencies Dependencies
 *
 *  This library depends on the Adafruit BusIO library
 *
 *  @section author Author
 *
 *  Bryan Siepert for Adafruit Industries
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Arduino.h"
#include <Wire.h>

#include "Adafruit_LPS35HW.h"

/*!
 *    @brief  Instantiates a new LPS35HW class
 */
Adafruit_LPS35HW::Adafruit_LPS35HW(void) {}

/*!
 *    @brief  Sets up the hardware and initializes I2C
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  wire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_LPS35HW::begin(uint8_t i2c_address, TwoWire *wire) {
  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }
  Adafruit_I2CRegister chip_id = Adafruit_I2CRegister(i2c_dev, LPS35HW_WHO_AM_I, 1);


  // make sure we're talking to the right chip
  if (chip_id.read() != 0xB1) {
    return false;
  }


  Config1 = new Adafruit_I2CRegister(i2c_dev, LPS35HW_CTRL_REG1, 1);
  Config2 = new Adafruit_I2CRegister(i2c_dev, LPS35HW_CTRL_REG2, 1);
  Config3 = new Adafruit_I2CRegister(i2c_dev, LPS35HW_CTRL_REG3, 1);
  InterruptCfg = new Adafruit_I2CRegister(i2c_dev, LPS35HW_INTERRUPT_CFG, 1);
  InterruptStatus = new Adafruit_I2CRegister(i2c_dev, LPS35HW_INT_SOURCE, 1);

  reset();

  setDataRate(LPS35HW_RATE_10_HZ); // default in continuous at 10 hz
  // setup block reads
  Adafruit_I2CRegisterBits block_reads =
  Adafruit_I2CRegisterBits(Config1, 1, 1);
  block_reads.write(0x1);

  return true;
}
/**************************************************************************/
/*!
    @brief Resets the hardware. All configuration registers are set to
            default values, the same as a power-on reset.
*/
/**************************************************************************/
void Adafruit_LPS35HW::reset(void){
  Adafruit_I2CRegisterBits reset =
    Adafruit_I2CRegisterBits(Config2, 1, 2);
  reset.write(1);
  while(reset.read() == true){
    delay(1);
  }
}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the temperature register.
    @return The current temperature in degrees C
*/
/**************************************************************************/
float Adafruit_LPS35HW::readTemperature(void) {
  Adafruit_I2CRegister temp =
    Adafruit_I2CRegister(i2c_dev, LPS35HW_TEMP_OUT_L, 2);
  return (int16_t)temp.read() / 100.0;
}
/**************************************************************************/
/*!
    @brief Reads and scales the value of the pressure register.
    @return The current pressure in hPa, relative to the reference temperature
*/
/**************************************************************************/
float Adafruit_LPS35HW::readPressure(void) {
  Adafruit_I2CRegister pressure =
    Adafruit_I2CRegister(i2c_dev, LPS35HW_PRESS_OUT_XL, 3);
    int32_t raw_pressure = pressure.read();
    // perform sign extension for 24 bit number if needed
    if (raw_pressure & 0x800000){
      raw_pressure = (0xff000000 | raw_pressure);
    }

  return (raw_pressure / 4096.0);
}

/**************************************************************************/
/*!
    @brief Takes a new measurement while in one shot mode.
*/
/**************************************************************************/
void Adafruit_LPS35HW::takeMeasurement(void) {
  Adafruit_I2CRegisterBits one_shot =
    Adafruit_I2CRegisterBits(Config2, 1, 0);
  one_shot.write(1);
  while(one_shot.read() == true){ delay(1);};
}

/**************************************************************************/
/*!
    @brief Sets the reference temperature to the current temperature. Future
            pressure readings will be relative to it until `resetPressure` is
            called.
*/
/**************************************************************************/
void Adafruit_LPS35HW::zeroPressure(void) {
  Adafruit_I2CRegisterBits zero_pressure =
    Adafruit_I2CRegisterBits(InterruptCfg, 1, 5);
  zero_pressure.write(1);
  while(zero_pressure.read() == true){ delay(1);};
}

/**************************************************************************/
/*!
    @brief Resets the reference pressure to zero so calls to `getPressure`
            are reported as the absolute value.
*/
/**************************************************************************/
void Adafruit_LPS35HW::resetPressure(void) {
  Adafruit_I2CRegisterBits pressure_reset =
    Adafruit_I2CRegisterBits(InterruptCfg, 1, 4);
  pressure_reset.write(1);
}

/**************************************************************************/
/*!
    @brief Sets the pressure threshold used by the high and low pressure thresholds
    @param threshold_pressure
            The threshold pressure in hPa, measured from zero
*/
/**************************************************************************/
void Adafruit_LPS35HW::setThresholdPressure(float threshold_pressure) {
  Adafruit_I2CRegister threshold = Adafruit_I2CRegister(i2c_dev, LPS35HW_THS_P_L, 2);
  threshold.write(threshold_pressure * 16);
}
/**************************************************************************/
/*!
    @brief Enables high pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableHighThreshold(void) {
  Adafruit_I2CRegisterBits high_thresh = Adafruit_I2CRegisterBits(InterruptCfg, 1, 0);
  high_thresh.write(0x1);
  Adafruit_I2CRegisterBits high_int_pin = Adafruit_I2CRegisterBits(Config3, 1, 0);
  high_int_pin.write(0x1);
}
/**************************************************************************/
/*!
    @brief Disables low pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableLowThreshold(void) {
  Adafruit_I2CRegisterBits low_thresh = Adafruit_I2CRegisterBits(InterruptCfg, 1, 1);
  low_thresh.write(0x1);
  Adafruit_I2CRegisterBits low_int_pin = Adafruit_I2CRegisterBits(Config3, 1, 1);
  low_int_pin.write(0x1);
}
/**************************************************************************/
/*!
    @brief Enables pressure threshold interrupts. High and low thresholds
          need to be enabled individually with `enableLowThreshold` and
          `enableHighThreshold`.
    @param open_drain
          Set to `true` to have the INT pin be open drain when active.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableInterrupts(bool open_drain){
  if (open_drain){
    Adafruit_I2CRegisterBits pin_mode = Adafruit_I2CRegisterBits(Config3, 2, 6);
    pin_mode.write(0x3);
  }
  Adafruit_I2CRegisterBits latch_enabled = Adafruit_I2CRegisterBits(InterruptCfg, 2, 2);
  latch_enabled.write(0x3);
}
/**************************************************************************/
/*!
    @brief Disables pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::disableInterrupts(void){
  Adafruit_I2CRegisterBits enabled = Adafruit_I2CRegisterBits(InterruptCfg, 2, 2);
  enabled.write(0x0);

}
/**************************************************************************/
/*!
    @brief Enables the low pass filter with ODR/9 bandwidth
    @param extra_low_bandwidth
            Set to `true` to scale the bandwidth to ODR/20
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableLowPass(bool extra_low_bandwidth){
  Adafruit_I2CRegisterBits filter_config =
    Adafruit_I2CRegisterBits(Config1, 2, 2);
  filter_config.write(0x2 | (extra_low_bandwidth == true));
}

/**************************************************************************/
/*!
    @brief Returns the current state of the high pressure threshold interrupt.
    @return `true` if the high pressure threshold has been triggered since
          last checked.
*/
/**************************************************************************/
bool Adafruit_LPS35HW::highThresholdExceeded(void) {
  return (InterruptStatus->read() == 0b101);
}
/**************************************************************************/
/*!
    @brief Returns the current state of the low pressure threshold interrupt.
    @return `true` if the low pressure threshold has been triggered since
          last checked.
*/
/**************************************************************************/
bool Adafruit_LPS35HW::lowThresholdExceeded(void) {
  return (InterruptStatus->read() == 0b110);
}
/**************************************************************************/
/*!
    @brief Sets a new measurement rate
    @param new_rate
          The new output data rate to be set (ODR)
*/
/**************************************************************************/
void Adafruit_LPS35HW::setDataRate(LPS35HW_DataRate new_rate) {
  Adafruit_I2CRegisterBits data_rate =
    Adafruit_I2CRegisterBits(Config1, 3, 4);
  data_rate.write(new_rate);
}