#include "button.h"
#include "system.h"

Button::Button(Pin pin): m_pin(pin)
{}

bool Button::init(bool usePullup)
{
    if(!m_pin.isValid())
        return true;

    return m_pin.init(GPIO_MODE_INPUT, usePullup?GPIO_PULLUP:GPIO_PULLDOWN);
}

bool Button::update()
{
    bool r = false;

    if(m_pin.read())
    {
        if(m_downTime == 0)
            m_downTime = System::millis();
    }
    else
    {
        r = (m_downTime && System::millis() - m_downTime > Button::PUSH_DELAY);

        m_downTime = 0;
    }

    return r;
}