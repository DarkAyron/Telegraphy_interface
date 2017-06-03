/*****************************************************************************
* vim:sw=8:ts=8:si:et
*
* Title        : Microchip ENC28J60 Ethernet Interface Driver
* Author        : Pascal Stang (c)2005
* Modified by Guido Socher
* Copyright: GPL V2
*
*This driver provides initialization and transmit/receive
*functions for the Microchip ENC28J60 10Mb Ethernet Controller and PHY.
*This chip is novel in that it is a full MAC+PHY interface all in a 28-pin
*chip, using an SPI interface to the host processor.
*
*
*****************************************************************************/

#ifndef ENC28J60_H
#define ENC28J60_H
#include <inttypes.h>

/* ENC28J60 Control Registers
 * Control register definitions are a combination of address,
 * bank number, and Ethernet/MAC/PHY indicator bits.
 * - Register address        (bits 0-4)
 * - Bank number        (bits 5-6)
 * - MAC/PHY indicator        (bit 7)
 */
#define ENC_ADDR_MASK	0x1f
#define ENC_BANK_MASK	0x60

/* Common registers */
#define ENC_EIE		0x1b
#define ENC_EIR		0x1c
#define ENC_ESTAT	0x1d
#define ENC_ECON2	0x1e
#define ENC_ECON1	0x1f

/* Bank 0 registers */
#define ENC_ERDPTL	0x00
#define ENC_ERDPTH	0x01
#define ENC_EWRPTL	0x02
#define ENC_EWRPTH	0x03
#define ENC_ETXSTL	0x04
#define ENC_ETXSTH	0x05
#define ENC_ETXNDL	0x06
#define ENC_ETXNDH	0x07
#define ENC_ERXSTL	0x08
#define ENC_ERXSTH	0x09
#define ENC_ERXNDL	0x0a
#define ENC_ERXNDH	0x0b
#define ENC_ERXRDPTL	0x0c
#define ENC_ERXRDPTH	0x0d
#define ENC_ERXWRPTL	0x0e
#define ENC_ERXWRPTH	0x0f
#define ENC_EDMASTL	0x10
#define ENC_EDMASTH	0x11
#define ENC_EDMANDL	0x12
#define ENC_EDMANDH	0x13
#define ENC_EDMADSTL	0x14
#define ENC_EDMADSTH	0x15
#define ENC_EDMACSL	0x16
#define ENC_EDMACSH	0x17

/* Bank 1 registers */
#define ENC_EHT0	0x20
#define ENC_EHT1	0x21
#define ENC_EHT2	0x22
#define ENC_EHT3	0x23
#define ENC_EHT4	0x24
#define ENC_EHT5	0x25
#define ENC_EHT6	0x26
#define ENC_EHT7	0x27
#define ENC_EPMM0	0x28
#define ENC_EPMM1	0x29
#define ENC_EPMM2	0x2a
#define ENC_EPMM3	0x2b
#define ENC_EPMM4	0x2c
#define ENC_EPMM5	0x2d
#define ENC_EPMM6	0x2e
#define ENC_EPMM7	0x2f
#define ENC_EPMCSL	0x30
#define ENC_EPMCSH	0x31
#define ENC_EPMOL	0x34
#define ENC_EPMOH	0x35
#define ENC_EWOLIE	0x36
#define ENC_EWOLIR	0x37
#define ENC_ERXFCON	0x38
#define ENC_EPKTCNT	0x39

/* Bank 2 registers */
#define ENC_MACON1	0x40
#define ENC_MACON2	0x41
#define ENC_MACON3	0x42
#define ENC_MACON4	0x43
#define ENC_MABBIPG	0x44
#define ENC_MAIPGL	0x46
#define ENC_MAIPGH	0x47
#define ENC_MACLCON1	0x48
#define ENC_MACLCON2	0x49
#define ENC_MAMXFLL	0x4a
#define ENC_MAMXFLH	0x4b
#define ENC_MAPHSUP	0x4d
#define ENC_MICON	0x51
#define ENC_MICMD	0x52
#define ENC_MIREGADR	0x54
#define ENC_MIWRL	0x56
#define ENC_MIWRH	0x57
#define ENC_MIRDL	0x58
#define ENC_MIRDH	0x59

