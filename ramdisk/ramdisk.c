/*
* Author: Copyright (C) Rudolf Boeddeker
* Date: 2010-01-13
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

#include "ramdisk.h"
#include "AccessInterface.h"
#include "fat12.h"
#include "platform_config.h"
#include "string.h"

#define RAMDISK_START_IN_MEM                                                                       \
  ((uint32_t)(0x20000000 + 0x4000)) // 0xa000 Check build config and ram chip size
#define RAMDISK_SIZE ((uint32_t)0x1000)
#define RAMDISK_BLOCK_SIZE ((uint16_t)0x0200) /* 512 bytes per block */
#define RAMDISK_MAX_BLOCK ((uint16_t)(RAMDISK_SIZE / RAMDISK_BLOCK_SIZE))

char *RamdiskStart = (char *)RAMDISK_START_IN_MEM;

/*******************************************************************************

  RamdiskInit

*******************************************************************************/

uint16_t RamdiskInit(void) {
  RamdiskStart = (char *)RAMDISK_START_IN_MEM;

  CreateFilesystem(RamdiskStart);

  return (TRUE);
}

/*******************************************************************************

  RamdiskWrite

*******************************************************************************/

uint16_t RamdiskWrite(uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length) {
  char *WB = (char *)Writebuff;
  int i;
  int n;

  n = Transfer_Length;

  for (i = 0; i < n; i++) {
    RamdiskStart[Memory_Offset + i] = WB[i];
  }
  //		memcpy (&RamdiskStart[Memory_Offset],Writebuff,Transfer_Length);
  return (TRUE);
}

/*******************************************************************************

  RamdiskRead

*******************************************************************************/

uint16_t RamdiskRead(uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length) {
  char *WB = (char *)Readbuff;
  int i;
  int n;

  n = Transfer_Length;

  for (i = 0; i < n; i++) {
    WB[i] = RamdiskStart[Memory_Offset + i];
  }

  //	memcpy (Readbuff,&RamdiskStart[Memory_Offset],Transfer_Length);

  return (TRUE);
}
