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
#include "CCID_usb_desc.h"

#define WBVAL(x) (x & 0xff), ((x >> 8) & 0xff)

const uint8_t CCID_DeviceDescriptor[CCID_SIZ_DEVICE_DESC] = {
    0x12,          /* DeviceDescriptor bLength  */
    0x01,          /* bDescriptorType */
    WBVAL(0x0110), /* bcdUSB, version 1.10 */
    0x00,          /* bDeviceClass : each interface define the device class */
    0x00,          /* bDeviceSubClass */
    0x00,          /* bDeviceProtocol */
    0x40,          /* bMaxPacketSize0 0x08 - 0x40 = 64 */

    WBVAL(0x20A0), /* VID for CCID driver idVendor */
    WBVAL(0x4107), /* PID for CCID driver idProduct */
                   //  	WBVAL(0x0800),  /* Test VID for MS CCID driver idVendor */
                   //  	WBVAL(0x0006),  /* Test PID for MS CCID driver idProduct */
                   //  	WBVAL(0x08e6),  /* 0x08e6 = idVendor  germalto - for testing */
    //  	WBVAL(0x3437),  /* 0x3437 = idProduct germalto usb-sl reader  - for testing */

    WBVAL(0x0100), /* bcdDevice version 1.00   */

    1,   /* index of string Manufacturer  */
    2,   /* index of string descriptor of product*/
    0,   // 3, RB            /* index of serial number */
    0x01 /*bNumConfigurations */
};

/*****************************************************************************/

#define USB_CONFIG_BUS_POWERED 0x80
#define USB_CONFIG_REMOTE_WAKEUP 0x20

const uint8_t
    CCID_ConfigDescriptor
        [CCID_SIZ_CONFIG_DESC] =
            {
                /* Configuration 1 */
                0x09,                              /* bLength ConfigDescriptor */
                0x02,                              /* bDescriptorType */
                WBVAL(                             /* wTotalLength */
                      1 * 0x09 + 1 * 0x09 + 0x36 + /* 0x36 =  CCID class descriptor size */
                      3 * 0x07),
                0x01,                   /* bNumInterfaces */
                0x01,                   /* bConfigurationValue */
                0x00,                   /* iConfiguration CCID = 6 ???? */
                USB_CONFIG_BUS_POWERED, /* bmAttributes */
                100 / 2,                /* MaxPower 200 mA */

                // Interface 0 descriptor (Interface 0 = Smart Card Reader)
                0x09, /* bLength */
                0x04, /* bDescriptorType */
                0x00, /* bInterfaceNumber */
                0x00, /* bAlternateSetting */
                0x03, /* bNumEndpoints = 3 */
                0x0B, /* bInterfaceClass = CCID */
                0x00, /* bInterfaceSubClass */
                0x00, /* bInterfaceProtocol */
                0x00, /* 0x04,  = 0 ??? iInterface Index of string descriptor (befor 0x64)*/

                // CCID class descriptor
                0x36, // bLength: CCID Descriptor size
                0x21, // bDescriptorType: HID To be updated with CCID specific number
                0x10, // bcdHID(LSB): CCID Class Spec release number (1.10)
                0x01, // bcdHID(MSB)

                0x00, // bMaxSlotIndex
                0x02, // bVoltageSupport: 3v

                0x02, 0x00, 0x00, 0x00, // dwProtocols: supports T=1
                0x10, 0x0E, 0x00, 0x00, // dwDefaultClock: 3,6 Mhz (0x00000E10)
                0x10, 0x0E, 0x00, 0x00, // dwMaximumClock: 3,6 Mhz (0x00000E10)
                0x00,                   // bNumClockSupported => only default clock

                0xCD, 0x25, 0x00, 0x00, // dwDataRate:      9677 bps (0x000025CD)
                0xA1, 0xC5, 0x01, 0x00, // dwMaxDataRate: 116129 bps (0x0001C5A1)
                0x00,                   // bNumDataRatesSupported => no manual setting

                0xFE, 0x00, 0x00, 0x00, /* dwMaxIFSD: 254 */
                0x00, 0x00, 0x00, 0x00, /* dwSynchProtocols  */
                0x00, 0x00, 0x00, 0x00, /* dwMechanical: no special characteristics */

                //	0x3E,0x00,0x02,0x00,							// instand pc
                //reset on XP :-)
                0xBA, 0x04, 0x01, 0x00, // 000104BAh
                // 00000002h Automatic parameter configuration based on ATR data
                // 00000008h Automatic ICC voltage selection
                // 00000010h Automatic ICC clock frequency change
                // 00000020h Automatic baud rate change
                // 00000080h Automatic PPS
                // 00000400h Automatic IFSD exchange
                // 00010000h TPDU level exchanges with CCID

                //	0x24,0x00,0x00,0x00,   						/*
                //dwMaxCCIDMessageLength : Maximun block size + header*/
                0x0F, 0x01, 0x00, 0x00, /* dwMaxCCIDMessageLength : Maximun block size + header*/
                                        /* 261 + 10   */

                0x00,       /* bClassGetResponse*/
                0x00,       /* bClassEnvelope */
                0x00, 0x00, /* wLcdLayout */
                0x00,       /* bPINSupport : no PIN verif and modif  */
                0x01,       /* bMaxCCIDBusySlots	*/

                // Endpoint 1 descriptor (Interrupt in SCR)
                0x07, /* bLength */
                0x05, // bDescriptorType: Endpoint descriptor type
                0x81, // bEndpointAddress: Endpoint  1 IN
                0x03, // bmAttributes: Interrupt endpoint
                0x40, // 0x08,	zur sicherheit 0x40  // wMaxPacketSize(LSB): 8 char max (0x0008)
                0x00, // wMaxPacketSize (MSB)
                0x18, // bInterval: Polling Interval (24 ms = 0x18)

                // Endpoint 2 descriptor (Bulk out SCR)
                0x07, /* bLength */
                0x05, // bDescriptorType: Endpoint descriptor type
                0x02, // bEndpointAddress: Endpoint  2 OUT
                0x02, // bmAttributes: Bulk endpoint
                0x40, // wMaxPacketSize(LSB): 64 char max (0x0040)
                0x00, // wMaxPacketSize (MSB)
                0x00, // bInterval: ignored

                // Endpoint 3 descriptor (Bulk in SCR)
                0x07, /* bLength */
                0x05, // bDescriptorType: Endpoint descriptor type
                0x82, // RB to avoid doublebuffering ?	0x82,// bEndpointAddress: Endpoint  2 IN
                0x02, // bmAttributes: Bulk endpoint
                0x40, // wMaxPacketSize(LSB): 64 char max (0x0040)
                0x00, // wMaxPacketSize (MSB)
                0x00, // bInterval: ignored

                /* Terminator */
                0 /* bLength */
};

