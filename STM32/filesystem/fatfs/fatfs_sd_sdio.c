/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2015
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "fatfs_sd_sdio.h"

#if SD_USE_DETECT
    #define SD_DETECT_PORT  GPIOB
    #define SD_DETECT_PIN   15
#endif

#if SD_USE_WRITE_PROTECT
    #define SD_WRITE_PROTECT_PORT   GPIOB
    #define SD_WRITE_PROTECT_PIN    7
#endif

/* Status of SDCARD */
static volatile DSTATUS Stat = STA_NOINIT;
static SD_HandleTypeDef uSdHandle;

/**
  * @brief  Initializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_Init(void)
{ 
	uint8_t SD_state = MSD_OK;

	/* uSD device interface configuration */
	uSdHandle.Instance = SDIO;

	uSdHandle.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
	uSdHandle.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
	uSdHandle.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
	uSdHandle.Init.BusWide             = SDIO_BUS_WIDE_1B;
	uSdHandle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	uSdHandle.Init.ClockDiv            = SDIO_TRANSFER_CLK_DIV;


	/* Init GPIO, DMA and NVIC */
	BSP_SD_MspInit(&uSdHandle, NULL);

	/* Check if the SD card is plugged in the slot */
	if (!BSP_SD_IsDetected())
		return MSD_ERROR;

	/* HAL SD initialization */
	if (HAL_SD_Init(&uSdHandle) != HAL_OK)
		return MSD_ERROR;

	return (HAL_SD_ConfigWideBusOperation(&uSdHandle, SDIO_BUS_WIDE_4B) != HAL_OK) ? MSD_ERROR : MSD_OK;
}

/**
  * @brief  DeInitializes the SD card device.
  * @retval SD status
  */
uint8_t BSP_SD_DeInit(void)
{ 
    uint8_t sd_state = MSD_OK;

    uSdHandle.Instance = SDIO;

    /* HAL SD deinitialization */
    if(HAL_SD_DeInit(&uSdHandle) != HAL_OK)
        sd_state = MSD_ERROR;

    uSdHandle.Instance = SDIO;
    BSP_SD_MspDeInit(&uSdHandle, NULL);

    return sd_state;
}

/**
  * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  ReadAddr: Address from where data is to be read  
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to read 
  * @retval SD status
  */
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint64_t sector, uint32_t NumOfBlocks, uint32_t Timeout)
{
    return (HAL_SD_ReadBlocks(&uSdHandle, (uint8_t *)pData, sector, NumOfBlocks, Timeout) != HAL_OK) ? MSD_ERROR : MSD_OK;
}

/**
  * @brief  Writes block(s) to a specified address in an SD card, in polling mode. 
  * @param  pData: Pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written  
  * @param  BlockSize: SD card data block size, that should be 512
  * @param  NumOfBlocks: Number of SD blocks to write
  * @retval SD status
  */
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint64_t sector, uint32_t NumOfBlocks, uint32_t Timeout)
{
    return (HAL_SD_WriteBlocks(&uSdHandle, (uint8_t *)pData, sector, NumOfBlocks, Timeout) != HAL_OK) ? MSD_ERROR : MSD_OK;
}

/**
  * @brief  Erases the specified memory area of the given SD card. 
  * @param  StartAddr: Start byte address
  * @param  EndAddr: End byte address
  * @retval SD status
  */
uint8_t BSP_SD_Erase(uint64_t StartAddr, uint64_t EndAddr)
{
    return (HAL_SD_Erase(&uSdHandle, StartAddr, EndAddr) != HAL_OK) ? MSD_ERROR : MSD_OK;
}

/**
  * @brief  Initializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params
  * @retval None
  */
__weak void BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
	__HAL_RCC_SDIO_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_Init_Structure;
    GPIO_Init_Structure.Mode        = GPIO_MODE_AF_PP;
    GPIO_Init_Structure.Pull        = GPIO_PULLUP;
    GPIO_Init_Structure.Speed       = GPIO_SPEED_HIGH;
    GPIO_Init_Structure.Alternate   = GPIO_AF12_SDIO;

    GPIO_Init_Structure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

    GPIO_Init_Structure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &GPIO_Init_Structure);

    /* SD card detection pin */
#if defined(SD_USE_DETECT)
    GPIO_Init_Structure.Mode        = GPIO_MODE_INPUT;
    GPIO_Init_Structure.Pull        = GPIO_PULLUP;
    GPIO_Init_Structure.Speed       = GPIO_SPEED_LOW;
    GPIO_Init_Structure.Alternate   = 0;

    GPIO_Init_Structure.Pin         = SD_DETECT_PIN;
    HAL_GPIO_Init(SD_DETECT_PORT, &GPIO_Init_Structure);
#endif

    /* Write protect pin */
#if defined(SD_WRITE_PROTECT_PORT)
    GPIO_Init_Structure.Mode        = GPIO_MODE_INPUT;
    GPIO_Init_Structure.Pull        = GPIO_PULLUP;
    GPIO_Init_Structure.Speed       = GPIO_SPEED_LOW;
    GPIO_Init_Structure.Alternate   = 0;
    GPIO_Init_Structure.Pin         = SD_WRITE_PROTECT_PIN;
    HAL_GPIO_Init(SD_WRITE_PROTECT_PORT, &GPIO_Init_Structure);
#endif
    
    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
}