/* Bank 3 registers */
#define ENC_MAADR1	0x60
#define ENC_MAADR0	0x61
#define ENC_MAADR3	0x62
#define ENC_MAADR2	0x63
#define ENC_MAADR5	0x64
#define ENC_MAADR4	0x65
#define ENC_EBSTSD	0x66
#define ENC_EBSTCON	0x67
#define ENC_EBSTCSL	0x68
#define ENC_EBSTCSH	0x69
#define ENC_MISTAT	0x6a
#define ENC_EREVID	0x72
#define ENC_ECOCON	0x75
#define ENC_EFLOCON	0x77
#define ENC_EPAUSL	0x78
#define ENC_EPAUSH	0x79

/* PHY registers */
#define PHCON1		0x00
#define PHSTAT1		0x01
#define PHHID1		0x02
#define PHHID2		0x03
#define PHCON2		0x10
#define PHSTAT2		0x11
#define PHIE		0x12
#define PHIR		0x13
#define PHLCON		0x14

/* ERXFCON Register Bit Definitions */
#define ERXFCON_UCEN	0x80
#define ERXFCON_ANDOR	0x40
#define ERXFCON_CRCEN	0x20
#define ERXFCON_PMEN	0x10
#define ERXFCON_MPEN	0x08
#define ERXFCON_HTEN	0x04
#define ERXFCON_MCEN	0x02
#define ERXFCON_BCEN	0x01

/* EIE Register Bit Definitions */
#define EIE_INTIE	0x80
#define EIE_PKTIE	0x40
#define EIE_DMAIE	0x20
#define EIE_LINKIE	0x10
#define EIE_TXIE	0x08
#define EIE_WOLIE	0x04
#define EIE_TXERIE	0x02
#define EIE_RXERIE	0x01

/* EIR Register Bit Definitions */
#define EIR_PKTIF	0x40
#define EIR_DMAIF	0x20
#define EIR_LINKIF	0x10
#define EIR_TXIF	0x08
#define EIR_WOLIF	0x04
#define EIR_TXERIF	0x02
#define EIR_RXERIF	0x01

