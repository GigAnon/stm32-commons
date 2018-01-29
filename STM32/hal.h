#ifndef GUARD_HAL
#define GUARD_HAL

#if defined(STM32F1xx)
    #include <stm32f1xx_hal.h>
#elif defined(STM32F4xx)
    #include <stm32f4xx_hal.h>
#else
    #error "Please #define STM32F1xx/STM32F4xx/etc."
#endif

/* For (u)intXX_t types */
#if __cplusplus
    #include <cstdint>
#else
    #include <stdint.h>
#endif

//#define ENABLE_EXTI_WRAPPER

#endif
