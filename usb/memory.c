/*
* Author: Copyright (C) Rudolf Boeddeker 					Date: 2010-01-13
*												STMicroelectronics
*												MCD Application
*Team			Date:	04/27/2009
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

#include "memory.h"
#include "hw_config.h"
#include "mass_mal.h"
#include "usb_bot.h"
#include "usb_conf.h"
#include "usb_mem.h"
#include "usb_regs.h"
#include "usb_scsi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t Block_Read_count = 0;
__IO uint32_t Block_offset;
__IO uint32_t Counter = 0;
uint32_t Idx;
uint32_t Data_Buffer[BULK_MAX_PACKET_SIZE * 2]; /* 512 bytes*/
uint8_t TransferState = TXFR_IDLE;
/* Extern variables ----------------------------------------------------------*/
extern uint8_t Bulk_Data_Buff[BULK_MAX_PACKET_SIZE]; /* data buffer*/
extern uint16_t Data_Len;
extern uint8_t Bot_State;
extern Bulk_Only_CBW CBW;
extern Bulk_Only_CSW CSW;
extern uint32_t Mass_Memory_Size;
extern uint32_t Mass_Block_Size;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Read_Memory(uint32_t Memory_Offset, uint32_t Transfer_Length) {
  static uint32_t Offset, Length;

  if (TransferState == TXFR_IDLE) {
    Offset = Memory_Offset * Mass_Block_Size;
    Length = Transfer_Length * Mass_Block_Size;
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING) {
    if (!Block_Read_count) {
      MAL_Read(Offset, Data_Buffer, Mass_Block_Size);

      UserToPMABufferCopy((uint8_t *)Data_Buffer, ENDP1_TXADDR, BULK_MAX_PACKET_SIZE);
      Block_Read_count = Mass_Block_Size - BULK_MAX_PACKET_SIZE;
      Block_offset = BULK_MAX_PACKET_SIZE;
    } else {
      UserToPMABufferCopy((uint8_t *)Data_Buffer + Block_offset, ENDP1_TXADDR,
                          BULK_MAX_PACKET_SIZE);
      Block_Read_count -= BULK_MAX_PACKET_SIZE;
      Block_offset += BULK_MAX_PACKET_SIZE;
    }

    SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);
    SetEPTxStatus(ENDP1, EP_TX_VALID);
    Offset += BULK_MAX_PACKET_SIZE;
    Length -= BULK_MAX_PACKET_SIZE;

    CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;
  }
  if (Length == 0) {
    Block_Read_count = 0;
    Block_offset = 0;
    Offset = 0;
    Bot_State = BOT_DATA_IN_LAST;
    TransferState = TXFR_IDLE;
  }
}

/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Write_Memory(uint32_t Memory_Offset, uint32_t Transfer_Length) {

  static uint32_t W_Offset, W_Length;

  uint32_t temp = Counter + 64;

  if (TransferState == TXFR_IDLE) {
    W_Offset = Memory_Offset * Mass_Block_Size;
    W_Length = Transfer_Length * Mass_Block_Size;
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING) {

    for (Idx = 0; Counter < temp; Counter++) {
      *((uint8_t *)Data_Buffer + Counter) = Bulk_Data_Buff[Idx++];
    }

    W_Offset += Data_Len;
    W_Length -= Data_Len;

    if (!(W_Length % Mass_Block_Size)) {
      Counter = 0;
      MAL_Write(W_Offset - Mass_Block_Size, Data_Buffer, Mass_Block_Size);
    }

    CSW.dDataResidue -= Data_Len;
    SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/
  }

  if ((W_Length == 0) || (Bot_State == BOT_CSW_Send)) {
    Counter = 0;
    Set_CSW(CSW_CMD_PASSED, SEND_CSW_ENABLE);
    TransferState = TXFR_IDLE;
  }
}
