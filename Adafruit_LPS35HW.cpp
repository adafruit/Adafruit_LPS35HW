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
 *    @brief  Sets up the HW
 *    @param  i2c_address
 *            The I2C address to be used.
 *    @param  theWire
 *            The Wire object to be used for I2C connections.
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_LPS35HW::begin(uint8_t i2c_address, TwoWire *theWire) {
  i2c_dev = new Adafruit_I2CDevice(i2c_address);

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
  // delay(2); // delay 2ms to give time for first measurement to finish
  return true;
}
/**************************************************************************/
/*!
    @brief Resets the harware. All registers are set to default values,
      the same as a power-on reset.
*/
/**************************************************************************/
void Adafruit_LPS35HW::reset(void){
  Adafruit_I2CRegisterBits reset =
    Adafruit_I2CRegisterBits(Config2, 1, 2);
  reset.write(1);
  while(reset.read() == 1){
    delay(1);
  }

  // we may also want to clear other registers not cleared by the soft reset
  setDataRate(LPS35HW_RATE_10_hz); // default in continuous at 10 hz
  // setup block reads
  Adafruit_I2CRegisterBits block_reads =
    Adafruit_I2CRegisterBits(Config1, 1, 1);
  block_reads.write(0x1);

}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Current register.
    @return The current current measurement in mA
*/
/**************************************************************************/
float Adafruit_LPS35HW::readTemperature(void) {
  Adafruit_I2CRegister temp =
    Adafruit_I2CRegister(i2c_dev, LPS35HW_TEMP_OUT_L, 2);
  return (int16_t)temp.read() / 100.0;
}
/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Current register.
    @return The current current measurement in mA
*/
/**************************************************************************/
float Adafruit_LPS35HW::readPressure(void) {
  Adafruit_I2CRegister pressure =
    Adafruit_I2CRegister(i2c_dev, LPS35HW_PRESS_OUT_XL, 3);
    int32_t raw_pressure = pressure.read();
    if (raw_pressure & 0x800000){
      raw_pressure = (0xff000000 | raw_pressure);
    }

  return (raw_pressure / 4096.0);
}

/**************************************************************************/
/*!
    @brief Reads and scales the current value of the Bus Voltage register.
    @return The current bus voltage measurement in mV
*/
/**************************************************************************/
void Adafruit_LPS35HW::takeMeasurement(void) {
  Adafruit_I2CRegisterBits one_shot =
    Adafruit_I2CRegisterBits(Config2, 1, 0);
  one_shot.write(1);
  while(one_shot.read() == 1){ delay(1);};
}

/**************************************************************************/
/*!
    @brief Set the current pressure as zero and report the pressure relative to it
*/
/**************************************************************************/
void Adafruit_LPS35HW::zeroPressure(void) {
  Adafruit_I2CRegisterBits zero_pressure =
    Adafruit_I2CRegisterBits(InterruptCfg, 1, 5);
  zero_pressure.write(1);
  while(zero_pressure.read() == 1){ delay(1);};
}

/**************************************************************************/
/*!
    @brief Reset pressure to be reported as the measured absolute value
*/
/**************************************************************************/
void Adafruit_LPS35HW::resetPressure(void) {
  Adafruit_I2CRegisterBits pressure_reset =
    Adafruit_I2CRegisterBits(InterruptCfg, 1, 4);
  pressure_reset.write(1);
}

// /**************************************************************************/
// /*!
//     @brief Reads and scales the current value of the Power register.
//     @return The current Power calculation in mW
// */
void Adafruit_LPS35HW::setThresholdPressure(float threshold_pressure) {
  Adafruit_I2CRegister threshold = Adafruit_I2CRegister(i2c_dev, LPS35HW_THS_P_L, 2);
  threshold.write(threshold_pressure);
}

void Adafruit_LPS35HW::enableHighThreshold(void) {
  Adafruit_I2CRegisterBits high_thresh = Adafruit_I2CRegisterBits(InterruptCfg, 1, 0);
  high_thresh.write(0x1); // interrupt on high threshold
  Adafruit_I2CRegisterBits high_int_pin = Adafruit_I2CRegisterBits(Config3, 1, 0);
  high_int_pin.write(0x1);
}

void Adafruit_LPS35HW::enableLowThreshold(void) {
  Adafruit_I2CRegisterBits low_thresh = Adafruit_I2CRegisterBits(InterruptCfg, 1, 1);
  low_thresh.write(0x1);
  Adafruit_I2CRegisterBits low_int_pin = Adafruit_I2CRegisterBits(Config3, 1, 1);
  low_int_pin.write(0x1);
}

