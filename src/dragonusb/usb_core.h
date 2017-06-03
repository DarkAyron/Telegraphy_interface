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
 usb_core.h

 Created on: 2 Apr 2017

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

#ifndef DRAGONUSB_USB_CORE_H_
#define DRAGONUSB_USB_CORE_H_

#define ENDPOINT_CTRL	0
#define ENDPOINT_BOUT	1
#define ENDPOINT_BIN	2
#define ENDPOINT_INTIN	3

struct USB_Ctrl_Setup
{
	uint8_t RequestType;
	uint8_t Request;
	uint8_t valueL;
	uint8_t valueH;
	uint16_t index;
	uint16_t length;
};

enum USB_CTRL_STD_Requests
{
	USB_CTRL_STD_Req_GET_DEV_STATUS = 0,
	USB_CTRL_STD_Req_CLEAR_FEATURE = 1,
	USB_CTRL_STD_Req_GET_EP_STATUS = 2,
	USB_CTRL_STD_Req_SET_FEATURE = 3,
	USB_CTRL_STD_Req_SET_ADDRESS = 5,
	USB_CTRL_STD_Req_GET_DESCRIPTOR = 6,
	USB_CTRL_STD_Req_SET_DESCRIPTOR = 7,
	USB_CTRL_STD_Req_GET_CONFIGURATION = 8,
	USB_CTRL_STD_Req_SET_CONFIGURATION = 9,
	USB_CTRL_STD_Req_GET_INTERFACE = 10,
	USB_CTRL_STD_Req_SET_INTERFACE = 11
};

#define USB_CTRL_REQUEST_DIR_HOSTTODEVICE	0x00
#define USB_CTRL_REQUEST_DIR_DEVICETOHOST	0x80
#define USB_CTRL_REQUEST_DIR_MASK		0x80

#define USB_CTRL_REQUEST_TYPE_STANDARD		0x00
#define USB_CTRL_REQUEST_TYPE_CLASS		0x20
#define USB_CTRL_REQUEST_TYPE_VENDOR		0x40
#define USB_CTRL_REQUEST_TYPE_MASK		0x60

#define USB_CTRL_REQUEST_RCPT_DEVICE		0x00
#define USB_CTRL_REQUEST_RCPT_INTERFACE		0x01
#define USB_CTRL_REQUEST_RCPT_ENDPOINT		0x02
#define USB_CTRL_REQUEST_RCPT_OTHER		0x03
#define USB_CTRL_REQUEST_RCPT_MASK		0x1F

#define USB_CTRL_REQUEST_STATUS_DEVICE		0x80
#define USB_CTRL_REQUEST_STATUS_INTF		0x81
#define USB_CTRL_REQUEST_STATUS_EP		0x82

#define MAX_USB_CTRL_EP_SIZE	64
#define MAX_USB_EP_SIZE_L	0
#define MAX_USB_EP_SIZE_H	2

enum USB_DESC_Type
{
	USB_DESC_DEV = 1,
	USB_DESC_CFG,
	USB_DESC_STR,
	USB_DESC_INTF,
	USB_DESC_EP,
	USB_DESC_DEV_QUAL
};

void USB_Init();
void USB_Reset();
void USB_stallEndpoint(int ep);
void USB_unstallEndpoint(int ep);
int USB_send_ctrl(const uint8_t *data, unsigned int len);

#endif /* DRAGONUSB_USB_CORE_H_ */
