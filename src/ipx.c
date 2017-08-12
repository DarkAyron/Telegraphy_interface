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
 *   \ \ `. \        |            ipx.c                                      *
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

#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "ipx.h"
#include "enc28j60.h"
#include "alchemy.h"
#include "SEGGER/SEGGER_RTT.h"
#include "memory.h"

#define MAX_ROUTES 3		/* 7 routes should be enough */
struct state {
	IPXNet toNet;
	IPXNode toNode;
	IPXPort toPort;
	IPXPort fromPort;
	uint8_t type;
	uint16_t length;
	uint8_t rip_requested;
};

static IPXNet myNet;		/* for convenience this is stored in network byte order */
static struct ipx_route routingTable[MAX_ROUTES];
static int createPacket2(IPXNet toNet, const IPXNode toNode, IPXPort toPort,
		 IPXPort fromPort, uint8_t type, uint16_t length);
static void sendPacket2(uint16_t length);
static void sendSAP(uint16_t operation, const IPXNode destination, IPXPort port);
static uint16_t remaining;
static struct state pktgen_state;
static struct state pktgen_state_back;

ROM static const unsigned char junk[] = {
	"For a thousand years to come "
	    "will the flight of dragons fill the clouds "
	    "and the shine of spring will lighten again "
	    "on the king and on his land. "
};

#define JUNKLENGTH 144

ROM const IPXNode broadcastNode = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint8_t junk_off = 0;

/* Change byte order because my Cortex-M is little endian */
void byteSwaps(uint16_t * bytes)
{
	uint8_t *cbytes = (uint8_t *) bytes;
	uint8_t c;
	c = cbytes[0];
	cbytes[0] = cbytes[1];
	cbytes[1] = c;
}

void byteSwapl(uint32_t * bytes)
{
	uint8_t *cbytes = (uint8_t *) bytes;
	uint8_t c;
	c = cbytes[0];
	cbytes[0] = cbytes[3];
	cbytes[3] = c;
	c = cbytes[1];
	cbytes[1] = cbytes[2];
	cbytes[2] = c;
}

static int isMeOrBroadcast(struct ipx_header *packet)
{
	if (!memcmp(packet->destNode, "\xff\xff\xff\xff\xff\xff", 6)) {
		/* Broadcast packet */
		if ((packet->destNetwork == myNet) || (myNet == 0))
			return 2;
	}

	if (myNet == 0)
		return 0;

	if (!memcmp(packet->destNode, myNode, 6)
	    && (packet->destNetwork == myNet))
		return 1;
	return 0;
}

static void nukeOneRoute()
{
	uint8_t x;
	uint8_t y = 0;
	int8_t prio = 127;

	/* find lowest priority */
	for (x = 0; x < MAX_ROUTES; x++) {
		if (routingTable[x].priority < prio) {
			prio = routingTable[x].priority;
			y = x;
		}
	}

	routingTable[y].priority = -128;
}

static void parseRIP(struct ipx_header *packet, int8_t priority)
{
	unsigned short int operation;
	struct rip_entry route;
	uint16_t length = packet->length - IPX_HEADER_LENGTH - 2;

	/* we first read the operation
	   as we aren't a router, we don't respond to requests */
	encReadBuffer(2, (uint8_t *) & operation);
	byteSwaps((uint16_t *) & operation);
	if (operation != IPX_RIP_OP_RESPONSE)
		return;

	/* if the interface isn't configured, configure it */
	if (myNet == 0) {
		myNet = packet->destNetwork;
		PIOA->PIO_CODR = PIO_PA3;
	}

	/* add the routes */
	while (length > 0) {
		uint8_t x;

		encReadBuffer(sizeof(struct rip_entry), (uint8_t *) & route);
		length -= sizeof(struct rip_entry);
		/* find a blank entry */
		for (x = 0; x < MAX_ROUTES; x++) {
			if (routingTable[x].network == 0)
				break;
		}

		if (x == MAX_ROUTES) {
			/* no free entries */
			for (x = 0; x < MAX_ROUTES; x++) {
				if (routingTable[x].priority < priority)
					break;
			}
		}

		if (x == MAX_ROUTES)
			/* still no free entry? */
			continue;

		routingTable[x].network = route.network;
		memcpy(routingTable[x].node, packet->srcNode, 6);
		routingTable[x].priority = priority;
	}
}

static void parseSAP(struct ipx_header *packet)
{
	unsigned short int operation;
	unsigned short int stype;


	encReadBuffer(2, (uint8_t *) & operation);
	byteSwaps((uint16_t *) & operation);

	encReadBuffer(2, (uint8_t *) & stype);
	byteSwaps((uint16_t *) & stype);

	if ((operation == IPX_SAP_OP_RESPONSE) || (operation == IPX_SAP_OP_GNS_RESPONSE)) {
		/* if the interface isn't configured, configure it */
		if (myNet == 0) {
			myNet = packet->destNetwork;
			PIOA->PIO_CODR = PIO_PA3;
		}
		return;
	}

	if (myNet == 0) {
		return;
	}

	if ((operation == IPX_SAP_OP_GNS_REQUEST) || (operation == IPX_SAP_OP_REQUEST)) {
		if ((stype == vConfiguration->sapType) || (stype == IPX_SAP_GENERAL_RQ)) {
			if (operation == IPX_SAP_OP_GNS_REQUEST)
				operation = IPX_SAP_OP_GNS_RESPONSE;
			else
				operation = IPX_SAP_OP_RESPONSE;
			sendSAP(operation, packet->srcNode, packet->srcPort);
		}
	}
}

void ipxInitialize(const IPXNode node)
{
	uint8_t i;

	encInit(node);
	for (i = 0; i < 6; i++)
		myNode[i] = node[i];

	/* 0 means uninitialised */
	myNet = 0;

	/* clear routing table */
	for (i = 0; i < MAX_ROUTES; i++) {
		routingTable[i].network = 0;
		routingTable[i].priority = -128;
	}
}

static void padPacket(uint16_t length)
{
	uint16_t pad_length;

	if (length + ETH_HEADER_LEN + IPX_HEADER_LENGTH >= ETH_PACKET_MIN)
		return;
	else
		pad_length =
		    ETH_PACKET_MIN - (length + ETH_HEADER_LEN +
				      IPX_HEADER_LENGTH);

	if (pad_length + junk_off <= JUNKLENGTH) {
		encWriteBuffer(pad_length, junk + junk_off);
		junk_off += pad_length;
	} else {
		encWriteBuffer(JUNKLENGTH - junk_off, junk + junk_off);
		pad_length -= JUNKLENGTH - junk_off;
		encWriteBuffer(pad_length, junk);
		junk_off = pad_length;
	}
}

int dispatchPacket()
{
	uint16_t length;
	union {
		struct eth_header eheader;
		struct ipx_header iheader;
	} buf;
	uint8_t result;

	length = encPacketReceive((uint8_t *) & buf);
	if (length <= 0)
		/* enc28j60 is already cleaned up here */
		return 0;

	byteSwaps(&buf.eheader.type);

	if ((buf.eheader.type != ETHTYPE_IPX) || (length < IPX_HEADER_LENGTH)) {
		/* not IPX or truncated. Flush the packet */
		encPacketReceiveFinish();
		return 1;
	}

	/* get the IPX header */
	encReadBuffer(IPX_HEADER_LENGTH, (uint8_t *) & buf);

	byteSwaps(&buf.iheader.length);
	byteSwaps(&buf.iheader.destPort);
	byteSwaps(&buf.iheader.srcPort);

	/* check, if the packet is correct and not truncated */
	if ((length < buf.iheader.length) || (buf.iheader.chksum != IPX_CHKSUM)) {
		encPacketReceiveFinish();
		return 1;
	}

	/* is this packet for me? */
	result = isMeOrBroadcast(&buf.iheader);
	switch (result) {
	case 1:
		/* unicast to me */
		if ((buf.iheader.type == IPX_RIP_PTYPE)
		    && (buf.iheader.destPort == IPX_RIP_PORT))
			/* RIP-Packet */
			parseRIP(&buf.iheader, 2);
		if (pktgen_state_back.rip_requested) {
			/* rip was previously requested */
			pktgen_state_back.rip_requested = 0;
			memcpy(&pktgen_state, &pktgen_state_back, sizeof(struct state));
		}
		else {
			/* dispatch this somehow to the user code */
			handleAlchemyPacket(&buf.iheader);
		}
		break;
	case 2:
		/* broadcast */
		if ((buf.iheader.type == IPX_RIP_PTYPE)
		    && (buf.iheader.destPort == IPX_RIP_PORT))
			/* RIP-Packet */
			parseRIP(&buf.iheader, -1);
		else if ((buf.iheader.type == IPX_SAP_PTYPE)
			 && (buf.iheader.destPort == IPX_SAP_PORT))
			/* SAP-Packet */
			parseSAP(&buf.iheader);
	}

	/* Flush the buffer and set it to the next packet */
	encPacketReceiveFinish();
	return 1;
}

int createPacket(IPXNet toNet, const IPXNode toNode, IPXPort toPort,
		 IPXPort fromPort, uint8_t type, uint16_t length)
{
	/* save parameters to internal state */
	pktgen_state.toNet = toNet;
	memcpy(pktgen_state.toNode, toNode, 6);
	pktgen_state.toPort = toPort;
	pktgen_state.fromPort = fromPort;
	pktgen_state.type = type;
	pktgen_state.length = length;

	return createPacket2(toNet, toNode, toPort, fromPort, type, length);
}

int recreatePacket()
{
	return createPacket2(pktgen_state.toNet, pktgen_state.toNode, pktgen_state.toPort,
		pktgen_state.fromPort, pktgen_state.type, pktgen_state.length);
}

static int createPacket2(IPXNet toNet, const IPXNode toNode, IPXPort toPort,
		 IPXPort fromPort, uint8_t type, uint16_t length)
{
	union {
		struct eth_header eheader;
		struct ipx_header iheader;
	} buf;
	int x;

	/* do we need a gateway? */
	if (toNet == myNet) {
		/* no routing necessary */
		memcpy(buf.eheader.destNode, toNode, 6);
		pktgen_state.rip_requested = 0;
	} else {
		/* find the route */
		for (x = 0; x < MAX_ROUTES; x++) {
			if (routingTable[x].network == toNet)
				break;
		}

		if (x == MAX_ROUTES) {
			/* no route to host? Ask! */
			nukeOneRoute();
			/* first save the internal state */
			memcpy(&pktgen_state_back, &pktgen_state,
			       sizeof(struct state));
			requestRIP(toNet);
			pktgen_state_back.rip_requested = 1;
			return 0;
		} else {
			/* change priorities */
			routingTable[x].priority += 3;
			if (routingTable[x].priority > 125)
				routingTable[x].priority = 124;
			memcpy(buf.eheader.destNode, routingTable[x].node, 6);
		}
	}

	memcpy(buf.eheader.srcNode, myNode, 6);
	buf.eheader.type = 0x3781;	/* Byteorder! */
	if (length + ETH_HEADER_LEN + IPX_HEADER_LENGTH < ETH_PACKET_MIN)
		encPacketBegin(ETH_PACKET_MIN);	/* the packet must be padded */
	else
		encPacketBegin(length + ETH_HEADER_LEN + IPX_HEADER_LENGTH);
	encWriteBuffer(ETH_HEADER_LEN, (uint8_t *) & buf);

	remaining = length;
	length += IPX_HEADER_LENGTH;
	byteSwaps(&length);
	buf.iheader.chksum = IPX_CHKSUM;
	buf.iheader.length = length;
	buf.iheader.hops = 0;
	buf.iheader.type = type;
	buf.iheader.destNetwork = toNet;
	memcpy(buf.iheader.destNode, toNode, 6);
	buf.iheader.destPort = toPort;
	buf.iheader.srcNetwork = myNet;
	memcpy(buf.iheader.srcNode, myNode, 6);
	buf.iheader.srcPort = fromPort;
	byteSwaps(&buf.iheader.srcPort);
	byteSwaps(&buf.iheader.destPort);
	encWriteBuffer(IPX_HEADER_LENGTH, (uint8_t *) & buf);
	return 1;
}

void sendPacket()
{
	sendPacket2(pktgen_state.length);
}

static void sendPacket2(uint16_t length)
{
	padPacket(length);
	encPacketSend();
}

void requestRIP(IPXNet network)
{
	uint16_t operation;
	struct rip_entry request;

	SEGGER_RTT_printf(0, "RIP requested\n");
	createPacket2(myNet, broadcastNode, IPX_RIP_PORT, IPX_RIP_PORT,
		     IPX_RIP_PTYPE, 10);
	operation = IPX_RIP_OP_REQUEST;
	request.network = network;
	request.hops = 0;
	request.ticks = 0;
	byteSwaps(&operation);
	encWriteBuffer(2, (uint8_t *) & operation);
	encWriteBuffer(sizeof(struct rip_entry), (uint8_t *) & request);
	sendPacket2(10);
}

int haveRoute()
{
	return !pktgen_state.rip_requested;
}

static void sendSAP(uint16_t operation, const IPXNode destination, IPXPort port)
{
	struct sap_entry response;
	int n;

	createPacket2(myNet, destination, port, IPX_SAP_PORT,
			IPX_SAP_PTYPE, sizeof(struct sap_entry) + 2);

	response.hops = 0;
	response.network = myNet;
	memcpy(response.node, myNode, sizeof(IPXNode));
	memcpy(response.ser_name, vConfiguration->name, 48);
	n = strlen(response.ser_name);
	memset(response.ser_name + n, 0, 48 - n);
	response.port = vConfiguration->port;
	byteSwaps(&response.port);
	response.ser_type = vConfiguration->sapType;
	byteSwaps(&response.ser_type);
	byteSwaps(&operation);
	encWriteBuffer(2, (uint8_t *)&operation);
	encWriteBuffer(sizeof(struct sap_entry), (unsigned char*)&response);
	sendPacket();
}

void broadcastSAP()
{
	sendSAP(IPX_SAP_OP_RESPONSE, broadcastNode, IPX_SAP_PORT);
}

int isConfigured()
{
	return myNet != 0;
}

void configureNet(IPXNet netnum)
{
	myNet = netnum;
	PIOA->PIO_CODR = PIO_PA3;
}
