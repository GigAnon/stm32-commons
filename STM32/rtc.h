#ifndef GUARD_RTC
#define GUARD_RTC

#include "hal.h"

#if defined(HAL_RTC_MODULE_ENABLED)

#include "datetime.h"

extern "C" void RTC_Alarm_IRQHandler(void);
extern "C" void RTC_WKUP_IRQHandler(void);

/**
    \brief Wrapper for RTC peripheral
**/
class RealTimeClock
{
    public:
    #if defined(STM32F4xx)
        static uint8_t* const BSRAM;                    ///< Backup SRAM start address
        static constexpr uint32_t BSRAM_SIZE = 4096;    ///< 4KB backup SRAM size
        /* Alternatively, it is possible to use a modified linker script to add a .backup
        SRAM section, then use it like so:
        __attribute__((section(".backup"))) uint8_t p;
        */
    #endif

    public:
        /**
            Constructor.
        **/
        RealTimeClock();

        /**
            \brief Initialize the RTC peripheral.
            \param useExternalOscillator Set to \c true to use LSE, \c false for LSI.
            \returns \c true on error, \c false otherwise
        **/
        bool init(bool useExternalOscillator = false);

        /**
            \returns RTC date & time
        **/
        DateTime getDateTime();

        /**
            \brief Set the RTC date and time
            \param dt New date and time
            \returns \c true on error, \c false otherwise
        **/
        bool setDateTime(const DateTime& dt);

        /**
            \brief Set the RTC alarm
            \todo WIP
        **/
        bool setAlarm(const DateTime::Time& when);

#if !defined(STM32F1xx)
        /**
            \brief Configure the 'wake up' RTC interrupt delay
            \param us Delay, in microseconds, between two interrupts. Set to 0 to disable.
            \param wakeupCallback Callback function to be called after waking up. Set to nullptr to disable.
            \returns \c true on error, \c false otherwise

            \remark Minimum delay is ~127us, maximum is ~32s
        **/
        bool configureWakeUpTimer(uint32_t us, void (*wakeupCallback) (void) = nullptr);
#endif

        bool isInitialized() const;

    private:
        RTC_HandleTypeDef m_rtc = {};

        void wakeupHandler();

        void (*m_wakeupCallback) (void) = nullptr;

        friend void ::RTC_Alarm_IRQHandler();
        friend void ::RTC_WKUP_IRQHandler(void);
};

extern RealTimeClock rtc;

#endif /* #if defined(HAL_RTC_MODULE_ENABLED) */

#endif /* #ifndef GUARD_RTC */
