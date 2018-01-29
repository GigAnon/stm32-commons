#include "uart.h"
#include "system.h"

void mdelay(uint32_t ms)
{
    System::delay(ms);
}


void SN8200_HAL_Init(uint32_t baudrate)
{
    uart1.init(baudrate, true);
}


void SN8200_HAL_SendData(unsigned char *buf, int len)
{
    uart1.write(reinterpret_cast<const uint8_t*>(buf), len);
}

bool SN8200_HAL_RxBufferEmpty(void)
{
    return uart1.dataAvailable() == 0;
}

uint8_t SN8200_HAL_ReadByte(void)
{
    return uart1.read();
}
