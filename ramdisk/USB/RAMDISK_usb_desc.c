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
#include "RAMDISK_usb_desc.h"

const uint8_t RAMDISK_DeviceDescriptor[RAMDISK_SIZ_DEVICE_DESC] =
  {
    0x12,   /* bLength  */
    0x01,   /* bDescriptorType */
    0x00,   /* bcdUSB, version 2.00 */
    0x02,
    0x00,   /* bDeviceClass : each interface define the device class */
    0x00,   /* bDeviceSubClass */
    0x00,   /* bDeviceProtocol */
    0x40,   /* bMaxPacketSize0 0x40 = 64 */
    0x83,   /* idVendor     (0483) */
    0x04,
    0x21,   /* idProduct */
    0x57,
    0x00,   /* bcdDevice 2.00*/
    0x02,
    1,              /* index of string Manufacturer  */
    /**/
    2,              /* index of string descriptor of product*/
    /* */
    3,              /* */
    /* */
    /* */
    0x01    /*bNumConfigurations */
  };
const uint8_t RAMDISK_ConfigDescriptor[RAMDISK_SIZ_CONFIG_DESC] =
  {

    0x09,   /* bLength: Configuation Descriptor size */
    0x02,   /* bDescriptorType: Configuration */
    RAMDISK_SIZ_CONFIG_DESC,

    0x00,
    0x01,   /* bNumInterfaces: 1 interface */
    0x01,   /* bConfigurationValue: */
    /*      Configuration value */
    0x00,   /* iConfiguration: */
    /*      Index of string descriptor */
    /*      describing the configuration */
    0x80,   /* bmAttributes: */
    /*      bus powered */
    0x32,   /* MaxPower 100 mA */

    /******************** Descriptor of Mass Storage interface ********************/
    /* 09 */
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    /*      Interface descriptor type */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints*/
    0x08,   /* bInterfaceClass: MASS STORAGE Class */
    0x06,   /* bInterfaceSubClass : SCSI transparent*/
    0x50,   /* nInterfaceProtocol */
    4,          /* iInterface: */
    /* 18 */
    0x07,   /*Endpoint descriptor length = 7*/
    0x05,   /*Endpoint descriptor type */
    0x81,   /*Endpoint address (IN, address 1) */
    0x02,   /*Bulk endpoint type */
    0x40,   /*Maximum packet size (64 bytes) */
    0x00,
    0x00,   /*Polling interval in milliseconds */
    /* 25 */
    0x07,   /*Endpoint descriptor length = 7 */
    0x05,   /*Endpoint descriptor type */
    0x02,   /*Endpoint address (OUT, address 2) */
    0x02,   /*Bulk endpoint type */
    0x40,   /*Maximum packet size (64 bytes) */
    0x00,
    0x00     /*Polling interval in milliseconds*/
    /*32*/
  };

const uint8_t RAMDISK_StringLangID[RAMDISK_SIZ_STRING_LANGID] =
  {
    RAMDISK_SIZ_STRING_LANGID,
    0x03,
    0x09,
    0x04
  }
  ;      /* LangID = 0x0409: U.S. English */

const uint8_t RAMDISK_StringVendor[RAMDISK_SIZ_STRING_VENDOR] =
  {
    RAMDISK_SIZ_STRING_VENDOR, /* Size of manufaturer string */
    0x03,           /* bDescriptorType = String descriptor */
    /* Manufacturer: "Cryptostick V1.000" */
    'C', 0, 'r', 0, 'y', 0, 'p', 0, 't', 0, 'o', 0, 's', 0, 't', 0,
    'i', 0, 'c', 0, 'k', 0, ' ', 0, 'V', 0, '1', 0, '.', 0, '0', 0,
    '0', 0, '0', 0
  };

const uint8_t RAMDISK_StringProduct[RAMDISK_SIZ_STRING_PRODUCT] =
  {
    RAMDISK_SIZ_STRING_PRODUCT,
    0x03,
    /* Product name: "RAMDISK" */
    'R', 0, 'A', 0, 'M', 0, 'D', 0, 'I', 0, 'S', 0, 'K', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0

  };

uint8_t RAMDISK_StringSerial[RAMDISK_SIZ_STRING_SERIAL] =
{
    RAMDISK_SIZ_STRING_SERIAL,
    0x03,
/* Serial number  "            " - (3 * 8 Byte defined in sourcecode)* 2 + 2 = 50 byte  */
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0, ' ', 0, ' ', 0
};

const uint8_t RAMDISK_StringInterface[RAMDISK_SIZ_STRING_INTERFACE] =
  {
    RAMDISK_SIZ_STRING_INTERFACE,
    0x03,
    /* Interface 0: "RAMDISK" */
    'R', 0, 'A', 0, 'M', 0, 'D', 0, 'I', 0, 'S', 0, 'K', 0
  };


