/*!
 *  @file Adafruit_LPS35HW.h
 *
 * 	I2C Driver for LPS35HW Current and Power sensor
 *
 * 	This is a library for the Adafruit LPS35HW breakout:
 * 	http://www.adafruit.com/products
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *
 *	BSD license (see license.txt)
 */

#ifndef _ADAFRUIT_LPS35HW_H
#define _ADAFRUIT_LPS35HW_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Wire.h>

#define LPS35HW_I2CADDR_DEFAULT 0x5D ///< LPS35HW default i2c address
#define LPS35HW_INTERRUPT_CFG 0x0B   ///< Interrupt configuration register
#define LPS35HW_THS_P_L 0x0C         ///< Threshold pressure low byte
#define LPS35HW_THS_P_H 0x0D         ///< Threshold pressure high byte
#define LPS35HW_WHO_AM_I 0x0F        ///< Chip ID
#define LPS35HW_CTRL_REG1 0x10       ///< Control register 1
#define LPS35HW_CTRL_REG2 0x11       ///< Control register 2
#define LPS35HW_CTRL_REG3 0x12       ///< Control register 3
#define LPS35HW_FIFO_CTRL 0x14       ///< FIFO Control register
#define LPS35HW_REF_P_XL 0x15        ///< Reference pressure low byte
#define LPS35HW_REF_P_L 0x16         ///< Reference pressure mid byte
#define LPS35HW_REF_P_H 0x17         ///< Reference pressure high byte
#define LPS35HW_RPDS_L 0x18          ///< Offset pressure low byte
#define LPS35HW_RPDS_H 0x19          ///< Offset pressure high byte
#define LPS35HW_RES_CONF 0x1A        ///< Low power mode configuration
#define LPS35HW_INT_SOURCE 0x25      ///< Interrupt source
#define LPS35HW_FIFO_STATUS 0x26     ///< FIFO Status
#define LPS35HW_STATUS 0x27          ///< Status register
#define LPS35HW_PRESS_OUT_XL 0x28    ///< Pressure low byte
#define LPS35HW_PRESS_OUT_L 0x29     ///< Pressure mid byte
#define LPS35HW_PRESS_OUT_H 0x2A     ///< Pressure high byte
#define LPS35HW_TEMP_OUT_L 0x2B      ///< Temperature low byte
#define LPS35HW_TEMP_OUT_H 0x2C      ///< Temperature high byte
#define LPS35HW_LPFP_RES 0x33        ///< Low pass filter reset

/**
 * @brief Data rate options.
 *
 * Allowed values for `setDataRate`.
 */
typedef enum _data_rate {
  LPS35HW_RATE_ONE_SHOT, /**< ONE_SHOT: Put the sensor in a low power shutdown
                            mode that will only take a measurement when
                            `takeMeasurement` is called **/
  LPS35HW_RATE_1_HZ,     /** 1 hz  **/
  LPS35HW_RATE_10_HZ,    /** 10 hz  **/
  LPS35HW_RATE_25_HZ,    /** 25 hz  **/
  LPS35HW_RATE_50_HZ,    /** 50 hz  **/
  LPS35HW_RATE_75_HZ,    /** 75 hz  **/
} LPS35HW_DataRate;

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            LPS35HW Current and Power Sensor
 */
class Adafruit_LPS35HW {
public:
  Adafruit_LPS35HW();
  boolean begin_I2C(uint8_t i2c_addr = LPS35HW_I2CADDR_DEFAULT,
                    TwoWire *wire = &Wire);
  boolean begin_SPI(uint8_t cs_pin, SPIClass *theSPI = &SPI);
  boolean begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                    int8_t mosi_pin);
  void reset(void);
  float readTemperature(void);
  float readPressure(void);
  void setDataRate(LPS35HW_DataRate new_rate);
  void takeMeasurement(void);
  void zeroPressure(void);
  void resetPressure(void);
  void setThresholdPressure(float threshold_pressure);
  void enableHighThreshold(void);
  void enableLowThreshold(void);
  bool highThresholdExceeded(void);
  bool lowThresholdExceeded(void);
  void enableInterrupts(bool active_low = false, bool open_drain = false);
  void disableInterrupts(void);
  void enableLowPass(bool extra_low_bandwidth = false);

  Adafruit_BusIO_Register *Config1, ///< BusIO Register for CONFIG_1
      *Config2,                     ///< BusIO Register for CONFIG_2
      *Config3,                     ///< BusIO Register for CONFIG_3
      *InterruptCfg,                ///< BusIO Register for INTERRUPT_CFG
      *InterruptStatus;             ///< BusIO Register for INTERRUPT_STATUS

private:
  bool _init(void);

  Adafruit_I2CDevice *i2c_dev;
  Adafruit_SPIDevice *spi_dev;
};

#endif
