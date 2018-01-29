#include "nunchuck.h"

#include "system.h"

bool Nunchuck::Data::operator==(const Data& other) const
{
    return  other.accelX == this->accelX &&
            other.accelY == this->accelY &&
            other.accelZ == this->accelZ &&
            other.joyX   == this->joyX   &&
            other.joyY   == this->joyY  &&
            other.cButtonDown == this->cButtonDown &&
            other.zButtonDown == this->zButtonDown;

}

bool Nunchuck::Data::operator!=(const Data& other) const
{
    return !(*this == other);
}

Nunchuck::Data::operator bool() const
{
    return !(*this == Data());
}


Nunchuck::Nunchuck(I2C& i2c): m_i2c(i2c)
{}

bool Nunchuck::init()
{
    if(!m_i2c.deviceAvailable(Nunchuck::ADDRESS))
        return true;

    uint8_t d[] = {0x40, 0x00};

    System::delay(1);

    /* Initialize the Nunchuck */
    return m_i2c.write(Nunchuck::ADDRESS, d, 2);
}

Nunchuck::Data Nunchuck::getData()
{
    /* Data request */
    if(m_i2c.write(Nunchuck::ADDRESS, 0x00))
        return Data();

    /* Wait for request to be processed */
    System::delay(Nunchuck::READ_DELAY_MS);

    uint8_t data[6] = {};

    /* Get data */
    if(m_i2c.read(Nunchuck::ADDRESS, data, 6))
        return Data();

    /* 'Decode' data */
    for(uint32_t i=0; i<6; ++i)
        data[i] = (data[i] ^ 0x17) + 0x17;

    Data d;
    d.joyX = data[0];
    d.joyY = data[1];
    d.cButtonDown = !(bool)(data[5] & 0x02);
    d.zButtonDown = !(bool)(data[5] & 0x01);

    d.accelX = ((uint16_t)(data[2])<<2) | ((uint16_t)(data[5] >> 6)&0x03);
    d.accelY = ((uint16_t)(data[3])<<2) | ((uint16_t)(data[5] >> 4)&0x03);
    d.accelZ = ((uint16_t)(data[4])<<2) | ((uint16_t)(data[5] >> 2)&0x03);

    return d;
}
