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
 *   \ \ `. \        |            alchemy.h                                  *
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

#ifndef ALCHEMY_H
#define ALCHEMY_H
#include <stdint.h>
#include "cpu.h"

#pragma pack(push, 1)
struct alchemyHeader {
	uint8_t flags;
	uint32_t seqnum;
};

struct commandHeader {
	uint8_t major;
	uint8_t minor;
};

struct cmdQueue {
	IPXNet network;
	IPXNode node;
	IPXPort srcport;
	IPXPort dstPort;
	uint8_t major;
	uint8_t minor;
	uint32_t seqnum;
	char*data;
	uint16_t size;
	uint8_t status;
};
#pragma pack(pop)

#define ALC_FLAG_CON 1
#define ALC_FLAG_REJ 2
#define ALC_FLAG_ACK 4
#define ALC_FLAG_FIN 8
#define ALC_FLAG_CONLESS 16
#define ALC_FLAG_COMMAND 32

extern struct cmdQueue cmdQeue[10];

void alchemyInit();
void alchemyTick();
void handleAlchemyPacket(struct ipx_header*header);
int replyPacketSimple(const struct ipx_header*ipxHeader, const struct alchemyHeader*alcHeader, const struct commandHeader*cmdHeader);
int replyPacketEx(const struct ipx_header*ipxHeader, const struct alchemyHeader*alcHeader, const struct commandHeader*cmdHeader, const uint8_t*data, uint32_t dsize);
int alchemyAuthenticate(int key, const unsigned char*token, const struct ipx_header*ipxHeader);

#define CMD_SYSTEM		0
#define CMD_SYSTEM_INTERROGATE	0
#define CMD_SYSTEM_NONCE	1
#define CMD_SET_USER_KEY	2

#define ALC_KEY_DEVICE		0
#define ALC_KEY_USER		1

#endif /* ALCHEMY_H */
