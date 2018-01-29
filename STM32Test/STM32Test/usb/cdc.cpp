#include "cdc.h"

#include "usbd_desc.h"
#include "usbd_cdc.h"

extern "C"
{
    int8_t itfInit()
    {
        return cdc.itfInit()?USBD_FAIL:USBD_OK;
    }

    int8_t itfDeInit()
    {
        return USBD_OK;
    }

    int8_t itfControl(uint8_t cmd, uint8_t* pbuf, uint16_t length)
    {
        return cdc.itfControlPvt(cmd, pbuf, length);
    }

    int8_t itfReceive(uint8_t* buf, uint32_t *len)
    {
        return cdc.rxCallback(buf, *len)?USBD_FAIL:USBD_OK;
    }

    USBD_CDC_ItfTypeDef fops = 
    {
        itfInit,
        itfDeInit,
        itfControl,
        itfReceive
    };
}

CDC cdc;

CDC::CDC(): m_rxBuffer(RX_BUFFER_SIZE)
{
}

bool CDC::itfInit()
{
    USBD_CDC_SetTxBuffer(&m_usbHandle, m_txCdcBuffer, 0);
    USBD_CDC_SetRxBuffer(&m_usbHandle, m_rxCdcBuffer);

    return true;
}

bool CDC::init()
{
    if(USBD_Init(&m_usbHandle, &VCP_Desc, 0))
        return true;

    if(USBD_RegisterClass(&m_usbHandle, USBD_CDC_CLASS))
        return true;

    if(USBD_CDC_RegisterInterface(&m_usbHandle, &fops) != USBD_OK)
        return true;

    if(USBD_Start(&m_usbHandle) != USBD_OK)
        return true;

}

uint32_t CDC::dataAvailable() const
{
    return m_rxBuffer.size();
}

uint32_t CDC::read(uint8_t* data, uint32_t maxData)
{
    return m_rxBuffer.pull(data, maxData);
}

bool CDC::rxCallback(uint8_t* buf, uint32_t size)
{
    static uint32_t t = 0;

    m_rxBuffer.push(buf, size);

    uint8_t *p = (uint8_t*)(buf+t);
    if(p)
        asm("nop");

    t += size;
    
    return USBD_CDC_ReceivePacket(&m_usbHandle) != USBD_OK;
}

int8_t CDC::itfControlPvt(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
    switch (cmd)
    {
        case CDC_SEND_ENCAPSULATED_COMMAND:
        case CDC_GET_ENCAPSULATED_RESPONSE:
        case CDC_SET_COMM_FEATURE:
        case CDC_GET_COMM_FEATURE:
        case CDC_CLEAR_COMM_FEATURE:
        case CDC_SET_CONTROL_LINE_STATE:
        case CDC_SEND_BREAK:
            break;

        case CDC_SET_LINE_CODING:
            m_coding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                                    (pbuf[2] << 16) | (pbuf[3] << 24));
            m_coding.format     = pbuf[4];
            m_coding.paritytype = pbuf[5];
            m_coding.datatype   = pbuf[6];

            break;

        case CDC_GET_LINE_CODING:
            pbuf[0] = (uint8_t)(m_coding.bitrate);
            pbuf[1] = (uint8_t)(m_coding.bitrate >> 8);
            pbuf[2] = (uint8_t)(m_coding.bitrate >> 16);
            pbuf[3] = (uint8_t)(m_coding.bitrate >> 24);
            pbuf[4] = m_coding.format;
            pbuf[5] = m_coding.paritytype;
            pbuf[6] = m_coding.datatype;     
            break;

        default:
            break;
    }
  
    return USBD_OK;
}

