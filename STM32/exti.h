/**
    \file exti.h
    \author Arnaud CADOT
    \version 0.1
**/

#ifndef GUARD_EXTI
#define GUARD_EXTI

#define ENABLE_EXTI_WRAPPER

#if defined(ENABLE_EXTI_WRAPPER)

#include <cstdint>

//#define EXTI_ENABLE_CPP11_CALLBACKS

#ifdef EXTI_ENABLE_CPP11_CALLBACKS
    #include <functional>
#endif

extern "C" void EXTI0_IRQHandler    (void);
extern "C" void EXTI1_IRQHandler    (void);
extern "C" void EXTI2_IRQHandler    (void);
extern "C" void EXTI3_IRQHandler    (void);
extern "C" void EXTI4_IRQHandler    (void);
extern "C" void EXTI9_5_IRQHandler  (void);
extern "C" void EXTI15_10_IRQHandler(void);

class Exti
{
    static constexpr uint16_t MAX_CALLBACKS = 4;

    struct CallbackList
    {
    #ifdef EXTI_ENABLE_CPP11_CALLBACKS
        std::function<void (void)> list[Exti::MAX_CALLBACKS];
    #else
        void (*list[Exti::MAX_CALLBACKS])(void) = {nullptr};
     #endif
    };

    public:
        static constexpr uint8_t DEFAULT_NVIC_PRIORITY          = 3;
        static constexpr uint8_t DEFAULT_NVIC_SUBGROUP_PRIORITY = 3;

    public:
        Exti();

        void enableInterruptLine(uint8_t line, uint8_t priority = DEFAULT_NVIC_PRIORITY, uint8_t subPriority = DEFAULT_NVIC_SUBGROUP_PRIORITY);

        #ifdef EXTI_ENABLE_CPP11_CALLBACKS
        bool registerCallback(uint8_t line, const std::function<void (void)>& callback,
                              uint8_t priority = DEFAULT_NVIC_PRIORITY, uint8_t subPriority = DEFAULT_NVIC_SUBGROUP_PRIORITY);
        #else
        bool registerCallback(uint8_t line, void (*callback)(void),
                              uint8_t priority = DEFAULT_NVIC_PRIORITY, uint8_t subPriority = DEFAULT_NVIC_SUBGROUP_PRIORITY);
        #endif

    private:
        CallbackList m_callbacks[16];

        void irqHandler(uint8_t pin);

        friend void ::EXTI0_IRQHandler      (void);
        friend void ::EXTI1_IRQHandler      (void);
        friend void ::EXTI2_IRQHandler      (void);
        friend void ::EXTI3_IRQHandler      (void);
        friend void ::EXTI4_IRQHandler      (void);
        friend void ::EXTI9_5_IRQHandler    (void);
        friend void ::EXTI15_10_IRQHandler  (void);
};

    extern Exti exti;
#endif

#endif