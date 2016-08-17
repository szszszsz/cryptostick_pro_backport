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
#include "hw_config.h"
#include "usb_scsi.h"
#include "mass_mal.h"
#include "usb_bot.h"
#include "usb_regs.h"
#include "memory.h"
#include "nand_if.h"
#include "platform_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern uint8_t Bulk_Data_Buff[BULK_MAX_PACKET_SIZE];  /* data buffer*/
extern uint8_t Bot_State;
extern Bulk_Only_CBW CBW;
extern Bulk_Only_CSW CSW;
extern uint32_t Mass_Memory_Size;
extern uint32_t Mass_Block_Size;
extern uint32_t Mass_Block_Count;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : SCSI_Inquiry_Cmd
* Description    : SCSI Inquiry Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Inquiry_Cmd(uint8_t lun)
{
  uint8_t* Inquiry_Data = Standard_Inquiry_Data_Ramdisk;		// default
  uint16_t Inquiry_Data_Length;

  if (CBW.CB[1] & 0x01)/*Evpd is set*/
  {
    Inquiry_Data        = Page00_Inquiry_Data;
    Inquiry_Data_Length = 5;
  }
  else
  {
		switch (nGlobalStickState)
		{
			case STICK_STATE_RAMDISK	 :
	      														Inquiry_Data = Standard_Inquiry_Data_Ramdisk;
																		break;
			case STICK_STATE_SD_DISK	 :
	      														Inquiry_Data = Standard_Inquiry_Data;
																		break;
			case STICK_STATE_FLASHDISK :
	      														Inquiry_Data = Standard_Inquiry_Data2;
																		break;
		}

    if (CBW.CB[4] <= STANDARD_INQUIRY_DATA_LEN)
		{
      Inquiry_Data_Length = CBW.CB[4];
		}
    else
		{
      Inquiry_Data_Length = STANDARD_INQUIRY_DATA_LEN;
		}

  }
  Transfer_Data_Request(Inquiry_Data, Inquiry_Data_Length);
}

