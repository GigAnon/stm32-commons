#ifndef GUARD_SN8200
#define GUARD_SN8200

#include "uart.h"
#include "pin.h"

class SN8200
{
    public:
        SN8200(Serial& uart, Pin rst = Pin());

        void reset();

        bool init();

    private:
        Serial& m_uart;
        Pin     m_rst;
        uint8_t m_seqNo = 0;
};

#endif