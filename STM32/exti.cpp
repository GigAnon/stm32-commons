#include "exti.h"
#include "hal.h"

#ifdef ENABLE_EXTI_WRAPPER

Exti exti;

Exti::Exti() {}

void Exti::irqHandler(uint8_t pin)
{
    auto p = m_callbacks[pin].list;
    while(*p)
        (*(p++))();
}

#ifdef EXTI_ENABLE_CPP11_CALLBACKS
bool Exti::registerCallback(uint8_t line, const std::function<void (void)>& callback,
                            uint8_t priority, uint8_t subPriority)
#else
bool Exti::registerCallback(uint8_t line, void (*callback)(void),
                            uint8_t priority, uint8_t subPriority)
#endif
{
    if(line > 15)
        return true;

    size_t i = 0;
    while(m_callbacks[line].list[i++]);

    --i;

    if(i >= Exti::MAX_CALLBACKS)
        return true;

    if(i == 0)
        enableInterruptLine(line);

    m_callbacks[line].list[i] = callback;

    return false;
}



#define ENABLE_IRQ(i) { HAL_NVIC_SetPriority(i, priority, subPriority); \
                        HAL_NVIC_EnableIRQ(i); }

#define ENABLE_IRQ_COND(a, i) if(line == a) { ENABLE_IRQ(i); }

void Exti::enableInterruptLine(uint8_t line, uint8_t priority, uint8_t subPriority)
{
    ENABLE_IRQ_COND(0, EXTI0_IRQn);
    ENABLE_IRQ_COND(1, EXTI1_IRQn);
    ENABLE_IRQ_COND(2, EXTI2_IRQn);
    ENABLE_IRQ_COND(3, EXTI3_IRQn);
    ENABLE_IRQ_COND(4, EXTI4_IRQn);

    if(line >= 5 && line <= 9)
        ENABLE_IRQ(EXTI9_5_IRQn);
    if(line >= 10 && line <= 15)
        ENABLE_IRQ(EXTI15_10_IRQn);
}

#undef ENABLE_IRQ
#undef ENABLE_IRQ_COND

#define LINE_CHECK(a) if(EXTI->PR & EXTI_PR_PR##a) { \
                        EXTI->PR = EXTI_PR_PR##a; \
                        exti.irqHandler(a); }

extern "C" void EXTI9_5_IRQHandler(void)
{
    LINE_CHECK(5);
    LINE_CHECK(6);
    LINE_CHECK(7);
    LINE_CHECK(8);
    LINE_CHECK(9);
}

extern "C" void EXTI15_10_IRQHandler(void)
{
    LINE_CHECK(10);
    LINE_CHECK(11);
    LINE_CHECK(12);
    LINE_CHECK(13);
    LINE_CHECK(14);
    LINE_CHECK(15);
}

extern "C" void EXTI0_IRQHandler(void) { LINE_CHECK(0); }
extern "C" void EXTI1_IRQHandler(void) { LINE_CHECK(1); }
extern "C" void EXTI2_IRQHandler(void) { LINE_CHECK(2); }
extern "C" void EXTI3_IRQHandler(void) { LINE_CHECK(3); }
extern "C" void EXTI4_IRQHandler(void) { LINE_CHECK(4); }

#undef LINE_CHECK

#else

/** HAL default handlers **/
extern "C"
{
void EXTI0_IRQHandler( void )
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}


void EXTI9_5_IRQHandler(void)
{  
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_14);
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}
}

#endif
