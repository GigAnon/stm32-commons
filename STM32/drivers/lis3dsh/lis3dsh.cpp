#include "lis3dsh.h"

#include "system.h"

LIS3DSH::LIS3DSH(SPI& spi, Pin cs):
		m_spi(spi), m_cs(cs)
{
}

bool LIS3DSH::init(Config config)
{
	if(m_cs)
	{
		m_cs.init(GPIO_MODE_OUTPUT_PP);
		m_cs.setHigh();
	}

/*    if(write(Registers::CTRL_3, 0x01)) // Soft reset
        return true;

    System::delay(5);*/

    const uint8_t ODR = config.odr               << 4;
    const uint8_t BDU = (config.blockUpdate?1:0) << 3; // Block for read
    const uint8_t ZEN = (config.enableZ?1:0)     << 2; // Z axis enabled
    const uint8_t YEN = (config.enableY?1:0)     << 1; // Y axis enabled
    const uint8_t XEN = (config.enableX?1:0)     << 0; // X axis enabled

	if(write(Registers::CTRL_4,	ODR|BDU|ZEN|YEN|XEN))
		return true;

    if(write(Registers::CTRL_5, 0x80))
        return true;

    if(write(Registers::CTRL_6, 0x10))
        return true;

    if(write(Registers::FIFO_CTRL, 0))
        return true;

	return read(Registers::WHO_AM_I) != LIS3DSH::PRODUCT_ID;
}

bool LIS3DSH::write(uint8_t reg, const uint8_t * const data, uint16_t size)
{
	if(size == 0)
		return true;

	m_cs.setLow();

	bool r = m_spi.write(reg, LIS3DSH::SPI_FREQUENCY);
	r |= m_spi.write(data, size, LIS3DSH::SPI_FREQUENCY);

	m_cs.setHigh();

	return r;
}

bool LIS3DSH::write(uint8_t reg, uint8_t val)
{
	return write(reg, &val, sizeof(val));
}

ByteArray LIS3DSH::read(uint8_t reg, uint16_t size)
{
	if(size == 0)
		return ByteArray();

	
	reg |= (1<<7); // Read

	m_cs.setLow();

	ByteArray b;

	if(!m_spi.write(reg, LIS3DSH::SPI_FREQUENCY))
	{
		b.resize(size, 0x00);
		
		if(m_spi.read(b.internalBuffer(), size, LIS3DSH::SPI_FREQUENCY))
			b.clear();
	}

	m_cs.setHigh();

	return b;
}

uint8_t LIS3DSH::read(uint8_t reg)
{
	ByteArray r = read(reg, 1);

	return (r.size() == 1) ? r[0] : 0;
}

LIS3DSH::Data LIS3DSH::waitForData()
{
    while(!isDataAvailable());

    return readData();
}

bool LIS3DSH::isDataAvailable()
{
    return read(LIS3DSH::Registers::STATUS) & (1<<7);
}

int16_t LIS3DSH::readTemperature()
{
    return (int16_t)(read(LIS3DSH::Registers::OUT_T)) + 25;
}

LIS3DSH::Data LIS3DSH::readData()
{
    ByteArray r = read(Registers::OUT_X, 6);

    if(r.size() != 6)
        return LIS3DSH::Data();

    return LIS3DSH::Data((uint16_t)(r[1])<<8 | (uint16_t)(r[0]),
                         (uint16_t)(r[3])<<8 | (uint16_t)(r[2]),
                         (uint16_t)(r[5])<<8 | (uint16_t)(r[4]));
}

uint16_t LIS3DSH::read16(uint8_t reg)
{
	ByteArray r = read(reg, 2);

	if(r.size() != 2)
		return 0;

	return (uint16_t)(r[1])<<8 | (uint16_t)(r[0]);
}

uint16_t LIS3DSH::readX()
{
	return read16(Registers::OUT_X);
}

uint16_t LIS3DSH::readY()
{
	return read16(Registers::OUT_Y);
}

uint16_t LIS3DSH::readZ()
{
	return read16(Registers::OUT_Z);
}
