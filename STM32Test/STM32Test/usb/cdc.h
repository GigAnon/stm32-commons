#ifndef GUARD_CDC
#define GUARD_CDC

#include <cstdint>
#include "usbd_core.h"
#include "usbd_cdc.h"

#include "circular_buffer.h"


class CDC
{
    constexpr static uint32_t RX_BUFFER_SIZE = 256;

    constexpr static uint32_t CDC_BUFFER_SIZE = 256;

    public:
        CDC();

        bool init();

        uint32_t dataAvailable() const;
        uint32_t read(uint8_t* data, uint32_t maxData);

        int8_t itfControlPvt(uint8_t cmd, uint8_t* pbuf, uint16_t length);
        bool itfInit();

        bool rxCallback(uint8_t* buf, uint32_t size);

        void update();

    private:
        USBD_HandleTypeDef m_usbHandle;

        CircularBuffer m_rxBuffer;

        uint8_t m_rxCdcBuffer[CDC_BUFFER_SIZE] = {0};

        uint8_t m_txCdcBuffer[CDC_BUFFER_SIZE] = {0};

        USBD_CDC_LineCodingTypeDef m_coding = /* Default coding settings */
          {
            115200, /* baud rate*/
            0x00,   /* stop bits-1*/
            0x00,   /* parity - none*/
            0x08    /* nb. of bits 8*/
          };
};

extern CDC cdc;

#endif