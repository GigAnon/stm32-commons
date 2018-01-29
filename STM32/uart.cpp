#include "uart.h"

#if defined(HAL_UART_MODULE_ENABLED)

#include "system.h"

#include <algorithm>
#include <cstring>

#if defined(USART1)
    #define GPIO_AF_USART1     GPIO_AF7_USART1
    Serial uart1(USART1);
#endif
#if defined(USART2)
    #define GPIO_AF_USART2     GPIO_AF7_USART2
    Serial uart2(USART2);
#endif
#if defined(USART3)
    #define GPIO_AF_USART3     GPIO_AF7_USART3
    Serial uart3(USART3);
#endif
#if defined(UART4)
    #define GPIO_AF_UART4      GPIO_AF8_UART4
    Serial uart4(UART4);
#endif
#if defined(UART5)
    Serial uart5(UART5);
#endif
#if defined(USART6)
    #define GPIO_AF_USART6     GPIO_AF8_USART6
    Serial uart6(USART6);
#endif


Serial::Serial(USART_TypeDef* uart): m_rxBuffer(RX_BUFFER_SIZE)
{
	m_handle.Instance = uart;
}

#define CLK_COND_ENABLE(a, x) { if(a == x) { __HAL_RCC_##x##_CLK_ENABLE(); __HAL_RCC_##x##_FORCE_RESET(); __HAL_RCC_##x##_RELEASE_RESET(); } }
#define NVIC_COND_CONFIG(a, x) { if(a == x) {	HAL_NVIC_DisableIRQ(x##_IRQn); \
												HAL_NVIC_SetPriority(x##_IRQn, 6, 0); \
												HAL_NVIC_EnableIRQ(x##_IRQn); \
												HAL_NVIC_ClearPendingIRQ(x##_IRQn); \
												x->CR1 |= USART_CR1_RXNEIE; }}

bool Serial::init(uint32_t baudrate, bool useAF)
{
	CLK_COND_ENABLE(m_handle.Instance, USART1);
	CLK_COND_ENABLE(m_handle.Instance, USART2);
	CLK_COND_ENABLE(m_handle.Instance, USART3);
	
	initPins(useAF);
	
	m_handle.Init.BaudRate		= baudrate;
	m_handle.Init.WordLength	= UART_WORDLENGTH_8B;
	m_handle.Init.Parity		= UART_PARITY_NONE;
	m_handle.Init.StopBits		= UART_STOPBITS_1;
	m_handle.Init.Mode			= UART_MODE_TX_RX;
	m_handle.Init.HwFlowCtl		= UART_HWCONTROL_NONE;
	m_handle.Init.OverSampling	= UART_OVERSAMPLING_16;
	
	if(HAL_UART_Init(&m_handle) != HAL_OK)
		return true;
	
	NVIC_COND_CONFIG(m_handle.Instance, USART1);
	NVIC_COND_CONFIG(m_handle.Instance, USART2);
	NVIC_COND_CONFIG(m_handle.Instance, USART3);
	
	return false;
}

#if defined(STM32F1xx)
    #define REMAP_COND_ENABLE(a, x) { if(a == x) __HAL_AFIO_REMAP_##x##_ENABLE(); }
    #define GPIO_SET_ALTERNATE(g, a) while(0);
#else
    #define REMAP_COND_ENABLE(a, x) while(0);
    #define GPIO_SET_ALTERNATE(g, a) { g.Alternate = GPIO_AF_##a; }
#endif

void Serial::initPins(bool useAF)
{
	GPIO_InitTypeDef gpio;
	GPIO_TypeDef* port = nullptr;
	gpio.Speed = GPIO_SPEED_HIGH;
	
	//TX
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pull = GPIO_PULLUP;
	if(m_handle.Instance == USART1)
	{
		port		= useAF?GPIOB		:GPIOA;
		gpio.Pin	= useAF?GPIO_PIN_6	:GPIO_PIN_9;
        GPIO_SET_ALTERNATE(gpio, USART1);
	}
	else if(m_handle.Instance == USART2)
	{
		port		= useAF?GPIOD		:GPIOA;
		gpio.Pin	= useAF?GPIO_PIN_5	:GPIO_PIN_2;
        GPIO_SET_ALTERNATE(gpio, USART2);
	}
	else if(m_handle.Instance == USART3)
	{
		port		= useAF?GPIOD		:GPIOB;
		gpio.Pin	= useAF?GPIO_PIN_8	:GPIO_PIN_10;
        GPIO_SET_ALTERNATE(gpio, USART3);
	}

    if(!port)
        return;

	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
	
	//RX
#if defined(STM32F1xx)
    gpio.Mode = GPIO_MODE_AF_INPUT;
#else
    gpio.Mode = GPIO_MODE_AF_PP;
#endif
	gpio.Pull = GPIO_NOPULL;
	if(m_handle.Instance == USART1)
	{
		port		= useAF?GPIOB		:GPIOA;
		gpio.Pin	= useAF?GPIO_PIN_7	:GPIO_PIN_10;
        GPIO_SET_ALTERNATE(gpio, USART1);
	}
	else if(m_handle.Instance == USART2)
	{
		port		= useAF?GPIOD		:GPIOA;
		gpio.Pin	= useAF?GPIO_PIN_6	:GPIO_PIN_3;
        GPIO_SET_ALTERNATE(gpio, USART2);
	}
	else if(m_handle.Instance == USART3)
	{
		port		= useAF?GPIOD		:GPIOB;
		gpio.Pin	= useAF?GPIO_PIN_9	:GPIO_PIN_11;
        GPIO_SET_ALTERNATE(gpio, USART3);
	}
	System::enableGPIOPortClock(port);
	HAL_GPIO_Init(port, &gpio);
	
	if(useAF)
	{
        /* For STM32F1 */
		REMAP_COND_ENABLE(m_handle.Instance, USART1);
		REMAP_COND_ENABLE(m_handle.Instance, USART2);
		REMAP_COND_ENABLE(m_handle.Instance, USART3);
	}
}

void Serial::clear()
{
    m_rxBuffer.clear();
}

bool Serial::write(const uint8_t* data, size_t dataSize)
{
	return (HAL_UART_Transmit(&m_handle, const_cast<uint8_t*>(data), dataSize, 5000) != HAL_OK);
}

bool Serial::write(const char* str)
{
    return write(reinterpret_cast<const uint8_t*>(str), std::strlen(str));
}

bool Serial::write(uint8_t c)
{
    return write(&c, 1);
}

bool Serial::write(const ByteArray& ba)
{
    return write(ba.internalBuffer(), ba.size());
}

#define IRQ_COND_CLEAR(a, x) { if(a == x) HAL_NVIC_ClearPendingIRQ(x##_IRQn); }

void Serial::rxInteruptHandler()
{
	
	if (__HAL_UART_GET_FLAG(&m_handle, USART_SR_RXNE))
        m_rxBuffer.push(m_handle.Instance->DR);

	__HAL_UART_CLEAR_PEFLAG(&m_handle);
	__HAL_UART_CLEAR_FEFLAG(&m_handle);
	__HAL_UART_CLEAR_NEFLAG(&m_handle);
	__HAL_UART_CLEAR_OREFLAG(&m_handle);
	__HAL_UART_CLEAR_IDLEFLAG(&m_handle);

	/* Clear IRQ bit */
	IRQ_COND_CLEAR(m_handle.Instance, USART1);
	IRQ_COND_CLEAR(m_handle.Instance, USART2);
	IRQ_COND_CLEAR(m_handle.Instance, USART3);
}

size_t Serial::dataAvailable() const
{
    return m_rxBuffer.size();
}

size_t Serial::read(uint8_t* data, size_t maxBytes)
{
	__disable_irq();
	
    size_t i = m_rxBuffer.pull(data, maxBytes);

	__enable_irq();
	
	return i;
}

uint8_t Serial::read()
{
    if(m_rxBuffer.size() == 0)
        return 0x00;

    return m_rxBuffer.pull();
}

size_t Serial::readAppend(ByteArray& a, uint16_t maxBytes)
{
	size_t n = std::min(dataAvailable(), static_cast<size_t>(maxBytes));
	a.reserve(a.size() + n);
	
	for(size_t i=0;i<n;++i)
		a.append(read());

    return n;
}


ByteArray Serial::readUntil(uint8_t endc, uint16_t maxBytes)
{
    ByteArray a;
	a.reserve(32);
	
	while(a.size() < maxBytes)
	{
		if(dataAvailable())
		{
			uint8_t c = read();
			a.append(c);
			if(c == endc)
				break;
		}
	}
	
	return a;

}

#if defined(USART1)
extern "C" void USART1_IRQHandler(void)
{
	uart1.rxInteruptHandler();
}
#endif

#if defined(USART2)
extern "C" void USART2_IRQHandler(void)
{
	uart2.rxInteruptHandler();
}
#endif

#if defined(USART3)
extern "C" void USART3_IRQHandler(void)
{
	uart3.rxInteruptHandler();
}
#endif

/** For printf **/
extern "C" int _write(int, char *ptr, int len)
{
#ifdef PRINTF_UART
	PRINTF_UART.write(reinterpret_cast<uint8_t*>(ptr), static_cast<size_t>(len));
#else
    for(int i=0;i<len;++i)
        ITM_SendChar(ptr[i]);
#endif
	return len;
}

extern "C" void abort()
{
	asm("bkpt 255");
	for(;;);
}

#endif /* #if defined(HAL_RNG_MODULE_ENABLED) */
