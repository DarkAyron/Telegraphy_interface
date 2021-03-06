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
 telegraph.c
 
 Created on: 30 May 2017
 
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

#include <string.h>
#include "cpu.h"
#include "telegraph.h"
#include "memory.h"
#include "code.h"
#include "enc28j60.h"
#include "ipx.h"
#include "alchemy.h"
#include "mtask.h"

static volatile int key;
static char text[1518];
static unsigned int replyConnection;
static int doReply;
static void sendBufferEmpty(int data);

void dot()
{
	int bufferEmpty = 0;
	TC_GetStatus(TC0, 0);
	if (key > 0) {
		key--;
		if (key == 0) {
			/*PMC->PMC_SCDR = PMC_SCER_PCK0;*/
			PIOA->PIO_SODR = (1 << 22);
		}
	} else if (key < 0) {
		key++;
	} else {
		int i = getNext(&bufferEmpty);
		switch (i) {
		case 0:
			key = 0;
			TC_Stop(TC0, 0);
			/* PIOA->PIO_SODR = (1 << 21);
			SysTick->CTRL &= (~SysTick_CTRL_ENABLE_Msk); */
			break;
		case 1:
			/* key a dot */
			/*PMC->PMC_SCER = PMC_SCER_PCK0;*/
			PIOA->PIO_CODR = (1 << 22);
			key = 1;
			break;
		case 2:
			/* key a dash */
			/*PMC->PMC_SCER = PMC_SCER_PCK0;*/
			PIOA->PIO_CODR = (1 << 22);
			key = 3;
			break;
		case 3:
			/* key a short pause */
			key = 0;
			break;
		case 4:
			/* key a long pause */
			key = -1;
			break;
		case 5:
			/* key a long pause */
			key = -50;
			break;
		case 6:
			/* key a short dash */
			PIOA->PIO_CODR = (1 << 22);
			key = 2;
			break;
		case 7:
			/* key a longer dash */
			PIOA->PIO_CODR = (1 << 22);
			key = 4;
			break;
		case 8:
			/* key a very long dash */
			PIOA->PIO_CODR = (1 << 22);
			key = 5;
			break;

		}
	}

	if (bufferEmpty && doReply) {
		coroutine_invoke_later(sendBufferEmpty, (int)replyConnection, "reply");
	}
}

int telegraphHandleCommand(struct ipx_header*ipxHeader, struct alchemyHeader*alcHeader, struct commandHeader cmdHeader, uint32_t remaining)
{
	int i1;
	int i2;
	unsigned char buf[48];

	if (cmdHeader.major == 3) {
		switch(cmdHeader.minor) {
		case 1:
			/* Configure call */
			if (remaining < 33 || remaining > 40) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(32, buf);
			if (alchemyAuthenticate(ALC_KEY_DEVICE, buf, ipxHeader)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(remaining - 32, buf);
			buf[remaining - 32] = 0;
			memory_prepareWrite(vConfiguration);
			myStrcpy(vConfiguration->callsign, (char*)buf);
			memory_eraseAndWrite(vConfiguration);
			break;
		case 2:
			/* Configure device name */
			if (remaining < 33 || remaining >= 80) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(32, buf);
			if (alchemyAuthenticate(ALC_KEY_DEVICE, buf, ipxHeader)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(remaining - 32, buf);
			buf[remaining - 32] = 0;
			memory_prepareWrite(vConfiguration);
			myStrcpy(vConfiguration->name, (char*)buf);
			memory_eraseAndWrite(vConfiguration);
			break;

		}
	}
	else if ((cmdHeader.major == 2) && (~alcHeader->flags & ALC_FLAG_CONLESS)) {
		switch(cmdHeader.minor) {
		case 0:
			/* Configure mode */
			if (remaining != 4) {
				return 0;
			}
			encReadBuffer(4, (uint8_t*)&i1);
			byteSwapl((uint32_t*)&i1);
			if ((i1 < 0) || (i1 > 2)) {
				return 0;
			}
			encoderSetMode(i1);
			break;
		case 1:
			/* Configure speed */
			if (remaining != 4) {
				return 0;
			}
			encReadBuffer(4, (uint8_t*)&i1);
			byteSwapl((uint32_t*)&i1);
			if ((i1 < 0) || (i1 > 60)) {
				return 0;
			}
			encoderSetSpeed(i1, 0);
			break;
		case 2:
			/* Configure speed and Farnsworth */
			if (remaining != 8) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(4, (uint8_t*)&i1);
			encReadBuffer(4, (uint8_t*)&i2);
			byteSwapl((uint32_t*)&i1);
			byteSwapl((uint32_t*)&i2);
			if ((i1 < 0) || (i1 > 60) || (i2 < 0) || (i2 > 40)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}

			encoderSetSpeed(i1, i2);
			break;
		case 3:
			/* Configure call */
			if (remaining < 33 || remaining > 40) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(32, buf);
			if (alchemyAuthenticate(ALC_KEY_DEVICE, buf, ipxHeader)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(remaining - 32, buf);
			buf[remaining - 32] = 0;
			memory_prepareWrite(vConfiguration);
			myStrcpy(vConfiguration->callsign, (char*)buf);
			memory_eraseAndWrite(vConfiguration);
			break;
		case 4:
			/* Configure device name */
			if (remaining < 33 || remaining >= 80) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(32, buf);
			if (alchemyAuthenticate(ALC_KEY_DEVICE, buf, ipxHeader)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(remaining - 32, buf);
			buf[remaining - 32] = 0;
			memory_prepareWrite(vConfiguration);
			myStrcpy(vConfiguration->name, (char*)buf);
			memory_eraseAndWrite(vConfiguration);
			break;
		case 5:
			/* Send call */
			alcHeader->flags |= ALC_FLAG_ACK;
			replyPacketEx(ipxHeader, alcHeader, &cmdHeader, (unsigned char*)vConfiguration->callsign, strlen(vConfiguration->callsign));
			return 1;
		default:
			return 0;
		}
	} else if (cmdHeader.major == 1) {
		switch(cmdHeader.minor) {
		case 0:
			replyConnection = getCurrentConnection();
			encReadBuffer(remaining, (uint8_t*)text);
			text[remaining] = 0;
			encoderSetText(text);
			break;
		case 1:
			if (remaining != 32) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}
			encReadBuffer(32, buf);
			if (alchemyAuthenticate(ALC_KEY_USER, buf, ipxHeader)) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				return 1;
			}

			TC_Start(TC0, 0);
			doReply = 1;
			break;
		}
	}

	alcHeader->flags |= ALC_FLAG_ACK;
	replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
	return 1;
}

static void sendBufferEmpty(int data)
{
	struct ipx_header ipxHeader;
	struct alchemyHeader alcHeader;
	struct commandHeader cmdHeader;

	if (!setCurrentConnection(data))
		return;

	getReplyHeader(&ipxHeader);

	cmdHeader.major = CMD_TELEGRAPHY;
	cmdHeader.minor = CMD_TELEGRAPHY_BUFFERE;

	alcHeader.flags = ALC_FLAG_COMMAND;

	alcHeader.seqnum = getNextSequenceNumber();
	replyPacketSimple(&ipxHeader, &alcHeader, &cmdHeader);

}

