#ifndef GUARD_RNG
#define GUARD_RNG

#include "hal.h"

#if defined(HAL_RNG_MODULE_ENABLED)

namespace Rand
{
    constexpr uint32_t RAND_MAX = 0xFFFFFFFF;

    bool init();

    uint32_t get(HAL_StatusTypeDef* error = nullptr);

    uint32_t range(uint32_t start, uint32_t end, HAL_StatusTypeDef* error = nullptr);
};

#endif /* #if defined(HAL_RNG_MODULE_ENABLED) */
#endif /* #ifndef GUARD_RNG */
