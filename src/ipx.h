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
 *   \ \ `. \        |            ipx.h                                      *
 *    \ \  `.`.      |            -----                                      *
 *     \ \   `.`.    |                                                       *
 *      \ \    `.`.  |            IPX library for ENC28J60 driver            *
 *       \ \     `.`.|                                                       *
 *        \ \      `.`.                                                      *
 *         \ \     ,^-'           Copyright (c) 2015 by Ayron                *
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

#ifndef IPX_H
#define IPX_H
#include <stdint.h>
#include "cpu.h"

typedef uint8_t IPXNode[6];
typedef uint32_t IPXNet;
typedef uint16_t IPXPort;
typedef uint16_t hop_t;
typedef uint16_t tick_t;

/******* ETH *******/
#define ETH_HEADER_LEN	14
#define ETH_PACKET_MIN	60

struct PACK eth_header
{
	IPXNode destNode;
	IPXNode srcNode;
	uint16_t type;
};

struct PACK eth_header_llc
{
	uint8_t dsap;
	uint8_t ssap;
	uint8_t control;
};

/* values of certain bytes: */
#define ETHTYPE_IPX 0x8137

/* Ethernet type field (2bytes): */
#define ETH_TYPE_H_P 12
#define ETH_TYPE_L_P 13

#define ETH_DST_MAC 0
#define ETH_SRC_MAC 6

/******* IPX *******/
#define IPX_HEADER_LENGTH 30

struct PACK ipx_header
{
	uint16_t chksum;
	uint16_t length;
	uint8_t hops;
	uint8_t type;
	IPXNet destNetwork;
	IPXNode destNode;
	IPXPort destPort;
	IPXNet srcNetwork;
	IPXNode srcNode;
	IPXPort srcPort;
};

struct ipx_route
{
	IPXNet network;
	IPXNode node;
	int8_t priority;
};

#define IPX_CHKSUM		(0xFFFFU)
#define IPX_NET_BROADCAST	(0xFFFFFFFFU)

void ipxInitialize(const IPXNode node);
int dispatchPacket();
int createPacket(IPXNet toNet, const IPXNode toNode, IPXPort toPort, IPXPort fromPort, uint8_t type, uint16_t length);
int recreatePacket();
void sendPacket();
int haveRoute();
void broadcastSAP();
int isConfigured();
void configureNet(IPXNet netnum);

void byteSwaps(uint16_t * bytes);
void byteSwapl(uint32_t * bytes);


/******* RIP *******/
#define IPX_RIP_PORT		(0x453U)
#define IPX_RIP_PTYPE		(1U)
#define IPX_RIP_OP_REQUEST	(1U)
#define IPX_RIP_GENERAL_RQ	(0xFFFFFFFFU)
#define IPX_RIP_OP_RESPONSE	(2U)
#define IPX_RIP_MAX_ENTRIES	(50U)
#define IPX_RIP_NET_DOWN	(16U)

struct PACK rip_entry
{
	IPXNet network;
	hop_t  hops;
	tick_t ticks;
};

struct PACK rip_packet
{
	uint16_t operation;
	struct rip_entry   rip_entries[IPX_RIP_MAX_ENTRIES];
};

void requestRIP(IPXNet network);

/******* SAP *******/
#define IPX_SAP_PORT		(0x452U)
#define IPX_SAP_PTYPE		(4U)
#define IPX_SAP_OP_REQUEST	(1U)
#define IPX_SAP_GENERAL_RQ	(0xFFFFU)
#define IPX_SAP_OP_RESPONSE	(2U)
#define IPX_SAP_OP_GNS_REQUEST	(3U)
#define IPX_SAP_OP_GNS_RESPONSE	(4U)
#define IPX_SAP_MAX_ENTRIES	(7U)
#define IPX_SAP_SERVER_DOWN	(16U)
#define IPX_SAP_SERVER_NAME_LEN	(48U)
#define IPX_SAP_REQUEST_LEN	(4U)

typedef uint16_t ser_type_t;
typedef char ser_name_t[IPX_SAP_SERVER_NAME_LEN];

struct PACK sap_entry
{
	ser_type_t ser_type;
	ser_name_t ser_name;
	IPXNet network;
	IPXNode node;
	IPXPort port;
	hop_t hops;
};

struct PACK sap_packet
{
	uint16_t operation;
	struct sap_entry   sap_entries[IPX_SAP_MAX_ENTRIES];
};

#endif /* IPX_H */
