#ifndef GUARD_NUNCHUCK
#define GUARD_NUNCHUCK

#include "i2c.h"

/**
    \brief Driver for Nintendo Nunchuck controler
**/
class Nunchuck
{
    public:
        static constexpr uint8_t  ADDRESS       = 0x52; ///< I2C address of the device
        static constexpr uint32_t READ_DELAY_MS = 1;    ///< Delay between command and data request

        /**
            \brief Structure for data returned by the sensor
        **/
        struct Data
        {
            /**
                Ctor
            **/
            Data(bool _cb, bool _zb, uint16_t _ax, uint16_t _ay, uint16_t _az, uint8_t _jx, uint8_t _jy):
                cButtonDown(_cb), zButtonDown(_zb),
                accelX(_ax), accelY(_ay), accelZ(_az), joyX(_jx), joyY(_jy) {}

            /**
                Default ctor
            **/
            Data(): Data(false, false, 0xFFFF, 0xFFFF, 0xFFFF, 0, 0) {}

            bool operator==(const Data& other) const;
            bool operator!=(const Data& other) const;

            operator bool() const;

            bool cButtonDown = false; ///< Is C button pressed
            bool zButtonDown = false; ///< Is Z button pressed

            uint16_t accelX  = 0xFFFF; ///< X-axis acceleration (10 bits)
            uint16_t accelY  = 0xFFFF; ///< Y-axis acceleration (10 bits)
            uint16_t accelZ  = 0xFFFF; ///< Z-axis acceleration (10 bits)

            uint8_t joyX    = 0; ///< Joystick X (center +/- 127)
            uint8_t joyY    = 0; ///< Joystick Y (center +/- 127)
        };

    public:
        /**
            Constructor
            \param i2c I2C peripheral to be used
        **/
        Nunchuck(I2C& i2c);

        /**
            No copy
        **/
        Nunchuck(const Nunchuck&) = delete;

        /**
            \brief Initialize the driver
            \returns \c true on error, \c false otherwise
            \remark I2C peripheral must have been prealably initialized
        **/
        bool init();

        /**
            \brief Read sensor data from the Nunchuck
            \returns Sensor data, or Data() on error
        **/
        Nunchuck::Data getData();

    private:
        I2C& m_i2c;
};

#endif
