#include "htu21d.h"

#include "system.h"

#include <limits>
#include <cmath>

HTU21D::HTU21D(I2C& i2c): m_bus(i2c)
{}

bool HTU21D::init()
{
    if(!isAvailable())
        return true;

    reset();

    /* Default value for user register after reset is 0x20 (0100 0000) */
    return m_bus.read(HTU21D::I2C_ADR, HTU21D::USR_REG_READ) != 0x20; 
}

void HTU21D::reset()
{
    m_bus.write(HTU21D::I2C_ADR, HTU21D::SOFT_RESET);
    System::delay(15);
}

bool HTU21D::isAvailable()
{
    return m_bus.deviceAvailable(HTU21D::I2C_ADR);
}

float HTU21D::readTemperature()
{
    if(m_bus.write(HTU21D::I2C_ADR, HTU21D::TEMP_HOLD))
        return std::numeric_limits<float>::quiet_NaN();

    /* Maximum measuring time is 50ms */
    /* @todo: a more subtle approach */
    System::delay(50);

    uint8_t data[3] = {0};

    if(m_bus.read(HTU21D::I2C_ADR, data, sizeof(data)))
        return std::numeric_limits<float>::quiet_NaN();

    uint16_t t = (static_cast<uint16_t>(data[0]) << 8) | data[1];

    float temp = static_cast<float>(t)*175.72/65536.f - 46.85;

    /* Truncate to 2 decimal places (maximum accuracy is .01°) */
    return round(temp*100.f)/100.f;
}

float HTU21D::readHumidity()
{
    if(m_bus.write(HTU21D::I2C_ADR, HTU21D::HUMI_HOLD))
        return std::numeric_limits<float>::quiet_NaN();

    /* Maximum measuring time is 16ms */
    /* @todo: a more subtle approach */
    System::delay(16);

    uint8_t data[3] = {0};

    if(m_bus.read(HTU21D::I2C_ADR, data, sizeof(data)))
        return std::numeric_limits<float>::quiet_NaN();

    uint16_t h = (static_cast<uint16_t>(data[0]) << 8) | data[1];

    float humi = static_cast<float>(h)*125.f/65536.f - 6.f;

    /* Truncate to 2 decimal places (maximum accuracy is .04 %) */
    return round(humi*100.f)/100.f;
}

