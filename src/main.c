/*
 * Copyright (C) 2016 ayron
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   main.c
 * Author: ayron
 *
 * Created on 07 May 2016, 15:58
 */

#include "cpu.h"
#include "ipx.h"
#include "spi.h"
#include "enc28j60.h"
#include "alchemy.h"
#include "mtask.h"
#include "memory.h"
#include "SEGGER_SYSVIEW.h"
#include "delay.h"
#include "dragonusb/usb_core.h"
#include "code.h"
#include "telegraph.h"

static volatile int stalled;
uint32_t tickCounter = 0;
uint32_t SystemCoreClock = 12000000;
volatile uint32_t doSend = 0;

void stall();
void mainLoop(int unused);

/*
 * 
 */
int main()
{
	/* Watchdog off */
	WDT->WDT_MR = WDT_MR_WDDIS;
	SysTick_Config(600000);

	PIOA->PIO_PER = (1 << 21);
	PIOA->PIO_OER = (1 << 21);
	PIOA->PIO_CODR = (1 << 21);

	PIOA->PIO_PER = (1 << 22);
	PIOA->PIO_SODR = (1 << 22);
	PIOA->PIO_OER = (1 << 22);

	PIOA->PIO_PDR = 0x02;
	PIOA->PIO_ABSR = 0x02;
	PMC->PMC_PCK[0] = PMC_PCK_CSS_PLLA_CLK;
	PMC->PMC_SCER = PMC_SCER_PCK0;
	
	memory_init();
	memory_readUID();
	SEGGER_SYSVIEW_Conf();
	mtask_init();
	USB_Init();
	coroutine_invoke_urgent(mainLoop, 0, "mainLoop");
	return 0;
}

void tick()
{
	tickCounter++;
}

void mainLoop(int unused)
{
	ipxInitialize(myNode);
	alchemyInit();
	PIOA->PIO_ISR;
	PIOA->PIO_IER = PIO_PA2;
	encoderInit(MORSE_CODE_CONTINENTAL, 25, 0);
	encoderSetText("dl7ayr");
	TC_Start(TC0, 0);
	while(1) {
		alchemyTick();
		coroutine_yield(TRIGGER_NONE);
	}
}

void handlePacket(int unused)
{
	/* Receive Operation */
	while(dispatchPacket());
	/* Clear interrupt bits */
	encWriteOp(ENC_WRITE_CTRL_REG, ENC_EIR, 0x00);
	PIOA->PIO_ISR;
	PIOA->PIO_IER = PIO_PA2;
}

void stall()
{
	stalled = 1;
	while(stalled) {
		asm volatile("wfi");
	}
}

void PIOA_Handler()
{
	SEGGER_SYSVIEW_RecordEnterISR();
	uint32_t status = PIOA->PIO_ISR;
	if (status & PIO_PA2) {
		PIOA->PIO_IDR = PIO_PA2;
		coroutine_invoke_later(handlePacket, 0, "handlePacket");
		/*handlePacket(0);*/
	}
	SEGGER_SYSVIEW_RecordExitISR();
}