void Adafruit_LPS35HW::enableInterrupts(bool active_low){
  if (active_low){
    Adafruit_I2CRegisterBits pin_mode = Adafruit_I2CRegisterBits(Config3, 2, 6);
    pin_mode.write(0x3);
  }
  Adafruit_I2CRegisterBits latch_enabled = Adafruit_I2CRegisterBits(InterruptCfg, 2, 2);
  latch_enabled.write(0x3);
}
void Adafruit_LPS35HW::disableInterrupts(void){

}
void Adafruit_LPS35HW::enableLowPass(bool extra_low_bandwidth){
  Adafruit_I2CRegisterBits filter_config =
    Adafruit_I2CRegisterBits(Config1, 2, 2);
  // this is assuming bool true is 1
  filter_config.write(0x2 | extra_low_bandwidth);
}


bool Adafruit_LPS35HW::highThresholdExceeded(void) {
  Adafruit_I2CRegisterBits high_threshold = Adafruit_I2CRegisterBits(InterruptStatus, 1, 0);
  return (high_threshold.read() == 1);
}

bool Adafruit_LPS35HW::lowThresholdExceeded(void) {
  Adafruit_I2CRegisterBits low_threshold = Adafruit_I2CRegisterBits(InterruptStatus, 1, 1);
  return (low_threshold.read() == 1);
}
/**************************************************************************/
/*!
    @brief Sets a new measurement rate
    @param new_rate
           The new data rate to be set
*/
/**************************************************************************/
void Adafruit_LPS35HW::setDataRate(LPS35HW_DataRate new_rate) {
  Adafruit_I2CRegisterBits data_rate =
    Adafruit_I2CRegisterBits(Config1, 3, 4);
  data_rate.write(new_rate);
}
// /**************************************************************************/
// /*!
//     @brief Reads the current number of averaging samples
//     @return The current number of averaging samples
// */
// /**************************************************************************/
// LPS35HW_AveragingCount Adafruit_LPS35HW::getAveragingCount(void){
//   Adafruit_I2CRegisterBits averaging_count =
//     Adafruit_I2CRegisterBits(Config, 3, 9);
//   return averaging_count.read();
// }
// /**************************************************************************/
// /*!
//     @brief Sets the number of averaging samples
//     @param count
//            The number of samples to be averaged
// */
// /**************************************************************************/
// void Adafruit_LPS35HW::setAveragingCount(LPS35HW_AveragingCount count){
//   Adafruit_I2CRegisterBits averaging_count =
//     Adafruit_I2CRegisterBits(Config, 3, 9);
//   averaging_count.write(count);
// }
// /**************************************************************************/
// /*!
//     @brief Reads the current current conversion time
//     @return The current current conversion time
// */
// /**************************************************************************/
// LPS35HW_ConversionTime Adafruit_LPS35HW::getCurrentConversionTime(void){
//   Adafruit_I2CRegisterBits current_conversion_time =
//     Adafruit_I2CRegisterBits(Config, 3, 3);
//   return current_conversion_time.read();
// }
// /**************************************************************************/
// /*!
//     @brief Sets the current conversion time
//     @param time
//            The new current conversion time
// */
// /**************************************************************************/
// void Adafruit_LPS35HW::setCurrentConversionTime(LPS35HW_ConversionTime time){
//   Adafruit_I2CRegisterBits current_conversion_time =
//     Adafruit_I2CRegisterBits(Config, 3, 3);
//   current_conversion_time.write(time);
// }
// /**************************************************************************/
// /*!
//     @brief Reads the current bus voltage conversion time
//     @return The current bus voltage conversion time
// */
// /**************************************************************************/
// LPS35HW_ConversionTime Adafruit_LPS35HW::getVoltageConversionTime(void){
//   Adafruit_I2CRegisterBits voltage_conversion_time =
//     Adafruit_I2CRegisterBits(Config, 3, 6);
//   return voltage_conversion_time.read();
// }
// /**************************************************************************/
// /*!
//     @brief Sets the bus voltage conversion time
//     @param time
//            The new bus voltage conversion time
// */
// /**************************************************************************/
// void Adafruit_LPS35HW::setVoltageConversionTime(LPS35HW_ConversionTime time){
//   Adafruit_I2CRegisterBits voltage_conversion_time =
//     Adafruit_I2CRegisterBits(Config, 3, 6);
//   voltage_conversion_time.write(time);
// }

// /**************************************************************************/
// /*!
//     @brief Checks if the most recent one shot measurement has completed
//     @return true if the conversion has completed
// */
// /**************************************************************************/
// bool Adafruit_LPS35HW::conversionReady(void){
//   Adafruit_I2CRegisterBits conversion_ready =
//     Adafruit_I2CRegisterBits(MaskEnable, 1, 3);
//   return conversion_ready.read();
// }
