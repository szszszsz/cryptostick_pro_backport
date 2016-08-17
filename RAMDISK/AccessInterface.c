/*
* Author: Copyright (C) Rudolf Boeddeker 					Date: 2010-01-13
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


#include "stm32f10x.h"
#include "platform_config.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

#include "string.h"
#include "stdio.h"
#include "setjmp.h"
#include "ramdisk.h"
#include "AccessInterface.h"
#include "FAT12.h"

short nCommandCounter = 0;

void Delay(u32 nCount);
void Delay_noUSBCheck (u32 nCount);

#define RAM_DISK_BACKUPFILE_OFFSET  0x400

/*******************************************************************************

 USB_SwitchMassStorage

 Switch the USB device context a jump to main

*******************************************************************************/

extern jmp_buf jmpRestartUSB;

int USB_SwitchUSBDevice (int nNewUSBDev)
{
  Delay_noUSBCheck (20);							// Wait 100 ms

	Suspend ();

  Delay_noUSBCheck (20);							// Wait 100 ms

	PowerOff();
												 	
	Delay_noUSBCheck (50);							// Wait 500 ms

	USB_SetDeviceConfiguration (nNewUSBDev);

  longjmp(jmpRestartUSB, TRUE);				// jump to main - live the new life

	return (TRUE);
}

/*******************************************************************************

 USB_Reconnect

 Restart the same USB device 

*******************************************************************************/

int USB_Reconnect (void)
{
	Delay_noUSBCheck (100);							// Wait 1000 ms

	Suspend ();

	Delay_noUSBCheck (10);							// Wait 100 ms

	PowerOff();
												 	
	Delay_noUSBCheck (100);							// Wait 1000 ms

  USB_Init();

  while (bDeviceState != CONFIGURED);

	return (TRUE);
}

/*******************************************************************************

 	SetRAMFileText

 	Set text in the readme file

 	Input		nPos			actual cursor
 					szText		text to insert

	Return	new position of cursor
					0 	EOF	- 0 > to avoid buffer overflows

*******************************************************************************/

#define RAM_DISK_OFFSET_TO_FILESTART (4*RAMDISK_BLOCK_SIZE)
#define MAX_FILETEXT_SIZE            RAMDISK_BLOCK_SIZE

short SetRAMFileText (u16 nPos, char *szText)
{
	char  *pFileStart;
	short  n;

	pFileStart = (char*)&RamdiskStart[RAM_DISK_OFFSET_TO_FILESTART];

	n = strlen (szText);
	if (MAX_FILETEXT_SIZE - 1 <= n + nPos) 		// check for end of file
	{
		return (0);
	}

	strncpy (&pFileStart[nPos],szText,n);

	return (n+nPos);
}

/*******************************************************************************

 	SetFileHeader

 	Clean the file and set the file header data
 
	Return	new position of cursor

*******************************************************************************/
#define RAM_DISK_FILESIZE  RAMDISK_BLOCK_SIZE

short SetFileHeader (void)
{
	short  nFilePos;
	char  *pFileStart;
	short  i;

/* Create clean filesysten */
	CreateFilesystem (RamdiskStart);

/* Clear file data */
	pFileStart = (char*)&RamdiskStart[RAM_DISK_OFFSET_TO_FILESTART];
	for (i=0;i<RAM_DISK_FILESIZE;i++)
	{
		pFileStart[i] = ' ';
	}

/* Clear possible backup file data - linux */
	pFileStart = (char*)&RamdiskStart[RAM_DISK_OFFSET_TO_FILESTART+RAM_DISK_BACKUPFILE_OFFSET];
	for (i=0;i<RAM_DISK_FILESIZE;i++)
	{
		pFileStart[i] = ' ';
	}

	nFilePos = 0;

	nFilePos = SetRAMFileText (nFilePos,"Crypto-Stick V 0.2 DEMO\015\012");
	nFilePos = SetRAMFileText (nFilePos,"\015\012");
/*
	sprintf (szText,"C = %d\015\012",nCommandCounter);
	nFilePos = SetRAMFileText (nFilePos,szText);
*/
	nFilePos = SetRAMFileText (nFilePos,"\015\012");

	return (nFilePos);
}

/*******************************************************************************

 	SetInputLine

 	Input		nFilePos					actual cursor
 					*nInputCharPos		pointer where the user entry had to come

	Return	new position of cursor
					-1 	EOF	

*******************************************************************************/

#define INPUT_CHAR_OFFEST   5

short SetInputLine (short nFilePos,short *nInputCharPos)
{
	nFilePos = SetRAMFileText (nFilePos,"\015\012");
	nFilePos = SetRAMFileText (nFilePos,"Please enter input char here >> <<\015\012");

	*nInputCharPos = nFilePos - INPUT_CHAR_OFFEST;

	return (nFilePos);
}


/*******************************************************************************

 	WaitForInputChar

 	Input		nInputCharPos		pointer where the user entry had to come

	Return	User entry (char)

*******************************************************************************/

