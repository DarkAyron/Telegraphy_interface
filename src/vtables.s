/*                  ___====-_  _-====___
              _--^^^#####//      \\#####^^^--_
           _-^##########// (    ) \\##########^-_
          -############//  |\^^/|  \\############-
        _/############//   (@::@)   \\############\_
       /#############((     \\//     ))#############\
      -###############\\    (oo)    //###############-
     -#################\\  / VV \  //#################-
    -###################\\/      \//###################-
   _#/|##########/\######(   /\   )######/\##########|\#_
   |/ |#/\#/\#/\/  \#/\##\  |  |  /##/\#/  \/\#/\#/\#| \|
   `  |/  V  V  `   V  \#\| |  | |/#/  V   '  V  V  \|  '
      `   `  `      `   / | |  | | \   '      '  '   '
                       (  | |  | |  )
                      __\ | |  | | /__
                     (vvv(VVV)(VVV)vvv)
 
vtables.s

Vector tables for the ATSAM
This will be completed over time.

Copyright (c) 2016 Ayron
*/

.syntax unified
.thumb

.section .traps
.global traps
traps:
@	.word	0x20087fff	@ stack
	.word	0x20001fff
	.word	_start		@ the beginning of something BIG
	.word	unused		@ NMI
	.word	error		@ something somewhere went terribly wrong
	.word	error		@ MMU fault
	.word	error		@ Bus error
	.word	error		@ Usage fault
	.word	0x00		@ reserved
	.word	0x00		@ reserved
	.word	0x00		@ reserved
	.word	0x00		@ reserved
	.word	SVC_Handler	@ SVCall
	.word	0x00		@ reserved
	.word	0x00		@ reserved
	.word	PendSV_Handler	@ PendSV
	.word	tick		@ SysTick

	.word	unused		@ IRQ 0  SUPC
	.word	unused		@ IRQ 1  RSTC
	.word	unused		@ IRQ 2  RTC
	.word	unused		@ IRQ 3  RTT
	.word	unused		@ IRQ 4  WDT
	.word	unused		@ IRQ 5  PMC
	.word	unused		@ IRQ 6  EFC0
	.word	unused		@ IRQ 7  EFC1
	.word	unused		@ IRQ 8  UART
	.word	unused		@ IRQ 9  SMC
	.word	unused		@ IRQ 10
	.word	PIOA_Handler	@ IRQ 11 PIOA
	.word	unused		@ IRQ 12 PIOB
	.word	unused		@ IRQ 13 PIOC
	.word	unused		@ IRQ 14 PIOD
	.word	unused		@ IRQ 15
	.word	unused		@ IRQ 16
	.word	unused		@ IRQ 17 USART0
	.word	unused		@ IRQ 18 USART1
	.word	unused		@ IRQ 19 USART2
	.word	unused		@ IRQ 20 USART3
	.word	unused		@ IRQ 21 HSMCI
	.word	unused		@ IRQ 22 TWI0
	.word	unused		@ IRQ 23 TWI1
	.word	unused		@ IRQ 24 SPI0
	.word	unused		@ IRQ 25
	.word	unused		@ IRQ 26 SCC
	.word	dot		@ IRQ 27 TC0
	.word	unused		@ IRQ 28 TC1
	.word	unused		@ IRQ 29 TC2
	.word	unused		@ IRQ 30 TC3
	.word	unused		@ IRQ 31 TC4
	.word	unused		@ IRQ 32 TC5
	.word	unused		@ IRQ 33 TC6
	.word	unused		@ IRQ 34 TC7
	.word	unused		@ IRQ 35 TC8
	.word	unused		@ IRQ 36 PWM
	.word	unused		@ IRQ 37 ADC
	.word	unused		@ IRQ 38 DACC
	.word	unused		@ IRQ 39 DMAC
	.word	USB_Handler	@ IRQ 40 UOTGHS
	.word	unused		@ IRQ 41 TRNG
	.word	unused		@ IRQ 42 EMAC
	.word	unused		@ IRQ 43 CAN0
	.word	unused		@ IRQ 44 CAN1

@********************************************************************
.text
.thumb_func
unused:	bx	r14	@ unused trap. Just ignore it.

.thumb_func
error:	movw	r3, #0x1030
	movt	r3, #0x400e
	mov	r2, #0x08000000
	str	r2, [r3, #0]
.L1:	b	.L1
	nop
