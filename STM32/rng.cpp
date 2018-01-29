#include "rng.h"

#if defined(HAL_RNG_MODULE_ENABLED)

static RNG_HandleTypeDef hdl = {};

bool Rand::init()
{
    __HAL_RCC_RNG_CLK_ENABLE();
    hdl.Instance = RNG;

    return HAL_RNG_Init(&hdl) != HAL_OK;
}

uint32_t Rand::get(HAL_StatusTypeDef* error)
{
    HAL_StatusTypeDef r;
    uint32_t rn = 0;

    if(hdl.Instance)
        r = HAL_RNG_GenerateRandomNumber(&hdl, &rn);
    else
        r = HAL_ERROR;

    if(error)
        *error = r;

    return rn;
}

uint32_t Rand::range(uint32_t start, uint32_t end, HAL_StatusTypeDef* error)
{
    HAL_StatusTypeDef r;
    uint32_t rn = 0;

    if(start <= end)
    {
        rn = Rand::get(&r);

        if(r == HAL_OK)
            rn = start + rn / (Rand::RAND_MAX / (end - start) + 1);
    }
    else
        r = HAL_ERROR;

    if(error)
        *error = r;

    return rn;
}

#endif /* #if defined(HAL_RNG_MODULE_ENABLED) */
