/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX1276 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
 /*******************************************************************************
  * @file    sx1276mb1mas.c
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    27-February-2017
  * @brief   driver sx1276mb1mas board
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/

extern "C"
{
    #include "../../wan/hw.h"
    #include "../../phy/radio.h"
    #include "../../utilities/delay.h"
}

#include "sx1276.h"
#include "rfm95.h"

#include "pin.h"
#include "exti.h"

static Pin resetPin (GPIOA, 3);
static Pin nssPin   (GPIOA, 4);
static Pin dio0Pin  (GPIOB, 10);
static Pin dio1Pin  (GPIOB, 3);
static Pin dio2Pin  (GPIOB, 5);
static Pin dio3Pin  (GPIOB, 4);

#define IRQ_HIGH_PRIORITY  0


/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1276IoInit,
    SX1276IoDeInit,
    SX1276Init,
    SX1276GetStatus,
    SX1276SetModem,
    SX1276SetChannel,
    SX1276IsChannelFree,
    SX1276Random,
    SX1276SetRxConfig,
    SX1276SetTxConfig,
    SX1276CheckRfFrequency,
    SX1276GetTimeOnAir,
    SX1276Send,
    SX1276SetSleep,
    SX1276SetStby, 
    SX1276SetRx,
    SX1276StartCad,
    SX1276SetTxContinuousWave,
    SX1276ReadRssi,
    SX1276Write,
    SX1276Read,
    SX1276WriteBuffer,
    SX1276ReadBuffer,
    SX1276SetSyncWord,
    SX1276SetMaxPayloadLength
};


void SX1276IoInit(void)
{
    dio0Pin.init(GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_HIGH);
    dio1Pin.init(GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_HIGH);
    dio2Pin.init(GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_HIGH);
    dio3Pin.init(GPIO_MODE_IT_RISING, GPIO_PULLDOWN, GPIO_SPEED_HIGH);

    nssPin.init(GPIO_MODE_OUTPUT_PP, GPIO_PULLUP);
    nssPin.setHigh();
}

void SX1276IoIrqInit( DioIrqHandler **irqHandlers )
{
    if(irqHandlers[0])
        exti.registerCallback(dio0Pin.pin(), irqHandlers[0], IRQ_HIGH_PRIORITY);

    if(irqHandlers[1])
        exti.registerCallback(dio1Pin.pin(), irqHandlers[1], IRQ_HIGH_PRIORITY);
    
    if(irqHandlers[2])
        exti.registerCallback(dio2Pin.pin(), irqHandlers[2], IRQ_HIGH_PRIORITY);

    if(irqHandlers[3])
        exti.registerCallback(dio3Pin.pin(), irqHandlers[3], IRQ_HIGH_PRIORITY);
}


void SX1276IoDeInit( void )
{
    dio0Pin.init(GPIO_MODE_INPUT);
    dio1Pin.init(GPIO_MODE_INPUT);
    dio2Pin.init(GPIO_MODE_INPUT);
    dio3Pin.init(GPIO_MODE_INPUT);
}

void SX1276Reset( void )
{
    resetPin.init(GPIO_MODE_OUTPUT_PP);
    resetPin.setLow();

    DelayMs(1);

    resetPin.init(GPIO_MODE_INPUT);
}

void SX1276BeginSpi(void)
{
    nssPin.setLow();
}

void SX1276EndSpi(void)
{
    nssPin.setHigh();
}

void SX1276SetRfTxPower( int8_t power )
{
#if 1
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    /* Set current limit to 240mA */
    SX1276Write( REG_OCP, 0x3B);

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );
    paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;

    /* For RFM95 PABOOST should always be selected */
    if(( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST)
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power < -1 )
        {
            power = -1;
        }
        if( power > 14 )
        {
            power = 14;
        }
        paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }

    SX1276Write( REG_PACONFIG, paConfig);
    SX1276Write( REG_PADAC, paDac);

#else
    /** Hack for RFM95 modules: PA_BOOST must be used (it appears) **/

    /** @todo Check current limiter config **/
    SX1276Write( REG_OCP, 0x3B);    /* Set current limit to 240mA */
    SX1276Write( REG_PACONFIG,  0x8f); /* Enable PA boost and set power to max */
    SX1276Write( REG_PADAC,     0x07); /* Enable PA boost (again?) */
#endif
}
uint8_t SX1276GetPaSelect( uint32_t channel )
{
    return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1276SetAntSwLowPower(bool status)
{
}

void SX1276SetAntSw(uint8_t opMode)
{
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
