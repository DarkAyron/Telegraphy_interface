/*****************************************************************************
 *                                                                           *
 *                                  (`-.                                     *
 *                                   \  `                                    *
 *      /)         ,   '--.           \    `                                 *
 *     //     , '          \/          \   `   `                             *
 *    //    ,'              ./         /\    \>- `   ,----------.            *
 *   ( \  ,'    .-.-,_        /      ,' /\    \   . `            `.          *
 *    \ \'     />--< .)       ./   ,'  /  \     .      `           `.        *
 *     \ `   -{/    \ .)        / /   / ,' \       `     `-----.     \       *
 *     <\      )     ).:)       ./   /,' ,' \        `.  /\)    `.    \      *
 *      >^,  //     /..:)       /   //--'    \         `(         )    )     *
 *       | ,'/     /. .:)      /   (/         \          \       /    /      *
 *       ( |(_    (...::)     (                \       .-.\     /   ,'       *
 *       (O| /     \:.::)                      /\    ,'   \)   /  ,'         *
 *        \|/      /`.:::)                   ,/  \  /         (  /           *
 *                /  /`,.:)                ,'/    )/           \ \           *
 *              ,' ,'.'  `:>-._._________,<;'    (/            (,'           *
 *            ,'  /  |     `^-^--^--^-^-^-'                                  *
 *  .--------'   /   |                                                       *
 * (       .----'    |                                                       *
 *  \ <`.  \         |                                                       *
 *   \ \ `. \        |            alchemy.c                                  *
 *    \ \  `.`.      |            ---------                                  *
 *     \ \   `.`.    |                                                       *
 *      \ \    `.`.  |            Alchemy library                            *
 *       \ \     `.`.|                                                       *
 *        \ \      `.`.                                                      *
 *         \ \     ,^-'           Copyright (c) 2016 by Ayron                *
 *          \ \    |                                                         *
 *           `.`.  |                                                         *
 *             `.`.|                                                         *
 *               `._>                                                        *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 *   This program is free software: you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation, either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 *                                                                           *
 ****************************************************************************/

#include "defines.h"
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "ipx.h"
#include "enc28j60.h"
#include "alchemy.h"
#include "anubis.h"
#include "telegraph.h"
#include "memory.h"

#define STATUS_ROUTE_WAIT 1
#define STATUS_ACK_WAIT 2

extern uint32_t tickCounter;
static uint32_t validity;
static uint32_t lastTickCounter;
static uint32_t rndState;
static uint32_t linkTimer;
static unsigned char nonceKey[40];

static int lastMajor;
static int lastMinor;
struct cmdQueue cmdQeue[10];
char databuf[512];
static anubisSchedule_t keySchedule;

#include "anubis_key.h"

void alchemyInit()
{
	anubisKeySetup(anubisKey, &keySchedule, 320);
	databuf[511] = 0xaa;
}

void alchemyTick()
{
	if (isConfigured()) {
		if (tickCounter % 1200 == 0) {
			broadcastSAP();
		}
	} else {
		if (encHasLink()) {
			if (linkTimer++ > 2400) {
				configureNet(myDefaultNet);
				broadcastSAP();
			} else if (linkTimer > 2200) {
				if (tickCounter % 3 == 0) {
					PIOA->PIO_ODSR ^= PIO_PA3;
				}
			} else if (linkTimer > 1800) {
				if (tickCounter % 6 == 0) {
					PIOA->PIO_ODSR ^= PIO_PA3;
				}
			}
		} else {
			linkTimer = 0;
			PIOA->PIO_SODR = PIO_PA3;
		}
	}

	rndState = rndState * 1664525 + 1013904223;
	if (!validity) {
		int n;
		for (n = 39; n > 3; n--) {
			nonceKey[n] = nonceKey[n-4];
		}
		*(uint32_t*)nonceKey = rndState;
	} else {
		validity--;
	}
}

int alchemyAuthenticate(int key, const unsigned char*token, const struct ipx_header*ipxHeader)
{
	unsigned char buf[32];
	unsigned char nonce[16];
	anubisKeySetup(nonceKey, &keySchedule, 320);
	alchemyTick(); /* consume the nonce */
	memcpy(buf, &(ipxHeader->srcNetwork), 4);
	memcpy(buf + 4, &(ipxHeader->srcNode), 6);
	memcpy(buf + 10, &(ipxHeader->srcPort), 2);
	memcpy(buf + 12, &lastTickCounter, 4);
	anubisEncrypt(&keySchedule, buf, nonce);

	if (key == ALC_KEY_DEVICE) {
		anubisKeySetup(anubisKey, &keySchedule, 320);
	} else {
		anubisKeySetup(userKey, &keySchedule, 256);
	}
	anubisDecrypt(&keySchedule, token, buf);
	return memcmp(buf, nonce, 16);
}

