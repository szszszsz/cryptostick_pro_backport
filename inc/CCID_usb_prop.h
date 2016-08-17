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
#ifndef __CCID_usb_prop_H
#define __CCID_usb_prop_H
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
#define USB_CCID_Storage_GetConfiguration NOP_Process
//#define USB_CCID_Storage_SetConfiguration          NOP_Process			 //
#define USB_CCID_Storage_GetInterface NOP_Process
#define USB_CCID_Storage_SetInterface NOP_Process
#define USB_CCID_Storage_GetStatus NOP_Process
#define USB_CCID_Storage_ClearFeature NOP_Process //
#define USB_CCID_Storage_SetEndPointFeature NOP_Process
#define USB_CCID_Storage_SetDeviceFeature NOP_Process
#define USB_CCID_Storage_SetDeviceAddress NOP_Process //

/* CCID Storage Requests*/
/*
#define GET_MAX_LUN                0xFE
#define CCID_STORAGE_RESET         0xFF
#define LUN_DATA_LENGTH            1
*/

#define USB_MAX_PACKET_SIZE 0x40

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USB_CCID_init(void);
void USB_CCID_Reset(void);
void USB_CCID_Storage_SetConfiguration(void);
void USB_CCID_Storage_ClearFeature(void);
void USB_CCID_Storage_SetDeviceAddress(void);
void USB_CCID_Status_In(void);
void USB_CCID_Status_Out(void);
RESULT USB_CCID_Data_Setup(uint8_t);
RESULT USB_CCID_NoData_Setup(uint8_t);
RESULT USB_CCID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *USB_CCID_GetDeviceDescriptor(uint16_t);
uint8_t *USB_CCID_GetConfigDescriptor(uint16_t);
uint8_t *USB_CCID_GetStringDescriptor(uint16_t);
uint8_t *USB_CCID_Get_Max_Lun(uint16_t Length);

#endif /* __CCID_usb_prop_H */
