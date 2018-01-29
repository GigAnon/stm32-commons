#ifndef GUARD_BUTTON
#define GUARD_BUTTON

#include "pin.h"

class Button
{
    static constexpr uint32_t PUSH_DELAY = 250;

    public:
        Button(Pin pin);

        bool init(bool usePullup = false);

        bool update();

    private:
        Pin m_pin;

        uint32_t m_downTime = 0;
};

#endif