char WaitForInputChar (short nInputCharPos)
{
	char  *pFileStart;

	pFileStart = (char*)&RamdiskStart[RAM_DISK_OFFSET_TO_FILESTART];

	if ('>' != pFileStart[nInputCharPos-1])			// wait for end of data transfer
	{
		Delay (3);		
	}

	while (' ' == pFileStart[nInputCharPos])
	{
		Delay (1);																// possible endless loop !!!

// is the new data stored a new file - for linux
		if ('>' == pFileStart[nInputCharPos-1+0x400])			
		{
			if (' ' != pFileStart[nInputCharPos+0x400])
			{
				return (pFileStart[nInputCharPos+0x400]);
			}
		}
	}

	return (pFileStart[nInputCharPos]);
}

/*******************************************************************************

 	ShowHelp

 	Input		nPos			actual cursor

	Return	new position of cursor

*******************************************************************************/

short ShowHelp (short  nFilePos)
{
	nFilePos = SetRAMFileText (nFilePos,"\015\012");
	nFilePos = SetRAMFileText (nFilePos,"HELP\015\012");
	nFilePos = SetRAMFileText (nFilePos,"h = Show this help\015\012");
	nFilePos = SetRAMFileText (nFilePos,"p = Enter pin\015\012");
	nFilePos = SetRAMFileText (nFilePos,"s = Set pin\015\012");
	nFilePos = SetRAMFileText (nFilePos,"O = Activate OpenPGP interface\015\012");
	nFilePos = SetRAMFileText (nFilePos,"E = Activete SD card interface\015\012");
	nFilePos = SetRAMFileText (nFilePos,"h = Show this help\015\012");

	return (nFilePos);
}

/*******************************************************************************

 PrintPasswordMatrix

 Displays the password maxtrix stored in cPasswordMatrix

	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0
	
	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0
	
	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0
	 0 0 0  0 0 0  0 0 0

*******************************************************************************/

#define MATRIX_SIZE       9
#define SUB_MATRIX_SIZE		3

unsigned char cPasswordMatrix[MATRIX_SIZE][MATRIX_SIZE];			 // store of password matrix

short PrintPasswordMatrix (short nFilePos)
{
	short i;
	short i1;
	short nTextLen;
	char  szText[80];

	for (i=0;i<MATRIX_SIZE;i++)					
	{
		strcpy (szText,"  ");
		nTextLen = 2;

		for (i1=0;i1<MATRIX_SIZE;i1++)	 // generate a line	
		{
			szText[nTextLen++] = '0' + cPasswordMatrix[i][i1];
			szText[nTextLen++] = ' ';

			if ((i1 % SUB_MATRIX_SIZE ) == 2)
			{
				szText[nTextLen++] = ' ';
			}

		}
		szText[nTextLen++] = 13;
		szText[nTextLen++] = 10;
		szText[nTextLen]   = 0;
	  nFilePos = SetRAMFileText (nFilePos,szText);

		if ((i % SUB_MATRIX_SIZE) == 2)
		{
			nFilePos = SetRAMFileText (nFilePos,"\015\012");
		}
	}

	nFilePos = SetRAMFileText (nFilePos,"Enter now the pin char\015\012");

	return (nFilePos);
}


/*******************************************************************************

 	RamDiskUserInterface

	Get data via the ramdisk file user interface

*******************************************************************************/

int RamDiskUserInterface (void)
{
	short  nFilePos      = 0;
	short  nInputCharPos = 0;
	char   cUserInput;

/* Generate the first ramdisk data */
	nFilePos = SetFileHeader ();
	nFilePos = PrintPasswordMatrix (nFilePos);	
	nFilePos = SetInputLine	(nFilePos,&nInputCharPos);

	while (1)
	{
/* get the user entry */
	  cUserInput = WaitForInputChar (nInputCharPos);
		nCommandCounter++;

/* switch to new USB device ? */
		switch (cUserInput)
		{
			case 'E' :
									Delay_noUSBCheck (100);												// Wait 1000 ms
									Suspend ();																		// Suspend USB
									USB_SwitchUSBDevice (STICK_STATE_SD_DISK);  	// jump to reset
									break;																				// never goes here
			case 'O' :
									Delay_noUSBCheck (100);												// Wait 1000 ms
									Suspend ();																		// Suspend USB
									USB_SwitchUSBDevice (STICK_STATE_SMARTCARD);  // jump to reset
									break;																				// never goes here
		}

// invalidate ramdisk data		 
		USB_Reconnect ();

// generate the new ramdisk data												 	
		nFilePos = SetFileHeader ();	 			

		switch (cUserInput)
		{
			case 'H' :
			case 'h' :
									nFilePos = ShowHelp (nFilePos);
									break;
			case 'P' :
			case 'p' :
									nFilePos = PrintPasswordMatrix (nFilePos);
									break;
		}

		nFilePos = SetInputLine	(nFilePos,&nInputCharPos);										 	
	}

	// never comes here
}





