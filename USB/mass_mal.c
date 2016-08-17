/*
* Author: Copyright (C) Rudolf Boeddeker 					Date: 2010-01-13
*												STMicroelectronics	 			
*												MCD Application Team			Date:	04/27/2009
*
* This file is part of GPF Crypto Stick.
*
* GPF Crypto Stick is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* GPF Crypto Stick is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with GPF Crypto Stick. If not, see <http://www.gnu.org/licenses/>.
*/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#ifdef USB_4BIT_SD_CARD_INTERFACE
#include "sdcard.h"
#else
#include "msd.h"
#endif
#include "fsmc_nand.h"
#include "nand_if.h"
#include "mass_mal.h"
#include "ramdisk.h"
#include "aes.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size;
uint32_t Mass_Block_Size;
uint32_t Mass_Block_Count;
__IO uint32_t Status = 0;

#ifdef USB_4BIT_SD_CARD_INTERFACE
SD_CardInfo SDCardInfo;
#endif

extern void GPIO_Configuration(void);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

uint16_t MAL_Init(void)
{
  uint16_t status = MAL_OK;


	switch (nGlobalStickState)
	{
		case STICK_STATE_RAMDISK	 :
																	RamdiskInit ();
																	break;

		case STICK_STATE_SD_DISK	 :
#ifdef USB_4BIT_SD_CARD_INTERFACE
														      Status = SD_Init();
														
																	if (SD_OK != Status)
																	{
														      	Status = SD_Init();				
																	} 
														
																	if (SD_OK != Status)
																	{
														      	Status = SD_Init();				
																	} 

														      Status = SD_GetCardInfo (&SDCardInfo);
																	Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
														      Status = SD_EnableWideBusOperation (SDIO_BusWide_4b);
														      Status = SD_SetDeviceMode (SD_DMA_MODE);
#else
														      MSD_Init();
#endif																	
																	break;

																	

		case STICK_STATE_FLASHDISK :
//      														status = NAND_Init();
																	break;

		case STICK_STATE_SMARTCARD :
		case STICK_STATE_COMPOSITE :	
    default:
														      return MAL_FAIL;
	}

  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write(uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
	switch (nGlobalStickState)
	{
		case STICK_STATE_RAMDISK	 :
																	RamdiskWrite (Memory_Offset, Writebuff, Transfer_Length);
																	break;
		case STICK_STATE_SD_DISK	 :
#ifdef CRYPTO_AES
																	 
#endif
#ifdef USB_4BIT_SD_CARD_INTERFACE
														      Status = SD_WriteBlock(Memory_Offset, Writebuff, Transfer_Length);
														      if ( Status != SD_OK )
														      {
														        return MAL_FAIL;
														      }      
#else
														      MSD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
#endif
																	break;
		case STICK_STATE_FLASHDISK :
//														      NAND_Write(Memory_Offset, Writebuff, Transfer_Length);
																	break;
		case STICK_STATE_SMARTCARD :
		case STICK_STATE_COMPOSITE :	
    default:
      														return MAL_FAIL;
	}

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{

	switch (nGlobalStickState)
	{
		case STICK_STATE_RAMDISK	 :
																	RamdiskRead (Memory_Offset, Readbuff, Transfer_Length);
																	break;
		case STICK_STATE_SD_DISK	 :
#ifdef USB_4BIT_SD_CARD_INTERFACE
														      Status = SD_ReadBlock(Memory_Offset, Readbuff, Transfer_Length);
														      if ( Status != SD_OK )
														      {
														        return MAL_FAIL;
														      }
#else
															    MSD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);
#endif
#ifdef CRYPTO_AES 
#endif
																	break;
		case STICK_STATE_FLASHDISK :
//      														NAND_Read(Memory_Offset, Readbuff, Transfer_Length);
																	break;
    default:
      														return MAL_FAIL;
	}

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (void)
{
#ifdef USB_4BIT_SD_CARD_INTERFACE
  NAND_IDTypeDef NAND_ID;
  uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;
//  static uint32_t SD_CardInitOK = FALSE;
#else
  uint32_t temp_block_mul = 0;
  sMSD_CSD MSD_csd;
  uint32_t DeviceSizeMul = 0;
#endif



	switch (nGlobalStickState)
	{
		case STICK_STATE_RAMDISK	 :
														      Mass_Block_Count = RAMDISK_MAX_BLOCK;
														      Mass_Block_Size  = RAMDISK_BLOCK_SIZE;
														      Mass_Memory_Size = (Mass_Block_Count * Mass_Block_Size);
														      return MAL_OK;

		case STICK_STATE_SD_DISK	 :
#ifdef USB_4BIT_SD_CARD_INTERFACE
																	if (1)//(FALSE == SD_CardInitOK) // to do : the  flag did not work 
																	{
																    if (SD_Init() == SD_OK)
																    {
																      SD_GetCardInfo(&SDCardInfo);
																      SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
																      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);
																
																      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
																      {
																        Mass_Block_Count = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
																      }
																      else
																      {
																        NumberOfBlocks   = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
																        Mass_Block_Count = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
																      }
																      Mass_Block_Size  = 512;
																
																      Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16)); 
																      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
																      if ( Status != SD_OK )
																      {
																        return MAL_FAIL;
																      }
																       
																      Status = SD_SetDeviceMode(SD_DMA_MODE);         
																      if ( Status != SD_OK )
																      {
																        return MAL_FAIL;
																      } 
//																			SD_CardInitOK = TRUE;
															      }
																	}
																	else
																	{
															      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);
															
															      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
															      {
															        Mass_Block_Count = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
															      }
															      else
															      {
															        NumberOfBlocks   = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
															        Mass_Block_Count = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
															      }
															      Mass_Block_Size  = 512;
																	}
#else
															    MSD_GetCSDRegister(&MSD_csd);
															    DeviceSizeMul = MSD_csd.DeviceSizeMul + 2;
															    temp_block_mul = (1 << MSD_csd.RdBlockLen)/ 512;
															    Mass_Block_Count = ((MSD_csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
															    Mass_Block_Size  = 512;
															    Mass_Memory_Size = (Mass_Block_Count * Mass_Block_Size);
#endif
																		
															    Mass_Memory_Size = Mass_Block_Count * Mass_Block_Size;
															    return MAL_OK;

		case STICK_STATE_FLASHDISK :
//    															FSMC_NAND_ReadID(&NAND_ID);
/*
															    if (NAND_ID.Device_ID != 0)
															    {
															      // only one zone is used 
															      Mass_Block_Count = NAND_ZONE_SIZE * NAND_BLOCK_SIZE * NAND_MAX_ZONE ;
															      Mass_Block_Size  = NAND_PAGE_SIZE;
															      Mass_Memory_Size = (Mass_Block_Count * Mass_Block_Size);
															      return MAL_OK;
															    }
*/
																	break;

		case STICK_STATE_SMARTCARD :
																	break;
		case STICK_STATE_COMPOSITE :	
																	break;
    default:
																	break;
	}

  return MAL_FAIL;
}

