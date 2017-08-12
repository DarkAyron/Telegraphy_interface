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
 memory.c
 
 Created on: 2 Jul 2017
 
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

#include "cpu.h"
#include "ipx.h"
#include "memory.h"
#include <string.h>

IPXNode myNode;
IPXNet myDefaultNet;
configuration_t *vConfiguration;
unsigned char*userKey;

__attribute__((section(".ramfunc")))
__attribute__((noinline))
void memory_readUID()
{
	char c[12];
	__disable_irq();
	efc_perform_read_sequence(EFC0, EFC_FCMD_STUI, EFC_FCMD_SPUI, (uint32_t*)c, 3);

	myNode[0] = 0x1a;
	myNode[1] = c[3];
	myNode[2] = c[4];
	myNode[3] = c[5];
	myNode[4] = c[6];
	myNode[5] = c[7];

	myDefaultNet = *((IPXNet*)(c + 8));

	efc_perform_command(EFC0, EFC_FCMD_SPUI, 0);
	__enable_irq();
}

void memory_prepareWrite(void *data)
{
	uint32_t *d = (uint32_t*)0;
	uint32_t *s = (uint32_t*)data;
	int n;

	for (n = 0; n < FLASH_PAGE_SIZE; n += 4) {
		d[n] = s[n];
	}
}

void memory_eraseAndWrite(void *data)
{
	int pagenum;

	pagenum = ((uint32_t)data & 0xfff00) / FLASH_PAGE_SIZE;
	efc_perform_command(EFC0, EFC_FCMD_EWP, pagenum);
}

static void loadDefaultValues()
{
	myStrcpy(vConfiguration->callsign,"NONE");
	myStrcpy(vConfiguration->name, "Telegraphy");
	vConfiguration->sapType = 0x4357;
	vConfiguration->port = 5016;
	memory_eraseAndWrite(vConfiguration);
}

void memory_init()
{
	vConfiguration = (configuration_t*)CONFIGURATION_ADDRESS;
	userKey = (unsigned char*)USER_KEY_ADDRESS;

	if (vConfiguration->callsign[0] == 0xff) {
		loadDefaultValues();
	}
}

void myStrcpy(char *dst, const char *src)
{
	int len;
	int n;


	len = strlen(src) + 1;
	myMemcpy(dst, src, len);
}

void myMemcpy(void *dst, const void *src, unsigned int len)
{
	int len2;
	int n;

	unsigned int x = 0xffffffffu;
	char *cp = (char*)&x;

	len2 = len % 4;
	for (n = 0; n < len; n++) {
		if ((n != 0) && (n % 4 == 0)) {
			*((unsigned int*)dst) = x;
			dst += 4;
			x = 0xffffffffu;
		}
		cp[n % 4] = ((char*)src)[n];
	}

	if (len2) {
		*((unsigned int*)dst) = x;
	}
}
