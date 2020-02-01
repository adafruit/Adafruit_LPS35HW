/*!
 *  @file Adafruit_LPS35HW.cpp
 *
 *  @mainpage Adafruit LPS35HW I2C water resistant barometric pressure sensor
 *
 *  @section intro_sec Introduction
 *
 * 	I2C Driver for the LPS35HW I2C water resistant barometric pressure
 * sensor
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
boolean Adafruit_LPS35HW::begin_I2C(uint8_t i2c_address, TwoWire *wire) {
  spi_dev = NULL;
  i2c_dev = new Adafruit_I2CDevice(i2c_address, wire);

  if (!i2c_dev->begin()) {
    return false;
  }

  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_LPS35HW::begin_SPI(uint8_t cs_pin, SPIClass *theSPI) {
  i2c_dev = NULL;
  spi_dev = new Adafruit_SPIDevice(cs_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0,             // data mode
                                   theSPI);

  if (!spi_dev->begin()) {
    return false;
  }

  return _init();
}

/*!
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_LPS35HW::begin_SPI(int8_t cs_pin, int8_t sck_pin,
                                    int8_t miso_pin, int8_t mosi_pin) {
  i2c_dev = NULL;
  spi_dev = new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                                   1000000,               // frequency
                                   SPI_BITORDER_MSBFIRST, // bit order
                                   SPI_MODE0);            // data mode

  if (!spi_dev->begin()) {
    return false;
  }

  return _init();
}

boolean Adafruit_LPS35HW::_init(void) {
  Adafruit_BusIO_Register chip_id = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_WHO_AM_I, 1);

  // make sure we're talking to the right chip
  if (chip_id.read() != 0xB1) {
    return false;
  }

  Config1 = new Adafruit_BusIO_Register(i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD,
                                        LPS35HW_CTRL_REG1, 1);
  Config2 = new Adafruit_BusIO_Register(i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD,
                                        LPS35HW_CTRL_REG2, 1);
  Config3 = new Adafruit_BusIO_Register(i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD,
                                        LPS35HW_CTRL_REG3, 1);
  InterruptCfg = new Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_INTERRUPT_CFG, 1);
  InterruptStatus = new Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_INT_SOURCE, 1);

  reset();

  setDataRate(LPS35HW_RATE_10_HZ); // default in continuous at 10 hz
  // setup block reads
  Adafruit_BusIO_RegisterBits block_reads =
      Adafruit_BusIO_RegisterBits(Config1, 1, 1);
  block_reads.write(0x1);

  return true;
}

/**************************************************************************/
/*!
    @brief Resets the hardware. All configuration registers are set to
            default values, the same as a power-on reset.
*/
/**************************************************************************/
void Adafruit_LPS35HW::reset(void) {
  Adafruit_BusIO_RegisterBits reset =
      Adafruit_BusIO_RegisterBits(Config2, 1, 2);
  reset.write(1);
  while (reset.read() == true) {
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
  Adafruit_BusIO_Register temp = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_TEMP_OUT_L, 2);
  return (int16_t)temp.read() / 100.0;
}
/**************************************************************************/
/*!
    @brief Reads and scales the value of the pressure register.
    @return The current pressure in hPa, relative to the reference temperature
*/
/**************************************************************************/
float Adafruit_LPS35HW::readPressure(void) {
  Adafruit_BusIO_Register pressure = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_PRESS_OUT_XL, 3);
  int32_t raw_pressure = pressure.read();
  // perform sign extension for 24 bit number if needed
  if (raw_pressure & 0x800000) {
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
  Adafruit_BusIO_RegisterBits one_shot =
      Adafruit_BusIO_RegisterBits(Config2, 1, 0);
  one_shot.write(1);
  while (one_shot.read() == true) {
    delay(1);
  };
}

/**************************************************************************/
/*!
    @brief Sets the reference temperature to the current temperature. Future
            pressure readings will be relative to it until `resetPressure` is
            called.
*/
/**************************************************************************/
void Adafruit_LPS35HW::zeroPressure(void) {
  Adafruit_BusIO_RegisterBits zero_pressure =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 1, 5);
  zero_pressure.write(1);
  while (zero_pressure.read() == true) {
    delay(1);
  };
}

/**************************************************************************/
/*!
    @brief Resets the reference pressure to zero so calls to `getPressure`
            are reported as the absolute value.
*/
/**************************************************************************/
void Adafruit_LPS35HW::resetPressure(void) {
  Adafruit_BusIO_RegisterBits pressure_reset =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 1, 4);
  pressure_reset.write(1);
}

/**************************************************************************/
/*!
    @brief Sets the pressure threshold used by the high and low pressure
   thresholds
    @param threshold_pressure
            The threshold pressure in hPa, measured from zero
*/
/**************************************************************************/
void Adafruit_LPS35HW::setThresholdPressure(float threshold_pressure) {
  Adafruit_BusIO_Register threshold = Adafruit_BusIO_Register(
      i2c_dev, spi_dev, ADDRBIT8_HIGH_TOREAD, LPS35HW_THS_P_L, 2);
  threshold.write(threshold_pressure * 16);
}
/**************************************************************************/
/*!
    @brief Enables high pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableHighThreshold(void) {
  Adafruit_BusIO_RegisterBits high_thresh =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 1, 0);
  high_thresh.write(0x1);
  Adafruit_BusIO_RegisterBits high_int_pin =
      Adafruit_BusIO_RegisterBits(Config3, 1, 0);
  high_int_pin.write(0x1);
}
/**************************************************************************/
/*!
    @brief Disables low pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableLowThreshold(void) {
  Adafruit_BusIO_RegisterBits low_thresh =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 1, 1);
  low_thresh.write(0x1);
  Adafruit_BusIO_RegisterBits low_int_pin =
      Adafruit_BusIO_RegisterBits(Config3, 1, 1);
  low_int_pin.write(0x1);
}
/**************************************************************************/
/*!
    @brief Enables pressure threshold interrupts. High and low thresholds
          need to be enabled individually with `enableLowThreshold` and
          `enableHighThreshold`.
    @param active_low Polarity of interrupt pin, true for active low.
    @param open_drain
          Set to `true` to have the INT pin be open drain when active.
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableInterrupts(bool active_low, bool open_drain) {
  Adafruit_BusIO_RegisterBits pin_mode =
      Adafruit_BusIO_RegisterBits(Config3, 2, 6);
  pin_mode.write((active_low << 1) | open_drain);

  Adafruit_BusIO_RegisterBits latch_enabled =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 2, 2);
  latch_enabled.write(0x3);
}
/**************************************************************************/
/*!
    @brief Disables pressure threshold interrupts.
*/
/**************************************************************************/
void Adafruit_LPS35HW::disableInterrupts(void) {
  Adafruit_BusIO_RegisterBits enabled =
      Adafruit_BusIO_RegisterBits(InterruptCfg, 2, 2);
  enabled.write(0x0);
}
/**************************************************************************/
/*!
    @brief Enables the low pass filter with ODR/9 bandwidth
    @param extra_low_bandwidth
            Set to `true` to scale the bandwidth to ODR/20
*/
/**************************************************************************/
void Adafruit_LPS35HW::enableLowPass(bool extra_low_bandwidth) {
  Adafruit_BusIO_RegisterBits filter_config =
      Adafruit_BusIO_RegisterBits(Config1, 2, 2);
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
  Adafruit_BusIO_RegisterBits data_rate =
      Adafruit_BusIO_RegisterBits(Config1, 3, 4);
  data_rate.write(new_rate);
}
