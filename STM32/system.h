/**
    \file system.h
    \author Arnaud CADOT
    \version 1.0
**/
#ifndef GUARD_RCC
#define GUARD_RCC

#include "hal.h"

/**
    \brief System namespace for clocks setup, general purpose functions, etc.
**/
namespace System
{
     /**  Clock settings **/
    /** See STM32CubeMX **/
    #if defined(STM32F1xx)
	    constexpr uint32_t HSE_PREDIV		= RCC_HSE_PREDIV_DIV1;  ///< STM32F1 HSE prescaler
	    constexpr uint32_t PLL_MUL			= RCC_PLL_MUL9;         ///< STM32F1 PLL multiplier
    #else
        #if HSE_VALUE == 8000000U
            constexpr uint32_t PLL_M = 4;   ///< PPL M factor (8Mhz HSE)
            constexpr uint32_t PLL_N = 168; ///< PPL N factor (8Mhz HSE)
            constexpr uint32_t PLL_P = 2;   ///< PPL P factor (8Mhz HSE)
            constexpr uint32_t PLL_Q = 4;   ///< PLL Q factor (8Mhz HSE)
        #elif HSE_VALUE == 16000000U
            /* @todo */
        #endif
    #endif
        
    #if defined(STM32F1xx)
	    constexpr uint32_t AHB_PRESCALER	= RCC_SYSCLK_DIV1;  ///< STM32F1 AHB prescaler
	    constexpr uint32_t APB1_PRESCALER	= RCC_HCLK_DIV2;    ///< STM32F1 APB1 prescaler
	    constexpr uint32_t APB2_PRESCALER	= RCC_HCLK_DIV1;    ///< STM32F1 APB2 prescaler
	
	    constexpr uint32_t FLASH_LATENCY	= FLASH_LATENCY_1;  ///< STM32F1 flash latency
    #else
        constexpr uint32_t AHB_PRESCALER	= 1;
	    constexpr uint32_t APB1_PRESCALER	= RCC_HCLK_DIV4;
	    constexpr uint32_t APB2_PRESCALER	= RCC_HCLK_DIV2;
	
	    constexpr uint32_t FLASH_LATENCY	= FLASH_LATENCY_6;
    #endif
	
        bool init();

	    /**
            \brief Initialize main clock
            \returns \c true on error, \c false otherwise
        **/
	    bool initMainClock();

        /**
            \returns Core clock frequency
        **/
	    uint32_t hClkFrequency();

        /**
            \param port GPIO port to enable the clock of
        **/
	    void enableGPIOPortClock(GPIO_TypeDef* port);

        /**
            \param port GPIO port to enable/disable the clock off
            \param enabled \c true to enable the clock, \c false otherwise
        **/
        void setGPIOPortClockEnabled(GPIO_TypeDef* port, bool enabled);

        /**
            \brief Enable/disable all the GPIO port clocks
            \param enabled \c true to enable the clocks, \c false to disable them
        **/
        void setAllGPIOPortClockEnabled(bool enabled);
    
        /**
            \brief Microsecond-resolution clock, for benchmarking purposes
            \todo Fix it...
        **/
        uint64_t micros();

        /**
            \brief Millisecond-resolution clock
            \remark Will overflow after 49 days
        **/
        uint32_t millis();

        /**
            \brief Active pause (ms resolution)
            \param duration Pause duration (in ms)
        **/
        void delay(uint32_t duration);

        /**
            \brief Active pause (us resolution)
            \param duration Pause duration (in us)
            \todo Tests?
        **/
        void delayMicros(uint32_t duration);

        /**
            \brief Resets the MCU
            \remark This function does not return.
        **/
        [[noreturn]] void reset();

        /**
            \brief Put the MCU to sleep (main clock off)
            \param disableSystick \c true to disable systick, \c false otherwise
        **/
        void sleep(bool disableSystick = false);

        /**
            \brief Put the MCU in stop mode (all clocks and peripherals off)
        **/
        void stop();

        /**
            \brief Put the MCU into standby mode.
        **/
        [[noreturn]] void standby();

    #if defined(HAL_RTC_MODULE_ENABLED)
        /**
            \brief Use the RTC Alarm to stop the MCU for a set duration
            \param s Seconds to stop for
            \returns \c true on error, \c false otherwise

            \remark RTC must be enabled and running
        **/
        bool stopFor(uint32_t s);

         /**
            \brief Use the RTC Alarm to go to sleep for a set duration
            \param s Seconds to sleep for
            \returns \c true on error, \c false otherwise

            \remark RTC must be enabled and running
        **/
        bool sleepFor(uint32_t s);
    #endif

        /**
            \brief Allow the MCU to sleep even if connected to a debugger
            \param enabled \c true to allow MCU to sleep, false to forbid it

            \remark When disabled, it's possible the MCU will ignore the sleep instructions altogether.
        **/
        void enableSleepUnderDebug(bool enabled);

        /**
            \brief Allow the MCU to sleep even if connected to a debugger
            \param enabled \c true to allow MCU to sleep, false to forbid it

            \warning This will most likely crash your debugger!
            \remark When disabled, it's very likely the MCU will ignore the stop instruction altogether.
        **/
        void enableStopUnderDebug(bool enabled);
}

#endif
