#include "pin.h"
#include "system.h"
#include "adc.h"

Pin::Pin(GPIO_TypeDef* port, uint8_t pin):
	m_port(port), m_pin(pin)
{}

Pin::Pin():
	m_port(nullptr), m_pin(0)
{}

GPIO_TypeDef* Pin::port() const
{
	return m_port;
}

uint8_t Pin::pin() const
{
	return m_pin;
}

bool Pin::operator==(const Pin& other) const
{
	return (port() == other.port()) && (pin() == other.pin());
}

bool Pin::isValid() const
{
	return port() != nullptr;
}


bool Pin::read() const
{
	if(!isValid())
		return false;

	return HAL_GPIO_ReadPin(port(), definePin());
}

uint16_t Pin::definePin() const
{
	return pinNumToDefine(pin());
}

void Pin::setHigh()
{
	if(isValid())
		HAL_GPIO_WritePin(port(), definePin(), GPIO_PIN_SET);
}

void Pin::setLow()
{
	if(isValid())
		HAL_GPIO_WritePin(port(), definePin(), GPIO_PIN_RESET);
}
	
void Pin::set(bool high)
{
	high?setHigh():setLow();
}

void Pin::toggle()
{
	if(isValid())
		HAL_GPIO_TogglePin(port(), definePin());
}

bool Pin::init(uint32_t mode, uint32_t pull, uint32_t speed)
{
	if(!isValid())
		return true;
    
    if(mode == GPIO_MODE_ANALOG)
    {
#if defined(HAL_ADC_MODULE_ENABLED)
        uint8_t channel = getADCChannel(ADC1);
        
        if(channel == 0xFF)
            return true;
        
        if(!adc1.isInitialized())
            if(adc1.init())
                return true;
        
        if(!adc2.isInitialized())
            if(adc2.init())
                return true;
#else
        return true;
#endif
    }
	
	System::enableGPIOPortClock(port());
	
	GPIO_InitTypeDef gpio;
	gpio.Pin		= definePin();
	gpio.Mode		= mode;
	gpio.Speed		= speed;
	gpio.Pull		= pull;

    auto p = port();
	
	HAL_GPIO_Init(port(), &gpio);
    
    return false;
}

#if defined(HAL_ADC_MODULE_ENABLED)
uint16_t Pin::adcRead(uint8_t sampleTime)
{
    if(!isValid())
        return true;
    
    uint8_t ch1 = getADCChannel(ADC1);
    uint8_t ch2 = getADCChannel(ADC2);
    
    if(ch1 == 0xFF || ch2 == 0xFF)
        return 0xFFFF;
    
    return (adc1.pollChannel(ch1, sampleTime) + adc2.pollChannel(ch2, sampleTime))/2;
        
}
#endif

#define p(n) if(num==n) { return GPIO_PIN_##n; }
uint16_t Pin::pinNumToDefine(uint8_t num)
{
	p(0);
	p(1);
	p(3);
	p(4);
	p(5);
	p(6);
	p(7);
	p(8);
	p(9);
	p(10);
	p(11);
	p(12);
	p(13);
	p(14);
	p(15);
	
	return 0;
}

#if defined(HAL_ADC_MODULE_ENABLED)
uint8_t Pin::getADCChannel(ADC_TypeDef* adc)
{
    if(adc == ADC1 || adc == ADC2)
    {
        if(port() == GPIOA)
        {
            if(/*pin() >= 0 &&*/ pin() <= 7)
                return pin();
        }
        else if(port() == GPIOB)
        {
            if(pin() == 0) return 8;
            if(pin() == 1) return 9;
        }
        else if(port() == GPIOC)
        {
            if(/*pin() >= 0 && */pin() <= 5)
                return pin() + 10;
        }
    }
    return 0xFF;
}
#endif
