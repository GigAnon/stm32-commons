#include "stm32f4xx.h"
#include "system.h"

void mdelay(uint32_t ms)
{
   /* TimingDelay = ms;

    while(TimingDelay != 0);*/
    System::delay(ms);
}
