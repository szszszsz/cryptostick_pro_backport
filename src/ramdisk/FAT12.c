/*
* Author: Copyright (C) Rudolf Boeddeker 					Date: 2010-01-26
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

#include "fat12.h"
#include "string.h"

#define OSNAME "MSDOS5.0"          //  8 Byte long
#define FAT_TYPE "FAT12   "        //  8 Byte long
#define VOLUME_LABEL "CRYPTOSTICK" // 11 Byte long

#define DEFAULT_FILENAME "README  TXT" // 11 Byte long

#define DRIVE_SIZE_IN_BYTE 4096
#define BYTES_PER_SECTOR 512
#define DRIVE_SECTORS (DRIVE_SIZE_IN_BYTE / BYTES_PER_SECTOR)

/*******************************************************************************

  InitBootSector

*******************************************************************************/

void InitBootSector(char *pRamDisk) {
  tBootSectorType *pBS;

  pBS = (tBootSectorType *)pRamDisk;

  memset(pRamDisk, 0, BYTES_PER_SECTOR); // clear area

  memcpy((char *)pBS->OSName, OSNAME, 8);

  pBS->JmpBoot[0] = 0x0; // 0xEB;
  pBS->JmpBoot[1] = 0x0; // 0x3C;
  pBS->JmpBoot[2] = 0x0; // 0x90;

  pBS->JmpBoot[0] = 0xEB;
  pBS->JmpBoot[1] = 0x3C;
  pBS->JmpBoot[2] = 0x90;

  pBS->BytesPerSector = BYTES_PER_SECTOR;
  pBS->SectorsPerCluster = 1;
  pBS->ReservedSectors = 1;
  pBS->FATCount = 1;
  pBS->RootEntries = 16;
  pBS->Sectors = DRIVE_SECTORS;
  pBS->MediaType = 0xF8;
  pBS->FATSectors = 2;
  pBS->SectorsPerTrack = 1;
  pBS->Heads = 1;
  pBS->HiddenSectors = 0;
  pBS->TotSec32 = 0;
  pBS->Drive = 0xff; // No boot drive available
  pBS->BootSig = 0x29;
  pBS->VolumeID = 0;

  memcpy((char *)pBS->VolumeLabel, VOLUME_LABEL, 11);
  memcpy((char *)pBS->FatType, FAT_TYPE, 8);
}

/*******************************************************************************

  InitFatSector

*******************************************************************************/

void InitFatSector(char *pRamDisk) {
  unsigned long nFATOffset;
  char *pFAT;
  tBootSectorType *pBS;

  pBS = (tBootSectorType *)pRamDisk;

  nFATOffset = pBS->BytesPerSector * pBS->SectorsPerCluster; // skip boot sector

  pFAT = &pRamDisk[nFATOffset];

  memset(pFAT, 0, BYTES_PER_SECTOR); // clear area

  pFAT[0] = 0xF8; // default
  pFAT[1] = 0xFF;
  pFAT[2] = 0xFF;
}

/*******************************************************************************

  GetFAT12Entry

*******************************************************************************/

void CreateDefaultDirEntry(char *pRamDisk) {
  tBootSectorType *pBS;
  tDirEntryType tDir;
  unsigned long nRootDirOffset;
  unsigned long nFATOffset;
  unsigned long nDataSecOffset;

  memset((void *)&tDir, 0, sizeof(tDirEntryType)); // clear mem

  memcpy((void *)&tDir.DirName, DEFAULT_FILENAME, 11);

  tDir.DirAttribute = 0;
  tDir.DirNTRes = 0;
  tDir.DirCrtTimeTenth = 0;
  tDir.DirCrtTime = 0;
  tDir.DirCrtDate = 0;
  tDir.DirLstAccDate = 0;
  tDir.DirFstClusHi = 0;
  tDir.DirWrtTime = 0;
  tDir.DirWrtDate = 0;
  tDir.DirFstClusLo = 2;  // Start cluster, is first datablock
  tDir.DirFileSize = 512; // 512 Byte

  // Get root dir
  pBS = (tBootSectorType *)pRamDisk;
  nRootDirOffset = pBS->BytesPerSector * pBS->SectorsPerCluster *
                   (pBS->FATSectors * pBS->FATCount + 1); // + 1 = + boot sector

  // Write first dir entry
  memcpy(&pRamDisk[nRootDirOffset], &tDir, sizeof(tDirEntryType));

  // Set FAT entry
  nFATOffset = pBS->BytesPerSector * pBS->SectorsPerCluster;
  SetFAT12Entry(&pRamDisk[nFATOffset], 2, 0xFFF); // entry 0, last cluster
  SetFAT12Entry(&pRamDisk[nFATOffset], 3, 0xFFF); // entry 0, last cluster		 why ??

  // Set mem to space
  nDataSecOffset =
      nRootDirOffset + pBS->RootEntries * sizeof(tDirEntryType); // should be nRootDirOffset + 0x200
  memset(&pRamDisk[nDataSecOffset], ' ', tDir.DirFileSize);
}

/*******************************************************************************

  GetFAT12Entry

*******************************************************************************/

unsigned short GetFAT12Entry(char *pFAT12Sec, unsigned short nEntry) {
  unsigned long nAddr;
  unsigned short nData;

  nAddr = (nEntry * 3) / 2;

  if (0 == ((nEntry * 3) % 2)) {
    nData = pFAT12Sec[nAddr]; // get bit 4-11
    nData = nData << 4;
    nData += pFAT12Sec[nAddr + 1] >> 4; // get lower nibbel from next byte, bit 0-3
  } else {
    nData = pFAT12Sec[nAddr] & 0x0F; // get lower nibbel from next byte, bit 8-11
    nData = nData << 8;
    nData += pFAT12Sec[nAddr + 1]; // get lower nibbel from next byte, bit 0-3
  }

  return (nData);
}

/*******************************************************************************

  SetFAT12Entry

*******************************************************************************/

void SetFAT12Entry(char *pFAT12Sec, unsigned short nEntry, unsigned short nData) {
  unsigned long nAddr;

  nData &= 0xFFF;

  nAddr = (nEntry * 3) / 2;

  if (0 == ((nEntry * 3) % 2)) {
    pFAT12Sec[nAddr] = nData >> 4;                                                // write bit 4-11
    pFAT12Sec[nAddr + 1] = (pFAT12Sec[nAddr + 1] & 0x0f) + ((nData & 0x0f) << 4); // write bit 0-3
  } else {
    pFAT12Sec[nAddr] = (pFAT12Sec[nAddr + 1] & 0xf0) + (nData >> 8); // write bit 9-11
    pFAT12Sec[nAddr + 1] = nData & 0xff;                             // write bit 0-8
  }
}

/*******************************************************************************

  CreateFilesystem

*******************************************************************************/

void CreateFilesystem(char *pRamDisk) {
  int i;

  for (i = 0; i < DRIVE_SIZE_IN_BYTE; i++) {
    pRamDisk[i] = 0;
  }

  InitBootSector(pRamDisk);
  InitFatSector(pRamDisk);
  CreateDefaultDirEntry(pRamDisk);
}
