#include "adc.h"

#if defined(HAL_ADC_MODULE_ENABLED)

#if defined(STM32F1xx)
    #include <stm32f1xx_hal_adc_ex.h>
#endif

#if defined(ADC1)
    ADCPeriph adc1(ADC1);
#endif
#if defined(ADC2)
    ADCPeriph adc2(ADC2);
#endif

ADCPeriph::ADCPeriph(ADC_TypeDef* adc)
{
    m_handle.Instance = adc;
}

#define CLK_COND_ENABLE(a, x) { if(a == x) __##x##_CLK_ENABLE(); }
#define CONF_CHAN(c) {  conf.Channel = ADC_CHANNEL_##c; \
                        conf.Rank = (c+1); \
                        HAL_ADC_ConfigChannel(&m_handle, &conf); }

bool ADCPeriph::init()
{
    m_initialized = false;
    
    CLK_COND_ENABLE(m_handle.Instance, ADC1);
    CLK_COND_ENABLE(m_handle.Instance, ADC2);
    
    m_handle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    m_handle.Init.ContinuousConvMode    = ENABLE;
    m_handle.Init.NbrOfConversion       = 16;
    m_handle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    
#if defined(STM32F1xx)
    m_handle.Init.ScanConvMode          = ADC_SCAN_DISABLE; // WIP (use NVIC or DMA in the future?)
#endif
    
    m_handle.Init.DiscontinuousConvMode = DISABLE; // Discarded if ScanConvMode or ContinuousConvMode are enabled
    m_handle.Init.NbrOfDiscConversion   = 8; // Dito
    
    if(HAL_ADC_Init(&m_handle) != HAL_OK)
        return true;
    
    HAL_Delay(1);
    
#if defined(STM32F1xx)
    HAL_ADCEx_Calibration_Start(&m_handle);
#endif

    m_initialized = true;
    
    return false;
}

bool ADCPeriph::isInitialized() const
{
    return m_initialized;
}

uint16_t ADCPeriph::pollChannel(uint8_t channel, uint32_t sampleTime, uint8_t superSampling)
{
    HAL_ADC_Init(&m_handle);
    
    ADC_ChannelConfTypeDef conf;
 
	conf.Channel = channel;
	conf.Rank = 1;
	conf.SamplingTime = sampleTime;
 
	if (HAL_ADC_ConfigChannel(&m_handle, &conf) != HAL_OK)
    	return 0xFFFF;
    
    HAL_ADC_Start(&m_handle);
    
    uint16_t val = 0xFFFF;
    
    uint32_t t = 0;
    
    uint32_t ss = superSampling;
    
    for(uint32_t i=0;i<ss;++i)
    {
        while(HAL_ADC_PollForConversion(&m_handle, 2000) != HAL_OK);

        t += HAL_ADC_GetValue(&m_handle);
    }
    
    val = t / ss;
    
    HAL_ADC_Stop(&m_handle);
    
    return val;
}

#endif /* #if defined(HAL_ADC_MODULE_ENABLED) */
