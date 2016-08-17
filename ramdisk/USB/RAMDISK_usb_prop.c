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
#include "RAMDISK_usb_prop.h"
#include "RAMDISK_usb_desc.h"
#include "hw_config.h"
#include "mass_mal.h"
#include "memory.h"
#include "usb_bot.h"
#include "usb_lib.h"
#include "usb_pwr.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t RAMDISK_Max_Lun = 0;

DEVICE_INFO RAMDISK_Device_Info;

DEVICE RAMDISK_Device_Table = {EP_NUM, 1};

DEVICE_PROP RAMDISK_Device_Property = {
    RAMDISK_init,
    RAMDISK_Reset,
    RAMDISK_Status_In,
    RAMDISK_Status_Out,
    RAMDISK_Data_Setup,
    RAMDISK_NoData_Setup,
    RAMDISK_Get_Interface_Setting,
    RAMDISK_GetDeviceDescriptor,
    RAMDISK_GetConfigDescriptor,
    RAMDISK_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
};

USER_STANDARD_REQUESTS RAMDISK_User_Standard_Requests = {
    RAMDISK_Storage_GetConfiguration,   RAMDISK_Storage_SetConfiguration,
    RAMDISK_Storage_GetInterface,       RAMDISK_Storage_SetInterface,
    RAMDISK_Storage_GetStatus,          RAMDISK_Storage_ClearFeature,
    RAMDISK_Storage_SetEndPointFeature, RAMDISK_Storage_SetDeviceFeature,
    RAMDISK_Storage_SetDeviceAddress};

ONE_DESCRIPTOR RAMDISK_Device_Descriptor = {(uint8_t *)RAMDISK_DeviceDescriptor,
                                            RAMDISK_SIZ_DEVICE_DESC};

ONE_DESCRIPTOR RAMDISK_Config_Descriptor = {(uint8_t *)RAMDISK_ConfigDescriptor,
                                            RAMDISK_SIZ_CONFIG_DESC};

ONE_DESCRIPTOR RAMDISK_String_Descriptor[5] = {
    {(uint8_t *)RAMDISK_StringLangID, RAMDISK_SIZ_STRING_LANGID},
    {(uint8_t *)RAMDISK_StringVendor, RAMDISK_SIZ_STRING_VENDOR},
    {(uint8_t *)RAMDISK_StringProduct, RAMDISK_SIZ_STRING_PRODUCT},
    {(uint8_t *)RAMDISK_StringSerial, RAMDISK_SIZ_STRING_SERIAL},
    {(uint8_t *)RAMDISK_StringInterface, RAMDISK_SIZ_STRING_INTERFACE},
};

/*************************************************************************/
/*
DEVICE                 *Device_Table           = &RAMDISK_Device_Table;
DEVICE_PROP            *Device_Property        = &RAMDISK_Device_Property;
USER_STANDARD_REQUESTS *User_Standard_Requests = &RAMDISK_User_Standard_Requests;
ONE_DESCRIPTOR         *Device_Descriptor      = &RAMDISK_Device_Descriptor;
ONE_DESCRIPTOR         *Config_Descriptor      = &RAMDISK_Config_Descriptor;
ONE_DESCRIPTOR         *String_Descriptor[5]   = {
                                                                                                                                                                                                   &RAMDISK_String_Descriptor[0],
                                                                                                                                                                                                   &RAMDISK_String_Descriptor[1],
                                                                                                                                                                                                   &RAMDISK_String_Descriptor[2],
                                                                                                                                                                                                   &RAMDISK_String_Descriptor[3],
                                                                                                                                                                                                   &RAMDISK_String_Descriptor[4],
                                                                                                                                                                                                 };
*/
/*************************************************************************/

