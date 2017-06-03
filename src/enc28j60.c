/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher, Ayron
 * Copyright: GPL V2
 *
 * Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
 * For AVRlib See http://www.procyonengineering.com/
 * Used with explicit permission of Pascal Stang.
 *
 * Title: Microchip ENC28J60 Ethernet Interface Driver
 * Chip type           : Cortex-M3
 *********************************************/
#include "cpu.h"
#include "spi.h"
#include "enc28j60.h"
#include "delay.h"

static uint8_t Enc28j60Bank;
static uint16_t NextPacketPtr;

#ifndef ETH_HEADER_LEN
#define ETH_HEADER_LEN 14
#endif

uint8_t encReadOp(uint8_t op, uint8_t address)
{
	uint8_t result;
	SPI_SELECT_ENC;
	/* issue read command */
	result = spiReadWrite(op | (address & ENC_ADDR_MASK));
	/* read data */
	result = spiReadWrite(0x00);
	/* do dummy read if needed (for mac and mii, see datasheet page 29) */
	if (address & 0x80) {
		result = spiReadWrite(0x00);
	}
	/* release CS */
	SPI_DESELECT;
	return result;
}

void encWriteOp(uint8_t op, uint8_t address, uint8_t data)
{
	SPI_SELECT_ENC;
	/* issue write command */
	spiReadWrite(op | (address & ENC_ADDR_MASK));
	/* write data */
	spiReadWrite(data);
	SPI_DESELECT;
}

void encReadBuffer(uint16_t len, uint8_t* data)
{
	SPI_SELECT_ENC;
	/* issue read command */
	spiReadWrite(ENC_READ_BUF_MEM);
	while (len) {
		len--;
		/* read data */
		*data = spiReadWrite(0x00);
		data++;
	}
	SPI_DESELECT;
}

void encWriteBuffer(uint16_t len, const uint8_t* data)
{
	SPI_SELECT_ENC;
	/* issue write command */
	spiReadWrite(ENC_WRITE_BUF_MEM);
	while (len) {
		len--;
		/* write data */
		spiReadWrite(*data);
		data++;
	}
	SPI_DESELECT;
}

void encSetBank(uint8_t address)
{
	/* set the bank (if needed) */
	if ((address & ENC_BANK_MASK) != Enc28j60Bank) {
		/* set the bank */
		encWriteOp(ENC_BIT_FIELD_CLR, ENC_ECON1,
				(ECON1_BSEL1 | ECON1_BSEL0));
		encWriteOp(ENC_BIT_FIELD_SET, ENC_ECON1,
				(address & ENC_BANK_MASK) >> 5);
		Enc28j60Bank = (address & ENC_BANK_MASK);
	}
}

uint8_t encRead(uint8_t address)
{
	/* set the bank */
	encSetBank(address);
	/* do the read */
	return encReadOp(ENC_READ_CTRL_REG, address);
}

void encWrite(uint8_t address, uint8_t data)
{
	/* set the bank */
	encSetBank(address);
	/* do the write */
	encWriteOp(ENC_WRITE_CTRL_REG, address, data);
}

void encPhyWrite(uint8_t address, uint16_t data)
{
	/* set the PHY register address */
	encWrite(ENC_MIREGADR, address);
	/* write the PHY data */
	encWrite(ENC_MIWRL, data);
	encWrite(ENC_MIWRH, data >> 8);
	/* wait until the PHY write completes */
	while (encRead(ENC_MISTAT) & MISTAT_BUSY) {
		delay_us(15);
	}
}

