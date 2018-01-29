#include "spi.h"

#if defined(HAL_SPI_MODULE_ENABLED)

#include "system.h"

#include <algorithm>

#if defined(SPI1)
SPI spi1(SPI1);
#endif
#if defined(SPI2)
SPI spi2(SPI2);
#endif

constexpr uint32_t SPI::READ_TIMEOUT;
constexpr uint32_t SPI::WRITE_TIMEOUT;

SPI::SPI(SPI_TypeDef* spi)
{
	m_handle.Instance = spi;
}

#define CLK_COND_ENABLE(a, x) { if(a == x) __##x##_CLK_ENABLE(); }

bool SPI::init(uint32_t frequency, Mode::Value mode)
{
	if(!m_handle.Instance)
		return true;
    
	CLK_COND_ENABLE(m_handle.Instance, SPI1);
	CLK_COND_ENABLE(m_handle.Instance, SPI2);
	
	initPins();
	
	m_handle.Init.Mode				= SPI_MODE_MASTER;
	m_handle.Init.Direction			= SPI_DIRECTION_2LINES;
	m_handle.Init.DataSize			= SPI_DATASIZE_8BIT;
	m_handle.Init.BaudRatePrescaler = getFloorPrescaler(frequency);
	m_handle.Init.NSS				= SPI_NSS_SOFT;
    
     /* CPOL = 0, CPHA = 0 (mode 0) */
     /* ... */
    /* CPOL = 1, CPHA = 1 (mode 3) */
    m_handle.Init.CLKPolarity       = (mode&Mode::CPOL_1)?SPI_POLARITY_HIGH :SPI_POLARITY_LOW;
    m_handle.Init.CLKPhase          = (mode&Mode::CPHA_1)?SPI_PHASE_2EDGE   :SPI_PHASE_1EDGE;

	m_handle.Init.FirstBit			= SPI_FIRSTBIT_MSB;
	m_handle.Init.CRCCalculation	= SPI_CRCCALCULATION_DISABLED;
	m_handle.Init.CRCPolynomial		= 7;
	m_handle.Init.TIMode			= SPI_TIMODE_DISABLED;
	
    return HAL_SPI_Init(&m_handle) != HAL_OK;
}


void SPI::initPins()
{
	GPIO_InitTypeDef gpio;
	GPIO_TypeDef* port = nullptr;
	gpio.Speed  = GPIO_SPEED_HIGH;
	gpio.Mode   = GPIO_MODE_AF_PP;
	gpio.Pull   = GPIO_PULLDOWN;

#ifndef STM32F1xx
    gpio.Alternate = GPIO_AF5_SPI1;
#endif
	
	// SCK
	if(m_handle.Instance == SPI1)
	{
		port		= GPIOA;
		gpio.Pin	= GPIO_PIN_5;
	}
	else if(m_handle.Instance == SPI2)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_13;
	}

    if(!port)
        return;

	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
	
	// MISO
	if(m_handle.Instance == SPI1)
	{
		port		= GPIOA;
		gpio.Pin	= GPIO_PIN_6;
	}
	else if(m_handle.Instance == SPI2)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_14;
	}
	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
	
	// MOSI
	if(m_handle.Instance == SPI1)
	{
		port		= GPIOA;
		gpio.Pin	= GPIO_PIN_7;
	}
	else if(m_handle.Instance == SPI2)
	{
		port		= GPIOB;
		gpio.Pin	= GPIO_PIN_15;
	}
	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
}

bool SPI::isInitialized() const
{
    return m_handle.State != HAL_SPI_STATE_RESET;
}

bool SPI::write(const uint8_t* data, uint16_t dataSize, uint32_t frequency, Mode::Value mode)
{
    beginTransaction(frequency, mode);

    bool ok = HAL_SPI_Transmit(&m_handle, const_cast<uint8_t*>(data), dataSize, WRITE_TIMEOUT) != HAL_OK;

    endTransaction();

    return ok;
}

bool SPI::write(uint8_t c, uint32_t frequency, Mode::Value mode)
{
	return write(&c, 1, frequency, mode);
}

bool SPI::read(uint8_t* data, uint16_t dataSize, uint32_t frequency, Mode::Value mode)
{
    beginTransaction(frequency, mode);

    bool ok = (HAL_SPI_Receive(&m_handle, data, dataSize, READ_TIMEOUT) != HAL_OK);

    endTransaction();

    return ok;
}

bool SPI::readWrite(const uint8_t* const dataOut, uint8_t* dataIn, uint16_t dataSize, uint32_t frequency, Mode::Value mode)
{
    beginTransaction(frequency, mode);

    bool ok = (HAL_SPI_TransmitReceive(&m_handle, const_cast<uint8_t*>(dataOut), dataIn, dataSize, std::max(READ_TIMEOUT, WRITE_TIMEOUT)) != HAL_OK);

    endTransaction();

    return ok;
}

void SPI::beginTransaction(uint32_t frequency, Mode::Value mode)
{
    m_backupCR1 = m_handle.Instance->CR1;

    if(frequency)
    {
        m_handle.Instance->CR1 &= (~SPI_CR1_BR_Msk);
        m_handle.Instance->CR1 |= getFloorPrescaler(frequency);
    }

    /** Doesn't work for some reason **/
    #if 0
    if(mode != Mode::USE_PREVIOUS)
    {
        m_handle.Instance->CR1 &= (~SPI_CR1_CPHA_Msk);
        if(mode & Mode::CPHA_1)
            m_handle.Instance->CR1 |= (1<<SPI_CR1_CPHA_Pos);

        m_handle.Instance->CR1 &= (~SPI_CR1_CPOL_Msk);
        if(mode & Mode::CPOL_1)
            m_handle.Instance->CR1 |= (1<<SPI_CR1_CPOL_Pos);

    }
    #endif

}

void SPI::endTransaction()
{
    m_handle.Instance->CR1 = m_backupCR1;
}

uint32_t SPI::getFloorPrescaler(uint32_t frequency) const
{
    if(frequency == 0)
        return SPI_BAUDRATEPRESCALER_2;

    uint32_t clk = 0;
    
    if(m_handle.Instance == SPI1)
        clk = HAL_RCC_GetPCLK2Freq();
    else
        clk = HAL_RCC_GetPCLK1Freq();

    if(frequency <= clk/256)
        return SPI_BAUDRATEPRESCALER_256;
    if(frequency <= clk/128)
        return SPI_BAUDRATEPRESCALER_128;
    if(frequency <= clk/64)
        return SPI_BAUDRATEPRESCALER_64;
    if(frequency <= clk/32)
        return SPI_BAUDRATEPRESCALER_32;
    if(frequency <= clk/16)
        return SPI_BAUDRATEPRESCALER_16;
    if(frequency <= clk/8)
        return SPI_BAUDRATEPRESCALER_8;
    if(frequency <= clk/4)
        return SPI_BAUDRATEPRESCALER_4;

    return SPI_BAUDRATEPRESCALER_2;
}

#endif /* #if defined(HAL_SPI_MODULE_ENABLED) */