/*******************************************************************************
* Function Name  : SCSI_ReadFormatCapacity_Cmd
* Description    : SCSI ReadFormatCapacity Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ReadFormatCapacity_Cmd(uint8_t lun)
{

  if (MAL_GetStatus() != 0 )
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
  ReadFormatCapacity_Data[4] = (uint8_t)(Mass_Block_Count >> 24);
  ReadFormatCapacity_Data[5] = (uint8_t)(Mass_Block_Count >> 16);
  ReadFormatCapacity_Data[6] = (uint8_t)(Mass_Block_Count >>  8);
  ReadFormatCapacity_Data[7] = (uint8_t)(Mass_Block_Count);

  ReadFormatCapacity_Data[9] = (uint8_t)(Mass_Block_Size >>  16);
  ReadFormatCapacity_Data[10] = (uint8_t)(Mass_Block_Size >>  8);
  ReadFormatCapacity_Data[11] = (uint8_t)(Mass_Block_Size);
  Transfer_Data_Request(ReadFormatCapacity_Data, READ_FORMAT_CAPACITY_DATA_LEN);
}

/*******************************************************************************
* Function Name  : SCSI_ReadCapacity10_Cmd
* Description    : SCSI ReadCapacity10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ReadCapacity10_Cmd(uint8_t lun)
{

  if (MAL_GetStatus())
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }

  ReadCapacity10_Data[0] = (uint8_t)(Mass_Block_Count - 1 >> 24);
  ReadCapacity10_Data[1] = (uint8_t)(Mass_Block_Count - 1 >> 16);
  ReadCapacity10_Data[2] = (uint8_t)(Mass_Block_Count - 1 >>  8);
  ReadCapacity10_Data[3] = (uint8_t)(Mass_Block_Count - 1);

  ReadCapacity10_Data[4] = (uint8_t)(Mass_Block_Size >>  24);
  ReadCapacity10_Data[5] = (uint8_t)(Mass_Block_Size >>  16);
  ReadCapacity10_Data[6] = (uint8_t)(Mass_Block_Size >>  8);
  ReadCapacity10_Data[7] = (uint8_t)(Mass_Block_Size);
  Transfer_Data_Request(ReadCapacity10_Data, READ_CAPACITY10_DATA_LEN);
}

/*******************************************************************************
* Function Name  : SCSI_ModeSense6_Cmd
* Description    : SCSI ModeSense6 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ModeSense6_Cmd (uint8_t lun)
{
  Transfer_Data_Request(Mode_Sense6_data, MODE_SENSE6_DATA_LEN);
}

/*******************************************************************************
* Function Name  : SCSI_ModeSense10_Cmd
* Description    : SCSI ModeSense10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ModeSense10_Cmd (uint8_t lun)
{
  Transfer_Data_Request(Mode_Sense10_data, MODE_SENSE10_DATA_LEN);
}

/*******************************************************************************
* Function Name  : SCSI_RequestSense_Cmd
* Description    : SCSI RequestSense Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_RequestSense_Cmd (uint8_t lun)
{
  uint8_t Request_Sense_data_Length;

  if (CBW.CB[4] <= REQUEST_SENSE_DATA_LEN)
  {
    Request_Sense_data_Length = CBW.CB[4];
  }
  else
  {
    Request_Sense_data_Length = REQUEST_SENSE_DATA_LEN;
  }
  Transfer_Data_Request(Scsi_Sense_Data, Request_Sense_data_Length);
}

/*******************************************************************************
* Function Name  : Set_Scsi_Sense_Data
* Description    : Set Scsi Sense Data routine.
* Input          : uint8_t Sens_Key
                   uint8_t Asc.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_Scsi_Sense_Data(uint8_t lun, uint8_t Sens_Key, uint8_t Asc)
{
  Scsi_Sense_Data[2] = Sens_Key;
  Scsi_Sense_Data[12] = Asc;
}

/*******************************************************************************
* Function Name  : SCSI_Start_Stop_Unit_Cmd
* Description    : SCSI Start_Stop_Unit Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Start_Stop_Unit_Cmd(uint8_t lun)
{
  Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}

/*******************************************************************************
* Function Name  : SCSI_Read10_Cmd
* Description    : SCSI Read10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Read10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{

  if (Bot_State == BOT_IDLE)
  {
    if (!(SCSI_Address_Management(CBW.bLUN, SCSI_READ10, LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((CBW.bmFlags & 0x80) != 0)
    {
      Bot_State = BOT_DATA_IN;
      Read_Memory(LBA , BlockNbr);
    }
    else
    {
      Bot_Abort(BOTH_DIR);
      Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    }
    return;
  }
  else if (Bot_State == BOT_DATA_IN)
  {
    Read_Memory(LBA , BlockNbr);
  }
}

/*******************************************************************************
* Function Name  : SCSI_Write10_Cmd
* Description    : SCSI Write10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Write10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{

  if (Bot_State == BOT_IDLE)
  {
    if (!(SCSI_Address_Management(CBW.bLUN, SCSI_WRITE10 , LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((CBW.bmFlags & 0x80) == 0)
    {
      Bot_State = BOT_DATA_OUT;
      SetEPRxStatus(ENDP2, EP_RX_VALID);
    }
    else
    {
      Bot_Abort(DIR_IN);
      Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    }
    return;
  }
  else if (Bot_State == BOT_DATA_OUT)
  {
    Write_Memory(LBA , BlockNbr);
  }
}

/*******************************************************************************
* Function Name  : SCSI_Verify10_Cmd
* Description    : SCSI Verify10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Verify10_Cmd(uint8_t lun)
{
  if ((CBW.dDataLength == 0) && !(CBW.CB[1] & BLKVFY))/* BLKVFY not set*/
  {
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
  else
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
}
/*******************************************************************************
* Function Name  : SCSI_Valid_Cmd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Valid_Cmd(uint8_t lun)
{
  if (CBW.dDataLength != 0)
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
  else
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}
/*******************************************************************************
* Function Name  : SCSI_Valid_Cmd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_TestUnitReady_Cmd(uint8_t lun)
{
  if (MAL_GetStatus())
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
  else
  {
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
}
/*******************************************************************************
* Function Name  : SCSI_Format_Cmd
* Description    : Format Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Format_Cmd(uint8_t lun)
{
  if (MAL_GetStatus())
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
/* RB not used
#ifdef USE_STM3210E_EVAL
  else
  {
    NAND_Format();
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
#endif
*/
}
/*******************************************************************************
* Function Name  : SCSI_Invalid_Cmd
* Description    : Invalid Commands routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Invalid_Cmd(uint8_t lun)
{
  if (CBW.dDataLength == 0)
  {
    Bot_Abort(DIR_IN);
  }
  else
  {
    if ((CBW.bmFlags & 0x80) != 0)
    {
      Bot_Abort(DIR_IN);
    }
    else
    {
      Bot_Abort(BOTH_DIR);
    }
  }
  Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
  Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
}

/*******************************************************************************
* Function Name  : SCSI_Address_Management
* Description    : Test the received address.
* Input          : uint8_t Cmd : the command can be SCSI_READ10 or SCSI_WRITE10.
* Output         : None.
* Return         : Read\Write status (bool).
*******************************************************************************/
bool SCSI_Address_Management(uint8_t lun , uint8_t Cmd , uint32_t LBA , uint32_t BlockNbr)
{

  if ((LBA + BlockNbr) > Mass_Block_Count)
  {
    if (Cmd == SCSI_WRITE10)
    {
      Bot_Abort(BOTH_DIR);
    }
    Bot_Abort(DIR_IN);
    Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    return (FALSE);
  }


  if (CBW.dDataLength != BlockNbr * Mass_Block_Size)
  {
    if (Cmd == SCSI_WRITE10)
    {
      Bot_Abort(BOTH_DIR);
    }
    else
    {
      Bot_Abort(DIR_IN);
    }
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
    return (FALSE);
  }
  return (TRUE);
}

