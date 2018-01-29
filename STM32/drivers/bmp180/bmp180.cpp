#include "bmp180.h"

#include "system.h"
#include <cstring>

BMP180::BMP180(I2C& i2c): m_i2c(i2c)
{
}

bool BMP180::init()
{
    if(!m_i2c.deviceAvailable(BMP180::ADDRESS) || m_i2c.read(BMP180::ADDRESS, Registers::ID) != 0x55)
        return true;

    return loadCalibration();
}

bool BMP180::loadCalibration()
{
    ByteArray d = m_i2c.read(BMP180::ADDRESS, Registers::AC1, 22);

    if(d.size() != 22)
        return true;

    int16_t *c = reinterpret_cast<int16_t*>(&m_calib);

    for(size_t i=0;i<11;++i)
        c[i] = ((int16_t)(d[i*2]) << 8) + d[i*2+1];
        
    return false;
}

int32_t BMP180::readB5()
{
    m_i2c.write(BMP180::ADDRESS, Registers::CONTROL, Commands::TEMP);

    System::delay(5);

    int32_t ut = ((uint16_t)(m_i2c.read(BMP180::ADDRESS, OUT_MSB)) << 8) | (uint16_t)(m_i2c.read(BMP180::ADDRESS, OUT_LSB));

    int32_t x1 = ((ut - (int32_t)(m_calib.AC6)) * (int32_t)(m_calib.AC5)) >> 15;
    int32_t x2 =  ((int32_t)(m_calib.MC) << 11) / (x1 + (int32_t)(m_calib.MD));

    return x1 + x2;
}

int32_t BMP180::readPressureRaw(PressureResolution::Value oss)
{
    if(oss == PressureResolution::LOWEST)
    {
        m_i2c.write(BMP180::ADDRESS, Registers::CONTROL, Commands::PRES_OSS_0);
        System::delay(5);
    }
    else if(oss == PressureResolution::LOW)
    {
        m_i2c.write(BMP180::ADDRESS, Registers::CONTROL, Commands::PRES_OSS_1);
        System::delay(8);
    }
    else if(oss == PressureResolution::HIGH)
    {
        m_i2c.write(BMP180::ADDRESS, Registers::CONTROL, Commands::PRES_OSS_2);
        System::delay(14);
    }
    else if(oss == PressureResolution::HIGHEST)
    {
        m_i2c.write(BMP180::ADDRESS, Registers::CONTROL, Commands::PRES_OSS_3);
        System::delay(26);
    }
    else
        return 0;

    return (((int32_t)(m_i2c.read(BMP180::ADDRESS, OUT_MSB)) << 16) |
            ((int32_t)(m_i2c.read(BMP180::ADDRESS, OUT_LSB)) << 8)  |
             (int32_t)(m_i2c.read(BMP180::ADDRESS, OUT_XLSB))         ) >> (8-oss);
}

int32_t BMP180::readTemperature()
{
    return (readB5() + 8) >> 4;
}

int32_t BMP180::readPressure(PressureResolution::Value oss)
{
    int32_t b6 = readB5() - 4000;

    int32_t x1, x2, x3;
    
    x1 = ((int32_t)(m_calib.B2) * ((b6*b6) >> 12)) >> 11;
    x2 = (m_calib.AC2 * b6) >> 11;
    x3 = x1 + x2;

    int32_t b3 = (((((int32_t)(m_calib.AC1)) * 4 + x3) << oss) + 2) >> 2;

    x1 = (m_calib.AC3 * b6) >> 13;
    x2 = (m_calib.B1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;

    uint32_t a = readPressureRaw(oss);

    uint32_t b4 = (m_calib.AC4 * (uint32_t)(x3 + 32768)) >> 15;
    uint32_t b7 = ((uint32_t)(a) - b3) * (uint32_t)(50000 >> oss);

    int32_t p = (b7 < 0x80000000L) ? ((b7 << 1) / b4) : ((b7 / b4) << 1);

    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;

    return p + ((x1 + x2 + 3791) >> 4);
}