/*****************************************************************************/

const uint8_t CCID_StringLangID[CCID_SIZ_STRING_LANGID] = {
    CCID_SIZ_STRING_LANGID, 0x03, 0x09, 0x04}; /* LangID = 0x0409: U.S. English */

const uint8_t CCID_StringVendor[CCID_SIZ_STRING_VENDOR] = {
    CCID_SIZ_STRING_VENDOR, /* Size of manufaturer string */
    0x03,                   /* bDescriptorType = String descriptor */
    /* Manufacturer: "German Privacy Foundation" */
    'G', 0, 'e', 0, 'r', 0, 'm', 0, 'a', 0, 'n', 0, ' ', 0, 'P', 0, 'r', 0, 'i', 0, 'v', 0, 'a', 0,
    'c', 0, 'y', 0, ' ', 0, 'F', 0, 'o', 0, 'u', 0, 'n', 0, 'd', 0, 'a', 0, 't', 0, 'i', 0, 'o', 0,
    'n', 0

};

const uint8_t CCID_StringProduct[CCID_SIZ_STRING_PRODUCT] = {
    CCID_SIZ_STRING_PRODUCT, 0x03,
    /* Product name: "Crypto Stick v1.2" */
    'C', 0, 'r', 0, 'y', 0, 'p', 0, 't', 0, 'o', 0, ' ', 0, 'S', 0, 't', 0, 'i', 0, 'c', 0, 'k', 0,
    ' ', 0, 'v', 0, '1', 0, '.', 0, '2', 0

};

uint8_t CCID_StringSerial[CCID_SIZ_STRING_SERIAL] = {
    CCID_SIZ_STRING_SERIAL, 0x03,
    /* Serial number  "            " - (3 * 8 Byte defined in sourcecode)* 2 + 2 = 50 byte  */
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0};

const uint8_t CCID_StringInterface[CCID_SIZ_STRING_INTERFACE] = {CCID_SIZ_STRING_INTERFACE, 0x03,
                                                                 /* Interface 0: "CS SmCd" */
                                                                 'C', 0, 'S', 0, ' ', 0, 'S', 0,
                                                                 'm', 0, 'C', 0, 'd', 0};
