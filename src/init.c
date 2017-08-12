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
 
init.c

Chip initialisation

Copyright (c) 2016 Ayron
 */

#include "cpu.h"
#include "ipx.h"
#include "spi.h"
#include <stdint.h>

extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _erodata;

extern void main();

void flashInit();

void _start()
{
	/* relocate ram functions and data segment */
	uint32_t*relocate_s;
	uint32_t*relocate_d;
	uint32_t*relocate_e;
	register uint32_t*cleanptr;

	for (cleanptr = (uint32_t*)0x20000000; cleanptr < (uint32_t*)0x20040000; *cleanptr++ = 0);
	for (cleanptr = (uint32_t*)0x20080000; cleanptr < (uint32_t*)0x20088000; *cleanptr++ = 0);

	relocate_s = &_erodata;
	relocate_d = &_srelocate;
	relocate_e = &_erelocate;
	for (; relocate_d < relocate_e; *relocate_d++ = *relocate_s++);

	/* vector table offset */
	SCB->VTOR = 0x00;
	flashInit();

	/* tell em how the clock works */
	PMC->CKGR_MOR = (PMC->CKGR_MOR & ~CKGR_MOR_MOSCXTBY) | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCXTST(64) | CKGR_MOR_KEY_VALUE;
	while (!(PMC->PMC_SR & PMC_SR_MOSCXTS));
	PMC->CKGR_MOR |= CKGR_MOR_MOSCSEL | CKGR_MOR_KEY_VALUE;
	while (!(PMC->PMC_SR & PMC_SR_MOSCSELS));

	/* PLL */
	PMC->CKGR_PLLAR = CKGR_PLLAR_ONE | CKGR_PLLAR_MULA(24) | CKGR_PLLAR_DIVA(12) | CKGR_PLLAR_PLLACOUNT(0x3f);
	while (!(PMC->PMC_SR & PMC_SR_LOCKA));

	/* switch to main clock */
	PMC->PMC_MCKR = PMC_MCKR_PRES_CLK_1 | PMC_MCKR_CSS_MAIN_CLK;
	while (!(PMC->PMC_SR & PMC_SR_MCKRDY));

	PIOA->PIO_PDR = SPI_MISO | SPI_MOSI | SPI_SCK;
	PIOA->PIO_PUER = SPI_MISO | SPI_MOSI | SPI_SCK | SPI_CS;
	PIOA->PIO_SODR = SPI_CS | PIO_PA3;
	PIOA->PIO_OER = SPI_CS | PIO_PA3;
	PIOA->PIO_OWER = PIO_PA3;
	/*PIOA->PIO_MDER = SPI_MISO | SPI_MOSI | SPI_SCK | SPI_CS;*/
	
	PIOA->PIO_FELLSR = PIO_PA2;
	PIOA->PIO_LSR = PIO_PA2;
	PIOA->PIO_AIMER = PIO_PA2;
	PMC->PMC_PCER0 = 1 << ID_PIOA;
	NVIC_EnableIRQ(PIOA_IRQn);
	
	PIO_SetOutput(PIOA, (1 << 4) | (1 << 6), 0xffffffffu, 0, 0);

	PMC->PMC_PCER0 = 1 << ID_SPI0;
	SPI0->SPI_MR = SPI_MR_MSTR | SPI_MR_PCS(0x00) | SPI_MR_MODFDIS;
	SPI0->SPI_CSR[0] = SPI_CSR_SCBR(1) | SPI_CSR_NCPHA;
	SPI0->SPI_CR = SPI_CR_SPIEN;

	PMC->PMC_PCER0 = 1 << ID_USART1;
	USART1->US_MR = US_MR_USART_MODE_SPI_MASTER | US_MR_USCLKS_MCK | US_MR_CHRL_8_BIT | US_MR_CPHA | US_MR_CLKO;
	USART1->US_BRGR = 2;
	USART1->US_CR = US_CR_RXEN | US_CR_TXEN;

	PMC->PMC_PCER0 = 1 << ID_TC0;
	TC_Configure(TC0, 0, TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVSEL_UP_RC | TC_CMR_WAVE);
	TC_SetRC(TC0, 0, 360000);
	TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
	NVIC_EnableIRQ(TC0_IRQn);

	/* Initialisation done. Turn off the yellow LED */

	PIOB->PIO_OER = 0x08000000;
	PIOB->PIO_CODR = 0x08000000;

	main();
}

__attribute__((section(".ramfunc")))
__attribute__((noinline))
void flashInit()
{
	EFC0->EEFC_FMR = EEFC_FMR_FWS(5); /* 0x00000500; */
	EFC1->EEFC_FMR = EEFC_FMR_FWS(5); /* 0x00000500; */
}
