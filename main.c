/*
* Author: Copyright (C) Rudolf Boeddeker 					Date: 2010-01-13
*												STMicroelectronics
*Date:	04/27/2009
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

#include "AccessInterface.h"
#include "CCID_usb.h"
#include "hw_config.h"
#include "platform_config.h"
#include "ramdisk.h"
#include "setjmp.h"
#include "smartcard.h"
#include "stm32f10x.h"
#include "usb_lib.h"
#include "usb_pwr.h"

int nGlobalStickState = STICK_STATE_SD_DISK;
int nFlagSendSMCardInserted = TRUE;

jmp_buf jmpRestartUSB; // reentrypoint for USB device change

void Test1(void);
void test2(void);

/*******************************************************************************

  Test2	- For pin toggle

*******************************************************************************/

void test2(void) {
  static int i = 0;
  i++;
  if (i & 1) {
    GPIO_ResetBits(GPIOB, GPIO_Pin_1); // org
  } else {
    GPIO_SetBits(GPIOB, GPIO_Pin_1); // org
  }
}

/*******************************************************************************

  Test1	- For pin toggle

*******************************************************************************/

void Test1(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  // enable port clock
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  // set pin modes
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/*******************************************************************************

  main							R8 Stick - RC development board

*******************************************************************************/

int main(void) {
  // SD_Test ();
  //	SDIO_DataLineTest ();

  //	USB_SetDeviceConfiguration(STICK_STATE_RAMDISK);
  //	USB_SetDeviceConfiguration(STICK_STATE_SD_DISK);
  USB_SetDeviceConfiguration(STICK_STATE_SMARTCARD);

  setjmp(jmpRestartUSB); // entrypoint for the changed USB device

  Set_System();

  SysTick_Config(720000); // set systemtick to 10 ms - for delay ()

  /* Setup befor USB startup */
  switch (nGlobalStickState) {
  case STICK_STATE_RAMDISK:
    RamdiskInit();
    break;
  case STICK_STATE_SD_DISK:
    break;
  case STICK_STATE_SMARTCARD:
    SmartCardInitInterface();
    break;
  case STICK_STATE_COMPOSITE:
    break;
  }

  USB_Start();

  /* Endless loop after USB startup  */
  while (1) {
    switch (nGlobalStickState) {
    case STICK_STATE_RAMDISK:
      RamDiskUserInterface(); // comes not back
      break;
    case STICK_STATE_SD_DISK:
      break;
    case STICK_STATE_SMARTCARD:
      CCID_CheckUsbCommunication();
      if (TRUE == nFlagSendSMCardInserted) {
        CCID_SendCardDetect(); // Send card detect to host
        nFlagSendSMCardInserted = FALSE;
      }
      break;
    case STICK_STATE_COMPOSITE:
      break;
    }
  }
}

#ifdef USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
