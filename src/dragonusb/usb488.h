/*____________________________________________________,8_________________
 ___________________________________________________,d8*_________________
 _________________________________________________,d88**_________________
 _______________________________________________,d888`**_________________
 ____________________________________________,d888`****__________________
 ___________________________________________,d88`******__________________
 _________________________________________,d88`*********_________________
 ________________________________________,d8`************________________
 ______________________________________,d8****************_______________
 ____________________________________,d88**************..d**`____________
 __________________________________,d88`*********..d8*`****______________
 ________________________________,d888`****..d8P*`********_______________
 ________________________._____,d8888*8888*`*************________________
 ______________________,*_____,88888*8P*****************_________________
 ____________________,*______d888888*8b.****************_________________
 __________________,P_______dP__*888.*888b.**************________________
 ________________,8*________8____*888*8`**88888b.*********_______________
 ______________,dP________________*88_8b.*******88b.******_______________
 _____________d8`__________________*8b_8b.***********8b.***______________
 ___________,d8`____________________*8._8b.**************88b.____________
 __________d8P_______________________88.*8b.***************______________
 ________,88P________________________*88**8b.************________________
 _______d888*_______.d88P____________888***8b.*********__________________
 ______d8888b..d888888*______________888****8b.*******________*__________
 ____,888888888888888b.______________888*****8b.*****_________8__________
 ___,8*;88888P*****788888888ba.______888******8b.****______*_8'_*________
 __,8;,8888*_________`88888*_________d88*******8b.***_______*8*'_________
 __)8e888*__________,88888be._______888*********8b.**_______8'___________
 _,d888`___________,8888888***_____d888**********88b.*____d8'____________
 ,d88P`___________,8888888Pb._____d888`***********888b.__d8'_____________
 888*____________,88888888**___.d8888*************______d8'______________
 `88____________,888888888____.d88888b*********________d88'______________
 ______________,8888888888bd888888********_____________d88'______________
 ______________d888888888888d888********________________d88'_____________
 ______________8888888888888888b.****____________________d88'____________
 ______________88*._*88888888888b.*______.oo._____________d888'__________
 ______________`888b.`8888888888888b._.d8888P_______________d888'________
 _______________**88b.`*8888888888888888888888b...____________d888'______
 ________________*888b.`*8888888888P***7888888888888e.__________d888'____
 _________________88888b.`********.d8888b**__`88888P*____________d888'___
 _________________`888888b_____.d88888888888**__`8888.____________d888'__
 __________________)888888.___d888888888888P______`8888888b.______d88888'
 _________________,88888*____d88888888888**`________`8888b__________d888'
 ________________,8888*____.8888888888P`______________`888b.________d888'
 _______________,888*______888888888b...________________`88P88b.__d8888'_
 ______.db.___,d88*________88888888888888b________________`88888888888___
 __,d888888b.8888`_________`*888888888888888888P`______________******____
 _/*****8888b**`______________`***8888P*``8888`__________________________
 __/**88`_______________________________/**88`___________________________
 __`|'_____________________________`|*8888888'___________________________

 ************************************************************************
 usb488.h
 
 Created on: 8 Apr 2017
 
 Copyright 2017 ayron
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ***********************************************************************/

#ifndef DRAGONUSB_USB488_H_
#define DRAGONUSB_USB488_H_

#include "usb_core.h"

void USB488_processData();
int USB488_processCtrl(struct USB_Ctrl_Setup *setup);

struct USB488_Bulk_Header {
	uint8_t MsgID;
	uint8_t bTag;
	uint8_t bTagInverse;
	uint8_t Reserved;
};

#define USB488_DEV_DEP_MSG_OUT		1
#define USB488_DEV_DEP_MSG_IN		2
#define USB488_VENDOR_SPECIFIC_OUT	126
#define USB488_VENDOR_SPECIFIC_IN	127
#define USB488_TRIGGER			128

#pragma pack(push, 1)
struct USB488_Bulk_DEV_DEP_MSG {
	uint32_t TransferSize;
	uint8_t bmTransferAttributes;
	uint8_t TermChar;
	uint16_t Reserved;
};

struct USB488_Bulk_VENDOR_SPECIFIC {
	uint32_t TransferSize;
	uint32_t Reserved;
};

struct USB488_Ctrl_CHECK_STATUS_response {
	uint8_t USBTMC_status;
	uint8_t bmAbortBulkIn;
	uint16_t reserved;
	uint32_t NBYTES_RXD;
};
#pragma pack(pop)

#define USB488_BUFFER_SIZE 1024

struct USB488_dataStream {
	char buffer[USB488_BUFFER_SIZE];
	char *wPointer;
	char *rPointer;
	int bytesAvailable;
};

struct USB488_localRemoteState {
	int REN;
	int localLockout;
	int remote;
};

extern struct USB488_dataStream USB_inStream;
extern struct USB488_dataStream USB_outStream;
extern struct USB488_localRemoteState localRemoteState;

#define USB488_CTRL_INITIATE_ABORT_BULK_OUT 	1
#define USB488_CTRL_CHECK_ABORT_BULK_OUT_STATUS	2
#define USB488_CTRL_INITIATE_ABORT_BULK_IN	3
#define USB488_CTRL_CHECK_ABORT_BULK_IN_STATUS	4
#define USB488_CTRL_INITIATE_CLEAR		5
#define USB488_CTRL_CHECK_CLEAR_STATUS		6
#define USB488_CTRL_GET_CAPABILITIES		7
#define USB488_CTRL_INDICATOR_PULSE		64
#define USB488_CTRL_READ_STATUS_BYTE		128
#define USB488_CTRL_REN_CONTROL			160
#define USB488_CTRL_GO_TO_LOCAL			161
#define USB488_CTRL_LOCAL_LOCKOUT		162

#define USB488_CTRL_STATUS_SUCCESS	0x01
#define USB488_CTRL_STATUS_PENDING	0x02
#define USB488_CTRL_STATUS_INTERRUPT_IN_BUSY	0x20
#define USB488_CTRL_STATUS_FAILED	0x80
#define USB488_CTRL_STATUS_TRANSFER_NOT_IN_PROGRESS	0x81
#define USB488_CTRL_STATUS_SPLIT_IN_PROGRESS	0x83
#endif /* DRAGONUSB_USB488_H_ */