/* ESTAT Register Bit Definitions */
#define ESTAT_INT        0x80
#define ESTAT_LATECOL    0x10
#define ESTAT_RXBUSY     0x04
#define ESTAT_TXABRT     0x02
#define ESTAT_CLKRDY     0x01
// ENC28J60 ECON2 Register Bit Definitions
#define ECON2_AUTOINC    0x80
#define ECON2_PKTDEC     0x40
#define ECON2_PWRSV      0x20
#define ECON2_VRPS       0x08
// ENC28J60 ECON1 Register Bit Definitions
#define ECON1_TXRST      0x80
#define ECON1_RXRST      0x40
#define ECON1_DMAST      0x20
#define ECON1_CSUMEN     0x10
#define ECON1_TXRTS      0x08
#define ECON1_RXEN       0x04
#define ECON1_BSEL1      0x02
#define ECON1_BSEL0      0x01
// ENC28J60 ENC_MACON1 Register Bit Definitions
#define MACON1_LOOPBK    0x10
#define MACON1_TXPAUS    0x08
#define MACON1_RXPAUS    0x04
#define MACON1_PASSALL   0x02
#define MACON1_MARXEN    0x01
// ENC28J60 MACON2 Register Bit Definitions
#define MACON2_MARST     0x80
#define MACON2_RNDRST    0x40
#define MACON2_MARXRST   0x08
#define MACON2_RFUNRST   0x04
#define MACON2_MATXRST   0x02
#define MACON2_TFUNRST   0x01
// ENC28J60 ENC_MACON3 Register Bit Definitions
#define MACON3_PADCFG2   0x80
#define MACON3_PADCFG1   0x40
#define MACON3_PADCFG0   0x20
#define MACON3_TXCRCEN   0x10
#define MACON3_PHDRLEN   0x08
#define MACON3_HFRMLEN   0x04
#define MACON3_FRMLNEN   0x02
#define MACON3_FULDPX    0x01
// ENC28J60 ENC_MICMD Register Bit Definitions
#define MICMD_MIISCAN    0x02
#define MICMD_MIIRD      0x01
// ENC28J60 ENC_MISTAT Register Bit Definitions
#define MISTAT_NVALID    0x04
#define MISTAT_SCAN      0x02
#define MISTAT_BUSY      0x01
// ENC28J60 PHY PHCON1 Register Bit Definitions
#define PHCON1_PRST      0x8000
#define PHCON1_PLOOPBK   0x4000
#define PHCON1_PPWRSV    0x0800
#define PHCON1_PDPXMD    0x0100
// ENC28J60 PHY PHSTAT1 Register Bit Definitions
#define PHSTAT1_PFDPX    0x1000
#define PHSTAT1_PHDPX    0x0800
#define PHSTAT1_LLSTAT   0x0004
#define PHSTAT1_JBSTAT   0x0002
// ENC28J60 PHY PHCON2 Register Bit Definitions
#define PHCON2_FRCLINK   0x4000
#define PHCON2_TXDIS     0x2000
#define PHCON2_JABBER    0x0400
#define PHCON2_HDLDIS    0x0100

// ENC28J60 Packet Control Byte Bit Definitions
#define PKTCTRL_PHUGEEN  0x08
#define PKTCTRL_PPADEN   0x04
#define PKTCTRL_PCRCEN   0x02
#define PKTCTRL_POVERRIDE 0x01

// SPI operation codes
#define ENC_READ_CTRL_REG       0x00
#define ENC_READ_BUF_MEM        0x3A
#define ENC_WRITE_CTRL_REG      0x40
#define ENC_WRITE_BUF_MEM       0x7A
#define ENC_BIT_FIELD_SET       0x80
#define ENC_BIT_FIELD_CLR       0xA0
#define ENC_SOFT_RESET          0xFF


// The RXSTART_INIT should be zero. See Rev. B4 Silicon Errata
// buffer boundaries applied to internal 8K ram
// the entire available packet buffer space is allocated
//
// start with recbuf at 0/
#define RXSTART_INIT     0x0
// receive buffer end
#define RXSTOP_INIT      (0x1FFF-0x0600-1)
// start TX buffer at 0x1FFF-0x0600, pace for one full ethernet frame (~1500 bytes)
#define TXSTART_INIT     (0x1FFF-0x0600)
// stp TX buffer at end of mem
#define TXSTOP_INIT      0x1FFF
//
// max frame length which the conroller will accept:
#define        MAX_FRAMELEN        1522        // maximum ethernet frame length

// functions
extern uint8_t encReadOp(uint8_t op, uint8_t address);
extern void encWriteOp(uint8_t op, uint8_t address, uint8_t data);
extern void encReadBuffer(uint16_t len, uint8_t* data);
extern void encWriteBuffer(uint16_t len, const uint8_t* data);
extern void encSetBank(uint8_t address);
extern uint8_t encRead(uint8_t address);
extern void encWrite(uint8_t address, uint8_t data);
extern void encPhyWrite(uint8_t address, uint16_t data);
extern void encInit(const uint8_t* macaddr);
extern void encPacketBegin(uint16_t len);
extern void encPacketSend();
extern uint8_t encHasPacket();
extern uint16_t encPacketReceive(uint8_t* packet);
extern void encPacketReceiveFinish();
extern uint8_t encGetrev(void);

#endif
//@}