void encInit(const uint8_t* macaddr)
{
	/* perform system reset */
	encWriteOp(ENC_SOFT_RESET, 0, ENC_SOFT_RESET);
	delay_ms(50);
	/* check CLKRDY bit to see if reset is complete
	 * The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
	 * while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
	 * do bank 0 stuff
	 * initialize receive buffer
	 * 16-bit transfers, must write low byte first
	 * set receive buffer start address */
	NextPacketPtr = RXSTART_INIT;
	/* Rx start */
	encWrite(ENC_ERXSTL, RXSTART_INIT & 0xFF);
	encWrite(ENC_ERXSTH, RXSTART_INIT >> 8);
	/* set receive pointer address */
	encWrite(ENC_ERXRDPTL, RXSTART_INIT & 0xFF);
	encWrite(ENC_ERXRDPTH, RXSTART_INIT >> 8);
	/* RX end */
	encWrite(ENC_ERXNDL, RXSTOP_INIT & 0xFF);
	encWrite(ENC_ERXNDH, RXSTOP_INIT >> 8);
	/* TX start */
	encWrite(ENC_ETXSTL, TXSTART_INIT & 0xFF);
	encWrite(ENC_ETXSTH, TXSTART_INIT >> 8);
	/* TX end */
	encWrite(ENC_ETXNDL, TXSTOP_INIT & 0xFF);
	encWrite(ENC_ETXNDH, TXSTOP_INIT >> 8);

	/* do bank 1 stuff, packet filter */
	encWrite(ENC_ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN);

	/*
	 * do bank 2 stuff
	 * enable MAC receive
	 */
	encWrite(ENC_MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
	/* bring MAC out of reset */
	encWrite(ENC_MACON2, 0x00);
	/* enable CRC operations */
	encWriteOp(ENC_BIT_FIELD_SET, ENC_MACON3,
			MACON3_TXCRCEN | MACON3_FRMLNEN);
	/* set inter-frame gap (non-back-to-back) */
	encWrite(ENC_MAIPGL, 0x12);
	encWrite(ENC_MAIPGH, 0x0C);
	/* set inter-frame gap (back-to-back) */
	encWrite(ENC_MABBIPG, 0x12);
	/* Set the maximum packet size which the controller will accept
	 * Do not send packets longer than MAX_FRAMELEN:
	 */
	encWrite(ENC_MAMXFLL, MAX_FRAMELEN & 0xFF);
	encWrite(ENC_MAMXFLH, MAX_FRAMELEN >> 8);
	/* do bank 3 stuff */
	/* write MAC address */
	/* NOTE: MAC address in ENC28J60 is byte-backward */
	encWrite(ENC_MAADR5, macaddr[0]);
	encWrite(ENC_MAADR4, macaddr[1]);
	encWrite(ENC_MAADR3, macaddr[2]);
	encWrite(ENC_MAADR2, macaddr[3]);
	encWrite(ENC_MAADR1, macaddr[4]);
	encWrite(ENC_MAADR0, macaddr[5]);

	/* LED configuration */
	encPhyWrite(PHLCON, 0x3472);

	/* no loopback of transmitted frames */
	encPhyWrite(PHCON2, PHCON2_HDLDIS);
	/* switch to bank 0 */
	encSetBank(ENC_ECON1);
	/* enable interrupts */
	encWriteOp(ENC_BIT_FIELD_SET, ENC_EIE, EIE_INTIE | EIE_PKTIE | EIE_TXIE);
	/* enable packet reception */
	encWriteOp(ENC_BIT_FIELD_SET, ENC_ECON1, ECON1_RXEN);
}

// read the revision of the chip:
uint8_t encGetrev(void)
{
	return (encRead(ENC_EREVID));
}

/* create a new packet */
void encPacketBegin(uint16_t len)
{
	/* Set the write pointer to start of transmit buffer area */
	encWrite(ENC_EWRPTL, TXSTART_INIT & 0xFF);
	encWrite(ENC_EWRPTH, TXSTART_INIT >> 8);
	/* Set the TXND pointer to correspond to the packet size given */
	encWrite(ENC_ETXNDL, (TXSTART_INIT + len) & 0xFF);
	encWrite(ENC_ETXNDH, (TXSTART_INIT + len) >> 8);
	/* write per-packet control byte (0x00 means use macon3 settings) */
	encWriteOp(ENC_WRITE_BUF_MEM, 0, 0x00);
}

/* send the packet */
void encPacketSend()
{
	/* send the contents of the transmit buffer onto the network */
	encWriteOp(ENC_BIT_FIELD_SET, ENC_ECON1, ECON1_TXRTS);
	/* Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12. */
	if ((encRead(ENC_EIR) & EIR_TXERIF)) {
		encWriteOp(ENC_BIT_FIELD_CLR, ENC_ECON1, ECON1_TXRTS);
	}
}

uint8_t encHasPacket()
{
	if (encRead(ENC_EPKTCNT) == 0) {
		return (0);
	} else {
		return (1);
	}
}

/* Gets a packet header from the network receive buffer, if one is available.
 *      packet  Pointer where packet data should be stored.
 * Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
 */
uint16_t encPacketReceive(uint8_t* packet)
{
	uint16_t rxstat;
	uint16_t len;
	// check if a packet has been received and buffered
	//if( !(encRead(EIR) & EIR_PKTIF) ){
	// The above does not work. See Rev. B4 Silicon Errata point 6.
	if (encRead(ENC_EPKTCNT) == 0) {
		return (0);
	}

	// Set the read pointer to the start of the received packet
	encWrite(ENC_ERDPTL, (NextPacketPtr));
	encWrite(ENC_ERDPTH, (NextPacketPtr) >> 8);
	// read the next packet pointer
	NextPacketPtr = encReadOp(ENC_READ_BUF_MEM, 0);
	NextPacketPtr |= encReadOp(ENC_READ_BUF_MEM, 0) << 8;
	// read the packet length (see datasheet page 43)
	len = encReadOp(ENC_READ_BUF_MEM, 0);
	len |= encReadOp(ENC_READ_BUF_MEM, 0) << 8;
	len -= 4; //remove the CRC count
	// read the receive status (see datasheet page 43)
	rxstat = encReadOp(ENC_READ_BUF_MEM, 0);
	rxstat |= encReadOp(ENC_READ_BUF_MEM, 0) << 8;
	// check CRC and symbol errors (see datasheet page 44, table 7-3):
	// The ENC_ERXFCON.CRCEN is set by default. Normally we should not
	// need to check this.
	if ((rxstat & 0x80) == 0) {
		// invalid
		len = 0;
		// clean up
		encPacketReceiveFinish();
	} else {
		// copy the packet from the receive buffer
		encReadBuffer(ETH_HEADER_LEN, packet);
	}
	return (len - ETH_HEADER_LEN);
}

// finish read of packet data and flush the rest from memory
void encPacketReceiveFinish()
{
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	encWrite(ENC_ERXRDPTL, (NextPacketPtr));
	encWrite(ENC_ERXRDPTH, (NextPacketPtr) >> 8);
	// decrement the packet counter indicate we are done with this packet
	encWriteOp(ENC_BIT_FIELD_SET, ENC_ECON2, ECON2_PKTDEC);
}
