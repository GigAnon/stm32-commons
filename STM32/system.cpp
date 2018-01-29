#include "system.h"

#include "rtc.h"

#if !defined(HAL_RCC_MODULE_ENABLED) || !defined(HAL_FLASH_MODULE_ENABLED)
    #error "RCC and FLASH module must be enabled in hal_conf"
#endif

#if defined(GPIOI)
    static GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI};
    constexpr size_t numPorts = 9;
#elif defined(GPIOH)
    static GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH};
    constexpr size_t numPorts = 8;
#elif defined(GPIOG)
    static GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG};
    constexpr size_t numPorts = 7;
#elif defined(GPIOF)
    static GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF};
    constexpr size_t numPorts = 6;
#elif defined(GPIOE)
    static GPIO_TypeDef* ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};
    constexpr size_t numPorts = 5;
#endif

bool System::init()
{
	HAL_Init();

	/* Force printf() not to use a buffer */
	setbuf(stdout, NULL);

    /* Set all pins to floating inputs */
    setAllGPIOPortClockEnabled(true);

    GPIO_InitTypeDef gpio;
	gpio.Pin		= GPIO_PIN_All;
	gpio.Mode		= GPIO_MODE_ANALOG;
	gpio.Speed		= GPIO_SPEED_HIGH;
	gpio.Pull		= GPIO_NOPULL;
	
    for(size_t i=0; i<numPorts; ++i)
        if(ports[i] != GPIOA)
            HAL_GPIO_Init(ports[i], &gpio);

    /* Do NOT set SWD pins as floating inputs... */
    gpio.Pin		= GPIO_PIN_All ^ (GPIO_PIN_13 | GPIO_PIN_14);
    HAL_GPIO_Init(GPIOA, &gpio);

    setAllGPIOPortClockEnabled(false);

    return initMainClock();
}

bool System::initMainClock()
{
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

#if !defined(STM32F1xx)
    /* For STM32F4 MCUs, it is possible to use a different voltage scale when HCLK is underclocked */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif
	
	RCC_OscInitTypeDef oscInit;
	/* Enable HSE Oscillator and activate PLL with HSE as source */
	oscInit.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	
	/* Select proper PLL input clock */
	oscInit.HSEState		= RCC_HSE_ON;
	oscInit.HSIState		= RCC_HSI_OFF;
	oscInit.PLL.PLLSource	= RCC_PLLSOURCE_HSE;

#if defined(STM32F1xx)
	oscInit.HSEPredivValue	= System::HSE_PREDIV;
	oscInit.PLL.PLLMUL		= System::PLL_MUL;
#else
    oscInit.PLL.PLLM    = PLL_M;
    oscInit.PLL.PLLN    = PLL_N;
    oscInit.PLL.PLLP    = PLL_P;
    oscInit.PLL.PLLQ    = PLL_Q;
#endif
	
	/* Set PLL parameters */
	oscInit.PLL.PLLState = RCC_PLL_ON;

	
	/* Try to init */
	if (HAL_RCC_OscConfig(&oscInit) != HAL_OK)
		return true;

	RCC_ClkInitTypeDef clkInit;
	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	clkInit.SYSCLKSource	= RCC_SYSCLKSOURCE_PLLCLK;
	clkInit.AHBCLKDivider	= System::AHB_PRESCALER;
	clkInit.ClockType		= (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);

	clkInit.APB1CLKDivider = System::APB1_PRESCALER;
	clkInit.APB2CLKDivider = System::APB2_PRESCALER;
    
    /* Enable DWT for sub-ms delays and measurements */
    CoreDebug->DEMCR    |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT          = 0;
    DWT->CTRL           |= DWT_CTRL_CYCCNTENA_Msk;
    
    if(HAL_RCC_ClockConfig(&clkInit, System::FLASH_LATENCY) != HAL_OK)
        return true;

    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	return HAL_SYSTICK_Config(hClkFrequency()/1000) != 0;
}

uint32_t System::hClkFrequency()
{
	return HAL_RCC_GetHCLKFreq();
}

#define CLOCK_COND_SET_ENABLED(a, e, x) { if(a == x) { \
                                        if(e) \
                                            __##x##_CLK_ENABLE(); \
                                        else \
                                            __##x##_CLK_DISABLE();    } }