int replyPacketSimple(const struct ipx_header*ipxHeader, const struct alchemyHeader*alcHeader, const struct commandHeader*cmdHeader)
{
	uint32_t size;
	size = sizeof(struct alchemyHeader);
	if (cmdHeader != 0)
		size += sizeof(struct commandHeader);

	if (!createPacket(ipxHeader->srcNetwork, ipxHeader->srcNode, ipxHeader->srcPort, ipxHeader->destPort, 0x66, size))
		return 0;
	encWriteBuffer(sizeof(struct alchemyHeader), (uint8_t*)alcHeader);
	if (cmdHeader != 0)
		encWriteBuffer(sizeof(struct commandHeader), (uint8_t*)cmdHeader);
	sendPacket();
	return 1;
}

int replyPacketEx(const struct ipx_header*ipxHeader, const struct alchemyHeader*alcHeader, const struct commandHeader*cmdHeader, const uint8_t*data, uint32_t dsize)
{
	uint32_t size;
	size = sizeof(struct alchemyHeader);
	if (cmdHeader != 0)
		size += sizeof(struct commandHeader);
	size += dsize;
	if (!createPacket(ipxHeader->srcNetwork, ipxHeader->srcNode, ipxHeader->srcPort, ipxHeader->destPort, 0x66, size))
		return 0;
	encWriteBuffer(sizeof(struct alchemyHeader), (uint8_t*)alcHeader);
	if (cmdHeader != 0)
		encWriteBuffer(sizeof(struct commandHeader), (uint8_t*)cmdHeader);
	encWriteBuffer(dsize, data);
	sendPacket();
	return 1;
}

static void handleCommand(struct ipx_header*ipxHeader, struct alchemyHeader*alcHeader, struct commandHeader cmdHeader, uint32_t remaining)
{
	unsigned char buf[32];
	lastMajor = cmdHeader.major;
	lastMinor = cmdHeader.minor;
	if (cmdHeader.major == CMD_SYSTEM)
	{
		switch(cmdHeader.minor) {
		case CMD_SYSTEM_INTERROGATE:
			alcHeader->flags |= ALC_FLAG_ACK;
			replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
			break;
		case CMD_SYSTEM_NONCE:
			validity = 0;
			alchemyTick();
			validity = 60;
			lastTickCounter = tickCounter;
			anubisKeySetup(nonceKey, &keySchedule, 320);
			memcpy(buf, &(ipxHeader->srcNetwork), 4);
			memcpy(buf + 4, &(ipxHeader->srcNode), 6);
			memcpy(buf + 10, &(ipxHeader->srcPort), 2);
			memcpy(buf + 12, &lastTickCounter, 4);
			anubisEncrypt(&keySchedule, buf, buf + 16);
			alcHeader->flags |= ALC_FLAG_ACK;
			replyPacketEx(ipxHeader, alcHeader, &cmdHeader, buf + 16, 16);
			break;
		case CMD_SET_USER_KEY:
			if (remaining != 48) {
				alcHeader->flags |= ALC_FLAG_REJ;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				break;
			} else {
				encReadBuffer(16, buf);
				if (alchemyAuthenticate(ALC_KEY_DEVICE, buf, ipxHeader)) {
					alcHeader->flags |= ALC_FLAG_REJ;
					replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
					break;
				}
				encReadBuffer(32, buf);
				myMemcpy(userKey, buf, 32);
				memory_eraseAndWrite(userKey);
				alcHeader->flags |= ALC_FLAG_ACK;
				replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
				break;
			}
		default:
			alcHeader->flags |= ALC_FLAG_REJ;
			replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
		}
	} else if (ipxHeader->destPort == TELEGRAPHY_PORT) {
		if (!telegraphHandleCommand(ipxHeader, alcHeader, cmdHeader, remaining)) {
			alcHeader->flags |= ALC_FLAG_REJ;
			replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
		}
	} else {
		alcHeader->flags |= ALC_FLAG_REJ;
		replyPacketSimple(ipxHeader, alcHeader, &cmdHeader);
	}
}

void handleAlchemyPacket(struct ipx_header*header)
{
	struct alchemyHeader alcHeader;
	struct commandHeader cmdHeader;
	uint32_t remaining = header->length - IPX_HEADER_LENGTH - sizeof(alcHeader);

	if (header->type != 0x66) {
		/* ignore */
		return;
	}

	encReadBuffer(sizeof(struct alchemyHeader), (uint8_t*)&alcHeader);
	if (alcHeader.flags & ALC_FLAG_COMMAND) {
		encReadBuffer(sizeof(struct commandHeader), (uint8_t*)&cmdHeader);
		remaining -= sizeof(struct commandHeader);
		handleCommand(header, &alcHeader, cmdHeader, remaining);
	} else {
		/* nothing else at the moment */
		alcHeader.flags |= ALC_FLAG_REJ;
		replyPacketSimple(header, &alcHeader, 0);
	}
}
