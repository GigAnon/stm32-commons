#ifndef GUARD_BMP180
#define GUARD_BMP180

#include <cstdint>
#include "i2c.h"

class BMP180
{
    public:
        constexpr static uint8_t ADDRESS = 0x77;

        struct PressureResolution
        {
            enum Value
            {
                LOWEST  = 0,
                LOW     = 1,
                HIGH    = 2,
                HIGHEST = 3
            };
        };

    public:
        BMP180(I2C& i2c);
        bool init();

        int32_t  readTemperature();
        int32_t readPressure(PressureResolution::Value resolution = PressureResolution::HIGHEST);

    private:
        bool loadCalibration();

        int32_t readB5();
        int32_t readPressureRaw(PressureResolution::Value resolution);

        I2C& m_i2c;

        enum Registers
        {
            ID                  = 0xD0, ///< ID register. Should always read 0x55
            SOFT_RESET          = 0xE0,
            CONTROL             = 0xF4,

            OUT_MSB             = 0xF6,
            OUT_LSB             = 0xF7,
            OUT_XLSB            = 0xF8,

            /* Calibration registers */
            AC1                 = 0xAA,
            AC2                 = 0xAC,
            AC3                 = 0xAE,
            AC4                 = 0xB0,
            AC5                 = 0xB2,
            AC6                 = 0xB4,
            B1                  = 0xB6,
            B2                  = 0xB8,
            MB                  = 0xBA,
            MC                  = 0xBC,
            MD                  = 0xBE,
        };

        /**
            Control values for CONTROL (0xF4) register
        **/
        enum Commands
        {
            TEMP        = 0x2E, ///< Read temperature command
            PRES_OSS_0  = 0x34, ///< Read pressure, no oversampling
            PRES_OSS_1  = 0x74, ///< Read pressure, 2x oversampling
            PRES_OSS_2  = 0xB4, ///< Read pressure, 4x oversampling
            PRES_OSS_3  = 0xF4, ///< Read pressure, 8x oversampling
        };

        struct CalibrationCoefficients
        {
            int16_t AC1  = 0;
            int16_t AC2  = 0;
            int16_t AC3  = 0;
            uint16_t AC4 = 0;
            uint16_t AC5 = 0;
            uint16_t AC6 = 0;
            int16_t B1   = 0;
            int16_t B2   = 0;
            int16_t MB   = 0;
            int16_t MC   = 0;
            int16_t MD   = 0;
        };

        CalibrationCoefficients m_calib;
};

#endif
