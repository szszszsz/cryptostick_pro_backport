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
#include "platform_config.h"
#include "usb_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*  The number of current endpoint, it will be used to specify an endpoint */
uint8_t EPindex;
/*  The number of current device, it is an index to the Device_Table */
/* uint8_t	Device_no; */
/*  Points to the DEVICE_INFO structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_INFO *pInformation;
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */
DEVICE_PROP *pProperty;
/*  Temporary save the state of Rx & Tx status. */
/*  Whenever the Rx or Tx state is changed, its value is saved */
/*  in this variable first and will be set to the EPRB or EPRA */
/*  at the end of interrupt process */
uint16_t SaveState;
uint16_t wInterrupt_Mask;

DEVICE_INFO *Device_Info;

USER_STANDARD_REQUESTS *pUser_Standard_Requests;

/* Extern variables ----------------------------------------------------------*/
extern DEVICE_INFO RAMDISK_Device_Info;
extern DEVICE_PROP RAMDISK_Device_Property;
extern USER_STANDARD_REQUESTS RAMDISK_User_Standard_Requests;
extern ONE_DESCRIPTOR RAMDISK_Device_Descriptor;
extern ONE_DESCRIPTOR RAMDISK_Config_Descriptor;
extern ONE_DESCRIPTOR RAMDISK_String_Descriptor[5];

extern DEVICE_INFO MASS_Device_Info;
extern DEVICE_PROP MASS_Device_Property;
extern USER_STANDARD_REQUESTS MASS_User_Standard_Requests;
extern ONE_DESCRIPTOR MASS_Device_Descriptor;
extern ONE_DESCRIPTOR MASS_Config_Descriptor;
extern ONE_DESCRIPTOR MASS_String_Descriptor[5];

extern DEVICE_INFO CCID_Device_Info;
extern DEVICE_PROP CCID_Device_Property;
extern USER_STANDARD_REQUESTS CCID_User_Standard_Requests;
extern ONE_DESCRIPTOR CCID_Device_Descriptor;
extern ONE_DESCRIPTOR CCID_Config_Descriptor;
extern ONE_DESCRIPTOR CCID_String_Descriptor[5];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : USB_SetDeviceConfiguration
* Description    : USB set device configuration
* Input          : device configuration.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_SetDeviceConfiguration(int nGDC) {
  nGlobalStickState = nGDC;

  switch (nGlobalStickState) {
  case STICK_STATE_RAMDISK:
    Device_Info = &RAMDISK_Device_Info;
    Device_Property = &RAMDISK_Device_Property;
    User_Standard_Requests = &RAMDISK_User_Standard_Requests;
    Device_Descriptor = &RAMDISK_Device_Descriptor;
    Config_Descriptor = &RAMDISK_Config_Descriptor;
    String_Descriptor[0] = &RAMDISK_String_Descriptor[0];
    String_Descriptor[1] = &RAMDISK_String_Descriptor[1];
    String_Descriptor[2] = &RAMDISK_String_Descriptor[2];
    String_Descriptor[3] = &RAMDISK_String_Descriptor[3];
    String_Descriptor[4] = &RAMDISK_String_Descriptor[4];
    break;

  case STICK_STATE_SD_DISK:
    Device_Info = &MASS_Device_Info;
    Device_Property = &MASS_Device_Property;
    User_Standard_Requests = &MASS_User_Standard_Requests;
    Device_Descriptor = &MASS_Device_Descriptor;
    Config_Descriptor = &MASS_Config_Descriptor;
    String_Descriptor[0] = &MASS_String_Descriptor[0];
    String_Descriptor[1] = &MASS_String_Descriptor[1];
    String_Descriptor[2] = &MASS_String_Descriptor[2];
    String_Descriptor[3] = &MASS_String_Descriptor[3];
    String_Descriptor[4] = &MASS_String_Descriptor[4];
    break;

  case STICK_STATE_SMARTCARD:
    Device_Info = &CCID_Device_Info;
    Device_Property = &CCID_Device_Property;
    User_Standard_Requests = &CCID_User_Standard_Requests;
    Device_Descriptor = &CCID_Device_Descriptor;
    Config_Descriptor = &CCID_Config_Descriptor;
    String_Descriptor[0] = &CCID_String_Descriptor[0];
    String_Descriptor[1] = &CCID_String_Descriptor[1];
    String_Descriptor[2] = &CCID_String_Descriptor[2];
    String_Descriptor[3] = &CCID_String_Descriptor[3];
    String_Descriptor[4] = &CCID_String_Descriptor[4];

    break;

  case STICK_STATE_COMPOSITE:
    break;
  }
}

/*******************************************************************************
* Function Name  : USB_Init
* Description    : USB system initialization
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Init(void) {
  pInformation = Device_Info;
  pInformation->ControlState = 2;
  pProperty = Device_Property;
  pUser_Standard_Requests = User_Standard_Requests;

  /* Initialize devices one by one */
  pProperty->Init();
}