/* Extern variables ----------------------------------------------------------*/
extern unsigned char Bot_State;
extern Bulk_Only_CBW CBW;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : RAMDISK_init
* Description    : Mass Storage init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_init() {
  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum();

  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* USB interrupts initialization */
  /* clear pending interrupts */
  _SetISTR(0);
  wInterrupt_Mask = IMR_MSK;
  /* set interrupts mask */
  _SetCNTR(wInterrupt_Mask);

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : RAMDISK_Reset
* Description    : Mass Storage reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Reset() {
  /* Set the device as not configured */
  Device_Info->Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = RAMDISK_ConfigDescriptor[7];

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPRxCount(ENDP0, Device_Property->MaxPacketSize);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_BULK);
  SetEPRxAddr(ENDP2, ENDP2_RXADDR);
  SetEPRxCount(ENDP2, Device_Property->MaxPacketSize);
  SetEPRxStatus(ENDP2, EP_RX_VALID);
  SetEPTxStatus(ENDP2, EP_TX_DIS);

  SetEPRxCount(ENDP0, Device_Property->MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Set the device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;

  CBW.dSignature = BOT_CBW_SIGNATURE;
  Bot_State = BOT_IDLE;
}

/*******************************************************************************
* Function Name  : RAMDISK_Storage_SetConfiguration
* Description    : Handle the SetConfiguration request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Storage_SetConfiguration(void) {
  if (pInformation->Current_Configuration != 0) {
    /* Device configured */
    bDeviceState = CONFIGURED;

    ClearDTOG_TX(ENDP1);
    ClearDTOG_RX(ENDP2);
    Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
  }
}

/*******************************************************************************
* Function Name  : RAMDISK_Storage_ClearFeature
* Description    : Handle the ClearFeature request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Storage_ClearFeature(void) {
  /* when the host send a CBW with invalid signature or invalid length the two
     Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset     */
  if (CBW.dSignature != BOT_CBW_SIGNATURE)
    Bot_Abort(BOTH_DIR);
}

/*******************************************************************************
* Function Name  : RAMDISK_Storage_SetConfiguration.
* Description    : Udpade the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Storage_SetDeviceAddress(void) { bDeviceState = ADDRESSED; }
/*******************************************************************************
* Function Name  : RAMDISK_Status_In
* Description    : Mass Storage Status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Status_In(void) { return; }

/*******************************************************************************
* Function Name  : RAMDISK_Status_Out
* Description    : Mass Storage Status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void RAMDISK_Status_Out(void) { return; }

/*******************************************************************************
* Function Name  : RAMDISK_Data_Setup.
* Description    : Handle the data class specific requests..
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT RAMDISK_Data_Setup(uint8_t RequestNo) {
  uint8_t *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) && (RequestNo == GET_MAX_LUN) &&
      (pInformation->USBwValue == 0) && (pInformation->USBwIndex == 0) &&
      (pInformation->USBwLength == 0x01)) {
    CopyRoutine = RAMDISK_Get_Max_Lun;
  } else {
    return USB_UNSUPPORT;
  }

  if (CopyRoutine == NULL) {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);

  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : RAMDISK_NoData_Setup.
* Description    : Handle the no data class specific requests.
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT RAMDISK_NoData_Setup(uint8_t RequestNo) {
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) &&
      (RequestNo == RAMDISK_STORAGE_RESET) && (pInformation->USBwValue == 0) &&
      (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00)) {
    /* Initialize Endpoint 1 */
    ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    ClearDTOG_RX(ENDP2);

    /*intialise the CBW signature to enable the clear feature*/
    CBW.dSignature = BOT_CBW_SIGNATURE;
    Bot_State = BOT_IDLE;

    return USB_SUCCESS;
  }
  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : RAMDISK_Get_Interface_Setting
* Description    : Test the interface and the alternate setting according to the
*                  supported one.
* Input          : uint8_t Interface, uint8_t AlternateSetting.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT RAMDISK_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting) {
  if (AlternateSetting > 0) {
    return USB_UNSUPPORT; /* in this application we don't have AlternateSetting*/
  } else if (Interface > 0) {
    return USB_UNSUPPORT; /*in this application we have only 1 interfaces*/
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : RAMDISK_GetDeviceDescriptor
* Description    : Get the device descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *RAMDISK_GetDeviceDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, Device_Descriptor);
}

/*******************************************************************************
* Function Name  : RAMDISK_GetConfigDescriptor
* Description    : Get the configuration descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *RAMDISK_GetConfigDescriptor(uint16_t Length) {
  return Standard_GetDescriptorData(Length, Config_Descriptor);
}

/*******************************************************************************
* Function Name  : RAMDISK_GetStringDescriptor
* Description    : Get the string descriptors according to the needed index.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *RAMDISK_GetStringDescriptor(uint16_t Length) {
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 5) {
    return NULL;
  } else {
    return Standard_GetDescriptorData(Length, String_Descriptor[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : Get_Max_Lun
* Description    : Handle the Get Max Lun request.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *RAMDISK_Get_Max_Lun(uint16_t Length) {
  if (Length == 0) {
    pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
    return 0;
  } else {
    return ((uint8_t *)(&RAMDISK_Max_Lun));
  }
}
