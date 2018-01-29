#ifndef GUARD_HTU21D
#define GUARD_HTU21D

#include "hal.h"
#include "i2c.h"

/**
    \brief Class driver for HTU21D sensor

    Class driver for HTU21D sensor.
    Features:
    - I2C communications
    - Temperature sensor
    - Humidity sensor
**/
class HTU21D
{
    public:
        static constexpr uint8_t I2C_ADR        = 0x40; ///< HTU21D I2C address

    private:
        /* HTU21D registers */
        static constexpr uint8_t USR_REG_WRITE  = 0xE6;
        static constexpr uint8_t USR_REG_READ   = 0xE7;
        static constexpr uint8_t SOFT_RESET     = 0xFE;

        static constexpr uint8_t TEMP_HOLD      = 0xE3;
        static constexpr uint8_t HUMI_HOLD      = 0xE5;
        static constexpr uint8_t TEMP_POLL      = 0xF3;
        static constexpr uint8_t HUMI_POLL      = 0xF5;

    public:
        /**
            \brief Constructor
            \param i2c I2C bus instance
        **/
        HTU21D(I2C& i2c);

        /**
            \brief Initialise the driver
            \returns \c true on error, \c false otherwise
        **/
        bool init();

        /**
            \brief Reset the HTU21D
        **/
        void reset();

        /**
            \brief Read temperature

            Poll HTU21D for temperature reading. 
            Maximum precision: 14bits
        **/
        float readTemperature();

        /**
            \brief Read relative humidity

            Poll HTU21D for relative humidity reading. 
            Maximum precision: 12bits
        **/
        float readHumidity();

        /**
            \brief 'ping' the HTU21D
            \returns \c true if sensor was detected, \c false otherwise
        **/
        bool isAvailable();

    private:
        I2C& m_bus;
};

#endif
