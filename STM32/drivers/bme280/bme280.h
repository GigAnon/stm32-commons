/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#ifndef GUARD_BME280
#define GUARD_BME280

#include "i2c.h"

class BME280
{
    public:
        static constexpr uint8_t I2C_DEFAULT_ADDRESS = 0x77;

    public:
        BME280(I2C& i2c);

        bool init(uint8_t addr = BME280::I2C_DEFAULT_ADDRESS);

        float       readTemperature();
	    uint16_t    readTemperature16bits();
        float       readPressure();
	    uint16_t    readPressure16bits();
        float       readHumidity();
	    uint16_t    readHumidity16bits();
        float       readAltitude(float seaLevel);
        float       seaLevelForAltitude(float altitude, float atmospheric);

    private:

        void readCoefficients();

        void      write8(uint8_t reg, uint8_t value);
        uint8_t   read8(uint8_t reg);
        uint16_t  read16(uint8_t reg);
        uint32_t  read24(uint8_t reg);
        int16_t   readS16(uint8_t reg);
        uint16_t  read16_LE(uint8_t reg); // little endian
        int16_t   readS16_LE(uint8_t reg); // little endian

        int32_t	  t_fine;

        struct CalibrationData
        {
          uint16_t dig_T1;
          int16_t  dig_T2;
          int16_t  dig_T3;

          uint16_t dig_P1;
          int16_t  dig_P2;
          int16_t  dig_P3;
          int16_t  dig_P4;
          int16_t  dig_P5;
          int16_t  dig_P6;
          int16_t  dig_P7;
          int16_t  dig_P8;
          int16_t  dig_P9;

          uint8_t  dig_H1;
          int16_t  dig_H2;
          uint8_t  dig_H3;
          int16_t  dig_H4;
          int16_t  dig_H5;
          int8_t   dig_H6;
        };

        CalibrationData m_calibrationData;

        uint8_t   m_i2cAddr;
        I2C& m_i2c;

	    bool m_initOK;

    public:
        struct Registers
        {
            static constexpr uint8_t DIG_T1 = 0x88;
            static constexpr uint8_t DIG_T2 = 0x8A;
            static constexpr uint8_t DIG_T3 = 0x8C;

            static constexpr uint8_t DIG_P1 = 0x8E;
            static constexpr uint8_t DIG_P2 = 0x90;
            static constexpr uint8_t DIG_P3 = 0x92;
            static constexpr uint8_t DIG_P4 = 0x94;
            static constexpr uint8_t DIG_P5 = 0x96;
            static constexpr uint8_t DIG_P6 = 0x98;
            static constexpr uint8_t DIG_P7 = 0x9A;
            static constexpr uint8_t DIG_P8 = 0x9C;
            static constexpr uint8_t DIG_P9 = 0x9E;

            static constexpr uint8_t DIG_H1 = 0xA1;
            static constexpr uint8_t DIG_H2 = 0xE1;
            static constexpr uint8_t DIG_H3 = 0xE3;
            static constexpr uint8_t DIG_H4 = 0xE4;
            static constexpr uint8_t DIG_H5 = 0xE5;
            static constexpr uint8_t DIG_H6 = 0xE7;

            static constexpr uint8_t CAL26  = 0xE1;  ///< R calibration stored in 0xE1-0xF0

            static constexpr uint8_t CHIPID         = 0xD0;
            static constexpr uint8_t VERSION        = 0xD1;
            static constexpr uint8_t SOFTRESET      = 0xE0;

            static constexpr uint8_t CONTROLHUMID   = 0xF2;
            static constexpr uint8_t CONTROL        = 0xF4;
            static constexpr uint8_t CONFIG         = 0xF5;
            static constexpr uint8_t PRESSUREDATA   = 0xF7;
            static constexpr uint8_t TEMPDATA       = 0xFA;
            static constexpr uint8_t HUMIDDATA      = 0xFD;
        };
};

#endif
