/*
* Author: Copyright (C) STMicroelectronics	 			Date:	04/27/2009
*												 MCD
*Application Team			Version V3.0.1
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_BOT_H
#define __USB_BOT_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Bulk-only Command Block Wrapper */

typedef struct _Bulk_Only_CBW {
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t bmFlags;
  uint8_t bLUN;
  uint8_t bCBLength;
  uint8_t CB[16];
} Bulk_Only_CBW;

/* Bulk-only Command Status Wrapper */
typedef struct _Bulk_Only_CSW {
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t bStatus;
} Bulk_Only_CSW;
/* Exported constants --------------------------------------------------------*/

/*****************************************************************************/
/*********************** Bulk-Only Transfer State machine ********************/
/*****************************************************************************/
#define BOT_IDLE 0         /* Idle state */
#define BOT_DATA_OUT 1     /* Data Out state */
#define BOT_DATA_IN 2      /* Data In state */
#define BOT_DATA_IN_LAST 3 /* Last Data In Last */
#define BOT_CSW_Send 4     /* Command Status Wrapper */
#define BOT_ERROR 5        /* error state */

#define BOT_CBW_SIGNATURE 0x43425355
#define BOT_CSW_SIGNATURE 0x53425355
#define BOT_CBW_PACKET_LENGTH 31

#define CSW_DATA_LENGTH 0x000D

/* CSW Status Definitions */
#define CSW_CMD_PASSED 0x00
#define CSW_CMD_FAILED 0x01
#define CSW_PHASE_ERROR 0x02

#define SEND_CSW_DISABLE 0
#define SEND_CSW_ENABLE 1

#define DIR_IN 0
#define DIR_OUT 1
#define BOTH_DIR 2

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Mass_Storage_In(void);
void Mass_Storage_Out(void);
void CBW_Decode(void);
void Transfer_Data_Request(uint8_t *Data_Pointer, uint16_t Data_Len);
void Set_CSW(uint8_t CSW_Status, uint8_t Send_Permission);
void Bot_Abort(uint8_t Direction);

#endif /* __USB_BOT_H */