/**
  * @brief  Initializes the SD Detect pin MSP.
  * @param  hsd: SD handle
  * @param  Params
  * @retval None
  */
__weak void BSP_SD_Detect_MspInit(SD_HandleTypeDef *hsd, void *Params)
{
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params
  * @retval None
  */
__weak void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params)
{
    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);

    /* DeInit GPIO pins can be done in the application
    (by surcharging this __weak function) */

    /* Disable SDMMC1 clock */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /* GPIO pins clock and DMA clocks can be shut down in the application
        by surcharging this __weak function */
}

/**
  * @brief  Handles SD card interrupt request.
  * @retval None
  */
void SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&uSdHandle);
}

/**
  * @brief  Handles SD DMA Tx transfer interrupt request.
  * @retval None
  */
void SD_DMAx_Tx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uSdHandle.hdmatx); 
}

/**
  * @brief  Handles SD DMA Rx transfer interrupt request.
  * @retval None
  */
void SD_DMAx_Rx_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uSdHandle.hdmarx);
}

/**
  * @brief  Gets the current SD card data status.
  * @retval Data transfer state.
  *          This value can be one of the following values:
  *            @arg  0: No data transfer is acting
  *            @arg  1: Data transfer is acting
  *            @arg  SD_TRANSFER_ERROR: Data transfer error 
  */
uint8_t BSP_SD_GetStatus(void)
{
    return (HAL_SD_GetCardState(&uSdHandle) == HAL_SD_CARD_TRANSFER) ? 0 : 1;
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  * @retval None 
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo)
{
    /* Get SD card Information */
    HAL_SD_GetCardInfo(&uSdHandle, CardInfo);
}

/**************************************************************/
/*                  SDCARD WP AND DETECT                      */
/**************************************************************/
/* SDCARD detect function */
__weak uint8_t BSP_SD_IsDetected(void)
{
#if defined(SD_USE_DETECT)
	return !HAL_GPIO_ReadPin(SD_DETECT_PORT, SD_DETECT_PIN);
#else    return 1;#endif
}

/* SDCARD write protect function */
static uint8_t SDCARD_IsWriteEnabled(void)
{
#if defined(SD_WRITE_PROTECT_PORT)
	return !HAL_GPIO_ReadPin(SD_WRITE_PROTECT_PORT, SD_WRITE_PROTECT_PIN);
#else
    return 1;
#endif
}

/**************************************************************/
/*                    LOW LEVEL FUNCTIONS                     */
/**************************************************************/
DSTATUS TM_FATFS_SD_SDIO_disk_initialize(void)
{
	Stat = STA_NOINIT;

	/* Configure the SDCARD device */
	if (BSP_SD_Init() == MSD_OK)
		Stat &= ~STA_NOINIT;
	else
		Stat |= STA_NOINIT;

	return Stat;
}

DSTATUS TM_FATFS_SD_SDIO_disk_status(void)
{
	Stat = STA_NOINIT;

	/* Check SDCARD status */
	if (BSP_SD_GetStatus() == MSD_OK)
		Stat &= ~STA_NOINIT;
	else
		Stat |= STA_NOINIT;

	/* Check if write enabled */
	if (SDCARD_IsWriteEnabled())
		Stat &= ~STA_PROTECT;
	else
		Stat |= STA_PROTECT;

	return Stat;
}

DRESULT TM_FATFS_SD_SDIO_disk_ioctl(BYTE cmd, void *buff)
{
	DRESULT res = RES_ERROR;
	HAL_SD_CardInfoTypeDef CardInfo = {0};
  
	/* Check if init OK */
	if (Stat & STA_NOINIT)
		return RES_NOTRDY;
  
	switch (cmd)
    {
		/* Make sure that no pending write process */
		case CTRL_SYNC :
			res = RES_OK;
			break;

		/* Size in bytes for single sector */
		case GET_SECTOR_SIZE:
			*(WORD *)buff = SD_BLOCK_SIZE;
			res = RES_OK;
			break;

		/* Get number of sectors on the disk (DWORD) */
		case GET_SECTOR_COUNT :
			BSP_SD_GetCardInfo(&CardInfo);
			*(DWORD *)buff = CardInfo.LogBlockNbr;
			res = RES_OK;
			break;

		/* Get erase block size in unit of sector (DWORD) */
		case GET_BLOCK_SIZE :
			*(DWORD*)buff = CardInfo.LogBlockSize;
			break;

		default:
			res = RES_PARERR;
	}
  
	return res;
}

DRESULT TM_FATFS_SD_SDIO_disk_read(BYTE *buff, DWORD sector, UINT count)
{
    uint32_t timeout = 100000;
	if (BSP_SD_ReadBlocks((uint32_t *)buff, sector, count, 1000) != MSD_OK)
		return RES_ERROR;

	while (BSP_SD_GetStatus() != MSD_OK)
        if (timeout-- == 0)
            return RES_ERROR;

	return RES_OK;
}

DRESULT TM_FATFS_SD_SDIO_disk_write(const BYTE *buff, DWORD sector, UINT count)
{
    uint32_t timeout = 100000;

	if (BSP_SD_WriteBlocks((uint32_t *)buff, sector, count, 1000) != MSD_OK)
		return RES_ERROR;

	while (BSP_SD_GetStatus() != MSD_OK)
        if (timeout-- == 0)
            return RES_ERROR;

	return RES_OK;
}
