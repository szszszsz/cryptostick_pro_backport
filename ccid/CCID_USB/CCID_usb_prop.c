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
#include "usb_lib.h"
#include "CCID_usb_conf.h"
#include "CCID_usb_desc.h"
#include "usb_pwr.h"
#include "usb_bot.h"
#include "hw_config.h"
#include "CCID_usb_prop.h"
#include "CCID_usb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

extern int nFlagSendSMCardInserted;
static uint32_t Max_Lun = 0;		// should not used

DEVICE_INFO CCID_Device_Info;

DEVICE CCID_Device_Table =
  {
    CCID_EP_NUM,
    1
  };


DEVICE_PROP CCID_Device_Property =
  {
    USB_CCID_init,
    USB_CCID_Reset,
    USB_CCID_Status_In,
    USB_CCID_Status_Out,
    USB_CCID_Data_Setup,
    USB_CCID_NoData_Setup,
    USB_CCID_Get_Interface_Setting,
    USB_CCID_GetDeviceDescriptor,	 //	
    USB_CCID_GetConfigDescriptor,
    USB_CCID_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };


USER_STANDARD_REQUESTS CCID_User_Standard_Requests =
  {
    USB_CCID_Storage_GetConfiguration,
    USB_CCID_Storage_SetConfiguration,
    USB_CCID_Storage_GetInterface,
    USB_CCID_Storage_SetInterface,
    USB_CCID_Storage_GetStatus,
    USB_CCID_Storage_ClearFeature,
    USB_CCID_Storage_SetEndPointFeature,
    USB_CCID_Storage_SetDeviceFeature,
    USB_CCID_Storage_SetDeviceAddress
  };

ONE_DESCRIPTOR CCID_Device_Descriptor =
  {
    (uint8_t*)CCID_DeviceDescriptor,
    CCID_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR CCID_Config_Descriptor =
  {
    (uint8_t*)CCID_ConfigDescriptor,
    CCID_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR CCID_String_Descriptor[5] =
  {
    {(uint8_t*)CCID_StringLangID,    CCID_SIZ_STRING_LANGID},
    {(uint8_t*)CCID_StringVendor,    CCID_SIZ_STRING_VENDOR},
    {(uint8_t*)CCID_StringProduct,   CCID_SIZ_STRING_PRODUCT},
    {(uint8_t*)CCID_StringSerial,    CCID_SIZ_STRING_SERIAL},
    {(uint8_t*)CCID_StringInterface, CCID_SIZ_STRING_INTERFACE},
  };


/* Extern variables ----------------------------------------------------------*/
extern unsigned char Bot_State;
extern Bulk_Only_CBW CBW;

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : CCID_init
* Description    : CCID init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_CCID_init(void)
{
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
* Function Name  : CCID_Reset
* Description    : CCID Storage reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_CCID_Reset(void)
{
/* Set the device as not configured */
  Device_Info->Current_Configuration = 0;

/* Current Feature initialization */
  pInformation->Current_Feature = CCID_ConfigDescriptor[7];

  SetBTABLE(BTABLE_ADDRESS);

/* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);

  SetEPTxStatus(ENDP0, EP_TX_NAK);
  SetEPRxAddr  (ENDP0, CCID_ENDP0_RXADDR);
  SetEPRxCount (ENDP0, Device_Property->MaxPacketSize);
  SetEPTxAddr  (ENDP0, CCID_ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxValid (ENDP0);

/* Initialize Endpoint 1 */
  SetEPType    (ENDP1, EP_INTERRUPT);
  SetEPTxAddr  (ENDP1, CCID_ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

/* Initialize Endpoint 2 */
  SetEPType    (ENDP2, EP_BULK);

  SetEPRxAddr  (ENDP2, CCID_ENDP2_RXADDR);
  SetEPRxCount (ENDP2, Device_Property->MaxPacketSize);
  SetEPRxStatus(ENDP2, EP_RX_VALID);

  SetEPTxAddr  (ENDP2, CCID_ENDP2_TXADDR);
  SetEPTxCount (ENDP2, Device_Property->MaxPacketSize);
  SetEPTxStatus(ENDP2, EP_TX_VALID);

/* */
  SetEPRxCount (ENDP0, Device_Property->MaxPacketSize);
  SetEPRxValid (ENDP0);

/* Set the device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;

  CBW.dSignature = BOT_CBW_SIGNATURE;
  Bot_State      = BOT_IDLE;

	nFlagSendSMCardInserted = TRUE;											// card is always inserted
}

/*******************************************************************************
* Function Name  : CCID_Storage_SetConfiguration
* Description    : Handle the SetConfiguration request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_CCID_Storage_SetConfiguration(void)
{
  if (pInformation->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;

    ClearDTOG_TX(ENDP1);
    ClearDTOG_RX(ENDP2);
    ClearDTOG_TX(ENDP2);
    Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
  }
}


/*******************************************************************************
* Function Name  : CCID_Storage_ClearFeature
* Description    : Handle the ClearFeature request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#ifdef NOT_USED
void USB_CCID_Storage_ClearFeature(void)
{
  /* when the host send a CBW with invalid signature or invalid length the two
     Endpoints (IN & OUT) shall stall until receiving a CCID Storage Reset     */
  if (CBW.dSignature != BOT_CBW_SIGNATURE)
    Bot_Abort(BOTH_DIR);
}
#endif
/*******************************************************************************
* Function Name  : CCID_Storage_SetConfiguration.
* Description    : Udpade the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#ifdef NOT_USED
void USB_CCID_Storage_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
#endif
/*******************************************************************************
* Function Name  : CCID_Status_In
* Description    : CCID Storage Status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_CCID_Status_In(void)
{
  return;
}

/*******************************************************************************
* Function Name  : CCID_Status_Out
* Description    : CCID Storage Status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_CCID_Status_Out(void)
{
  return;
}

/*******************************************************************************
* Function Name  : CCID_Data_Setup.
* Description    : Handle the data class specific requests..
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT USB_CCID_Data_Setup(uint8_t RequestNo)
{
 	return USB_UNSUPPORT; 		// not used ???
}

/*******************************************************************************
* Function Name  : CCID_NoData_Setup.
* Description    : Handle the no data class specific requests.
* Input          : RequestNo.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT USB_CCID_NoData_Setup(uint8_t RequestNo)
{
  return USB_UNSUPPORT;			
}

/*******************************************************************************
* Function Name  : CCID_Get_Interface_Setting
* Description    : Test the interface and the alternate setting according to the
*                  supported one.
* Input          : uint8_t Interface, uint8_t AlternateSetting.
* Output         : None.
* Return         : RESULT.
*******************************************************************************/
RESULT USB_CCID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;/* in this application we don't have AlternateSetting*/
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;/*in this application we have only 1 interfaces*/
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : CCID_GetDeviceDescriptor
* Description    : Get the device descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *USB_CCID_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, Device_Descriptor );
}

/*******************************************************************************
* Function Name  : CCID_GetConfigDescriptor
* Description    : Get the configuration descriptor.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *USB_CCID_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, Config_Descriptor );
}

/*******************************************************************************
* Function Name  : CCID_GetStringDescriptor
* Description    : Get the string descriptors according to the needed index.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint8_t *USB_CCID_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 5)
  {
    return NULL;
  }
  else
  {
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

uint8_t *USB_CCID_Get_Max_Lun(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1; 			// should not used LUN_DATA_LENGTH;
    return 0;
  }
  else
  {
    return((uint8_t*)(&Max_Lun));
  }
}

