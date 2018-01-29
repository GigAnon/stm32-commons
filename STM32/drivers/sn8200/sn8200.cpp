#include "sn8200.h"
#include "system.h"

SN8200::SN8200(Serial& uart, Pin rst) :
    m_uart(uart), m_rst(rst)
{
}

void SN8200::reset()
{
    if(m_rst)
    {
        m_rst.setLow();
        System::delay(100);
        m_rst.setHigh();
        System::delay(100);
    }
}

bool SN8200::init()
{
    reset();

    return false;
}
