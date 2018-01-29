/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2015 Semtech

Description: End device comissioning parameters

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
 /******************************************************************************
  * @file    comissioning.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    27-February-2017
  * @brief   End device comissioning parameters
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_COMISSIONING_H__
#define __LORA_COMISSIONING_H__

#ifdef __cplusplus
 extern "C" {
#endif


#define LORAWAN_DEVICE_EUI                         { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 }
#define LORAWAN_APPLICATION_EUI                    { 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x51, 0x5A }
#define LORAWAN_APPLICATION_KEY                    { 0x02, 0xB0, 0x7E, 0x01, 0x50, 0x16, 0x02, 0x80, 0xAE, 0x0D, 0x20, 0xA6, 0x0A, 0xB0, 0xF7, 0x01 }
/*
#define LORAWAN_APPLICATION_EUI                    { 0x70, 0xB3, 0xD5, 0x7E, 0xF0, 0x00, 0x57, 0xAB }
#define LORAWAN_APPLICATION_KEY                    { 0xDA, 0x41, 0xC7, 0x00, 0x6D, 0x07, 0xC0, 0xBD, 0x46, 0x68, 0x30, 0x81, 0x1C, 0x9F, 0xDF, 0x9C }
*/
#ifdef __cplusplus
}
#endif

#endif /* __LORA_COMISSIONING_H__ */
