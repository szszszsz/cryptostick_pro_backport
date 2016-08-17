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
#include "usb_bot.h"
#include "usb_istr.h"
#include "usb_lib.h"

#include "CCID_usb.h"
#include "platform_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    : EP1 IN Callback Routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void) {
  switch (nGlobalStickState) {
  case STICK_STATE_RAMDISK:
    Mass_Storage_In();
    break;
  case STICK_STATE_SD_DISK:
    Mass_Storage_In();
    break;

  case STICK_STATE_SMARTCARD:
    break;

  case STICK_STATE_COMPOSITE:
    break;
  }
}

/*******************************************************************************
* Function Name  : EP2_OUT_Callback.
* Description    : EP2 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP2_OUT_Callback(void) {
  switch (nGlobalStickState) {
  case STICK_STATE_RAMDISK:
    Mass_Storage_Out();
    break;

  case STICK_STATE_SD_DISK:
    Mass_Storage_Out();
    break;

  case STICK_STATE_SMARTCARD:
    CCID_BulkOutMessage();
    break;

  case STICK_STATE_COMPOSITE:
    break;
  }
}

/*******************************************************************************
* Function Name  : EP2_IN_Callback.
* Description    : EP2 IN Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP2_IN_Callback(void) {
  switch (nGlobalStickState) {
  case STICK_STATE_RAMDISK:
    break;

  case STICK_STATE_SD_DISK:
    break;

  case STICK_STATE_SMARTCARD:
    CCID_BulkInMessage();
    break;
  case STICK_STATE_COMPOSITE:
    break;
  }
}