void System::setGPIOPortClockEnabled(GPIO_TypeDef* port, bool enabled)
{
    #if defined(GPIOA)
	    CLOCK_COND_SET_ENABLED(port, enabled, GPIOA);
    #endif
    #if defined(GPIOB)
	    CLOCK_COND_SET_ENABLED(port, enabled, GPIOB);
    #endif
    #if defined(GPIOC)
	    CLOCK_COND_SET_ENABLED(port, enabled, GPIOC);
    #endif
    #if defined(GPIOD)
	    CLOCK_COND_SET_ENABLED(port, enabled, GPIOD);
    #endif
    #if defined(GPIOE)
	    CLOCK_COND_SET_ENABLED(port, enabled, GPIOE);
    #endif
    #if defined(GPIOF)
        CLOCK_COND_SET_ENABLED(port, enabled, GPIOF);
    #endif
    #if defined(GPIOG)
        CLOCK_COND_SET_ENABLED(port, enabled, GPIOG);
    #endif
    #if defined(GPIOH)
        CLOCK_COND_SET_ENABLED(port, enabled, GPIOH);
    #endif
    #if defined(GPIOI)
        CLOCK_COND_SET_ENABLED(port, enabled, GPIOI);
    #endif
}
#undef CLOCK_COND_SET_ENABLED

void System::enableGPIOPortClock(GPIO_TypeDef* port)
{
    setGPIOPortClockEnabled(port, true);
}

void System::setAllGPIOPortClockEnabled(bool enabled)
{
    for(size_t i=0; i<numPorts; ++i)
        setGPIOPortClockEnabled(ports[i], enabled);
}

uint64_t System::micros()
{
    uint64_t c = DWT->CYCCNT*1000L;
    uint64_t f = static_cast<uint64_t>(System::hClkFrequency())/1000L;
    
    return c/f;
}

uint32_t System::millis()
{
    return HAL_GetTick();
}

void System::delay(uint32_t duration)
{
    HAL_Delay(duration);
}

void System::delayMicros(uint32_t duration)
{
    uint32_t us = duration;

    if(duration > 10000)
    {
        const uint32_t ms = duration/1000;

        us -= ms*1000;
        delay(ms);
    }

    const uint64_t start = micros();

    while(micros() - start < us);
}

[[noreturn]] void System::reset()
{
    HAL_NVIC_SystemReset();
    for(;;);
}

void System::sleep(bool disableSystick)
{
/* @todo */
#if defined(STM32F4xx)
    if(disableSystick)
        HAL_SuspendTick();

    HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);

    if(disableSystick)
        HAL_ResumeTick();
#endif
}

void System::stop()
{
/* @todo */
#if defined(STM32F4xx)
    HAL_PWREx_EnableFlashPowerDown();

    setAllGPIOPortClockEnabled(false);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    setAllGPIOPortClockEnabled(true);

    System::initMainClock();
#endif
}

[[noreturn]] void System::standby()
{
    HAL_PWR_EnterSTANDBYMode();
    for(;;);
}

#if defined(HAL_RTC_MODULE_ENABLED)

bool System::stopFor(uint32_t s)
{
    if(!rtc.isInitialized())
        return true;

    auto dt = (rtc.getDateTime() + DateTime::Time::fromSeconds(s)).getTime();

    if(rtc.setAlarm(dt))
        return true;

    stop();

    return false;
}

bool System::sleepFor(uint32_t s)
{
    if(!rtc.isInitialized())
        return true;

    auto dt = (rtc.getDateTime() + DateTime::Time::fromSeconds(s)).getTime();

    if(rtc.setAlarm(dt))
        return true;
        
    sleep(true);

    return false;
}

#endif

void System::enableSleepUnderDebug(bool enabled)
{
    constexpr uint16_t DBG_SLEEP = (1<<0);
    if(enabled)
        DBGMCU->CR &= (~DBG_SLEEP);
    else
        DBGMCU->CR |= DBG_SLEEP;
}

void System::enableStopUnderDebug(bool enabled)
{
    constexpr uint16_t DBG_STOP = (1<<1);
    if(enabled)
        DBGMCU->CR &= (~DBG_STOP);
    else
        DBGMCU->CR |= DBG_STOP;
}

extern "C" void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}


/*
 sbrk
 Increase program data space.
 Malloc and related functions depend on this
 Source: http://stm32discovery.nano-age.co.uk/open-source-development-with-the-stm32-discovery/getting-newlib-to-work-with-stm32-and-code-sourcery-lite-eabi
 */
extern "C" caddr_t _sbrk(int incr)
{
    extern char _ebss; // Defined by the linker
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
        heap_end = &_ebss;

    prev_heap_end = heap_end;

	char * stack = (char*) __get_MSP();
	if (heap_end + incr > stack)
		return  (caddr_t) -1;

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}
