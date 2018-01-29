#include "i2c.h"

#if defined(HAL_I2C_MODULE_ENABLED)

#include "system.h"

#if defined(I2C1)
    I2C i2c1(I2C1);
#endif
#if defined(I2C2)
    I2C i2c2(I2C2);
#endif

I2C::I2C(I2C_TypeDef* bus)
{
    m_i2cHandle.Instance = bus;
}

#define CLK_COND_ENABLE(a, x) { if(a == x) __##x##_CLK_ENABLE(); }
#define I2C_COND_RESET(a, x) { if(a == x) { __HAL_RCC_##x##_FORCE_RESET(); \
                                            __HAL_RCC_##x##_RELEASE_RESET(); }}

bool I2C::init(uint32_t speed)
{
    if(!m_i2cHandle.Instance)
		return true;
    
	CLK_COND_ENABLE(m_i2cHandle.Instance, I2C1);
	CLK_COND_ENABLE(m_i2cHandle.Instance, I2C2);

    initPins();

    m_i2cHandle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    m_i2cHandle.Init.ClockSpeed     = speed;
    m_i2cHandle.Init.DualAddressMode= I2C_DUALADDRESS_DISABLE;
    m_i2cHandle.Init.DutyCycle      = I2C_DUTYCYCLE_2;
    m_i2cHandle.Init.GeneralCallMode= I2C_GENERALCALL_DISABLE;
    m_i2cHandle.Init.NoStretchMode  = I2C_NOSTRETCH_ENABLE;// I2C_NOSTRETCH_DISABLE;

    m_i2cHandle.Init.OwnAddress1    = 0x00; /* Master only */
    m_i2cHandle.Init.OwnAddress2    = 0x00; /*    Ditto    */

#if defined(STM32F4xx)
    I2C_COND_RESET(m_i2cHandle.Instance, I2C1);
    I2C_COND_RESET(m_i2cHandle.Instance, I2C2);
#endif

	return HAL_I2C_Init(&m_i2cHandle) != HAL_OK;
}

void I2C::initPins()
{
    GPIO_InitTypeDef gpio;
	GPIO_TypeDef* port = nullptr;
	gpio.Speed  = GPIO_SPEED_HIGH;
	gpio.Mode   = GPIO_MODE_AF_OD;
	gpio.Pull   = GPIO_PULLUP;

#if !defined(STM32F1xx)
    gpio.Alternate = GPIO_AF4_I2C1;
#endif

	// SDA
	if(m_i2cHandle.Instance == I2C1)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_7;
	}
	else if(m_i2cHandle.Instance == I2C2)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_11;
	}
    if(!port)
        return;

	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);

    // SCL
	if(m_i2cHandle.Instance == I2C1)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_6;
	}
	else if(m_i2cHandle.Instance == I2C2)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_10;
	}
	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
}

bool I2C::deviceAvailable(uint8_t adr)
{
    auto r = HAL_I2C_IsDeviceReady(&m_i2cHandle, adr << 1, 1, I2C::TIMEOUT);
	return r == HAL_OK;
}

bool I2C::write(uint8_t devAdr, const uint8_t* data, uint16_t dataSize)
{
    return HAL_I2C_Master_Transmit(&m_i2cHandle, devAdr << 1, const_cast<uint8_t*>(data), dataSize, I2C::TIMEOUT) != HAL_OK;
}

bool I2C::write(uint8_t devAdr, const ByteArray& data)
{
    return write(devAdr, data.internalBuffer(), data.size());
}

bool I2C::write(uint8_t devAdr, uint16_t regAdr, uint8_t data)
{
	return write(devAdr, regAdr, ByteArray(1, data));
}

bool I2C::write(uint8_t devAdr, uint8_t d)
{
    return write(devAdr, ByteArray(1, d));
}

bool I2C::write(uint8_t devAdr, uint16_t regAdr, const ByteArray& data)
{
	return HAL_I2C_Mem_Write(&m_i2cHandle, devAdr << 1, regAdr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)data.internalBuffer(), data.size(), TIMEOUT) != HAL_OK;
}

bool I2C::write16(uint8_t devAdr, uint16_t regAdr, uint8_t data)
{
	return write16(devAdr, regAdr, ByteArray(1, data));
}

bool I2C::write16(uint8_t devAdr, uint16_t regAdr, const ByteArray& data)
{
	return HAL_I2C_Mem_Write(&m_i2cHandle, devAdr << 1, regAdr, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data.internalBuffer(), data.size(), TIMEOUT) != HAL_OK;
}

bool I2C::read(uint8_t devAdr, uint8_t* data, uint16_t dataSize)
{
    return HAL_I2C_Master_Receive(&m_i2cHandle, devAdr << 1, data, dataSize, TIMEOUT) != HAL_OK;
}
	
uint8_t	I2C::read(uint8_t devAdr, uint16_t regAdr)
{
	ByteArray a(read(devAdr, regAdr, 1));
	
	if(a.size() != 1)
		return 0x00;
	return a[0];
}

ByteArray I2C::read(uint8_t devAdr, uint16_t regAdr, uint16_t size)
{
	ByteArray data(size);
	
	if(HAL_I2C_Mem_Read(&m_i2cHandle, devAdr << 1, regAdr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)data.internalBuffer(), data.size(), TIMEOUT) != HAL_OK)
		return ByteArray();
	
	return data;
}

uint8_t	I2C::read16(uint8_t devAdr, uint16_t regAdr)
{
	ByteArray a(read16(devAdr, regAdr, 1));
	
	if(a.size() != 1)
		return 0x00;
	return a[0];
}

ByteArray I2C::read16(uint8_t devAdr, uint16_t regAdr, uint16_t size)
{
	ByteArray data(size);
	
	if(HAL_I2C_Mem_Read(&m_i2cHandle, devAdr << 1, regAdr, I2C_MEMADD_SIZE_16BIT, (uint8_t*)data.internalBuffer(), data.size(), TIMEOUT) != HAL_OK)
		return ByteArray();
	
	return data;
}

#endif /* #if defined(HAL_I2C_MODULE_ENABLED) */
