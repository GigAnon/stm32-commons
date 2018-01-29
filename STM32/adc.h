#ifndef GUARD_ADC
#define GUARD_ADC

#include "hal.h"

#if defined(HAL_ADC_MODULE_ENABLED)

/**
    \brief Basic overlay for ADC peripherals

    This class provides a *very* basic interface to poll ADC channels.
**/
class ADCPeriph
{
    public:
        static constexpr uint8_t NUM_CHANNELS = 16;

   #if defined(STM32F1xx)
        static constexpr uint32_t DEFAULT_SAMPLE_TIME = ADC_SAMPLETIME_13CYCLES_5;
   #else
        static constexpr uint32_t DEFAULT_SAMPLE_TIME = ADC_SAMPLETIME_15CYCLES;
   #endif
    
        /**
            \brief Constructor
            \param adc ADC peripheral instance
        **/
        ADCPeriph(ADC_TypeDef* adc);
        
        /**
            \brief Initialize ADC peripheral
            \returns \c true on error, \c false otherwise
        **/
        bool init();
    
        /**
            \returns \c true if ADC peripheral has been initialized by the object, \c false otherwise
        **/
        bool isInitialized() const;
    
        /**
            \brief Read an ADC channel via polling
            \param channel Channel to be read
            \param sampleTime ADC sample time
            \param superSampling Super-sampling count (number of samples to be averaged)
        **/
        uint16_t pollChannel(uint8_t channel, uint32_t sampleTime = DEFAULT_SAMPLE_TIME, uint8_t superSampling = 1);
    
    private:
        ADC_HandleTypeDef m_handle;
        bool m_initialized = false;
};

#if defined(ADC1)
    extern ADCPeriph adc1; ///< Singleton instance for ADC1 peripheral
#endif
#if defined(ADC2)
    extern ADCPeriph adc2; ///< Singleton instance for ADC2 peripheral
#endif

#endif /* #if defined(HAL_ADC_MODULE_ENABLED) */
#endif
