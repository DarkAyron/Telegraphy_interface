/*
 * mtask.c
 *
 * Do rudimentary multitasking.
 * This scheduler implements cooperative multitasking.
 * Context switching occurs only on request.
 */

#include <string.h>
#include "cpu.h"
#include "SEGGER_SYSVIEW.h"
#include "SEGGER_RTT.h"
#include "mtask.h"

extern volatile uint32_t tickCounter;

enum coroutineState {
	STATE_RUNNING,
	STATE_WAITING,
	STATE_TERMINATED
};

struct stackFrame {
	int R4;
	int R5;
	int R6;
	int R7;
	int R8;
	int R9;
	int R10;
	int R11;
	int R0;
	int R1;
	int R2;
	int R3;
	int R12;
	void *LR;
	void *PC;
	int xPSR;
};

struct coroutine {
	enum coroutineState state;
	enum triggers trigger;
	void *stack;
	char name[16];
};

struct coroutine coroutines[15];
static struct {
	int cur;
	int next;
} coroutineNum;

static volatile uint32_t currentTick;
static volatile enum triggers currentTrigger;

#define SYS_SP 0x20010000 /* Stack pointer */
#define STACK_SIZE 0x400

/* bx lr with this value in lr causes what's known
 * as rti by other CPUs
 */
#define MAGIC_LINK 0xfffffff9

/* for SEGGER SystemView */
void cbSendTaskList(void)
{
	int n;
	SEGGER_SYSVIEW_TASKINFO info;

	for (n = 1; n < 15; n++) {
		if (coroutines[n].state != STATE_TERMINATED) {
			SEGGER_SYSVIEW_OnTaskCreate((unsigned)&coroutines[n]);
			info.TaskID = (unsigned)&coroutines[n];
			info.sName = coroutines[n].name;
			info.StackBase = (uint32_t) coroutines[n].state;
			SEGGER_SYSVIEW_SendTaskInfo(&info);
		}
	}
}

/* idle routine. Just wait until current tick has finished */
static void coroutine_idle(int unused)
{
	int n;
	while (1) {
		/* reset trigger flag */
		currentTrigger = TRIGGER_NONE;
		while (currentTick == tickCounter) {
			if (currentTrigger) {
				currentTrigger = TRIGGER_NONE;
				coroutine_yield(TRIGGER_NONE);
			}
			/*asm volatile ("wfi"); */
		}
		
		/* Tick is over, wake all coroutines */
		currentTick = tickCounter;
		for (n = 0; n < 15; n++) {
			if ((coroutines[n].state == STATE_WAITING) && (coroutines[n].trigger == TRIGGER_NONE))
			coroutines[n].state = STATE_RUNNING;
		}
		coroutine_yield(TRIGGER_NONE);
	}
}

/* Initialize the multitasking engine */
void mtask_init()
{
	int n;
	for (n = 0; n < 15; n++) {
		coroutines[n].state = STATE_TERMINATED;
		coroutines[n].stack = (void *)(SYS_SP - (n + 1) * 0x400);
		coroutines[n].trigger = TRIGGER_NONE;
	}
	coroutineNum.cur = 14;
	currentTick = 0;
	currentTrigger = TRIGGER_NONE;

	/* 
	 * set interrupt priorities.
	 * The lower the number, the higher the priority
	 */
	NVIC_SetPriority(SVCall_IRQn, 0x00);
	NVIC_SetPriority(PendSV_IRQn, 0x0f);
	NVIC_SetPriority(SysTick_IRQn, 0x0e);
	coroutine_invoke_later(coroutine_idle, 0, "Idler");

}

static int findNextCoroutine()
{
	int n;
	for (n = 0; n < 15; n++) {
		/* find next running coroutine */
		if (coroutines[n].state == STATE_RUNNING) {
			break;
		}
	}
	return n;
}

/* do a context switch */
__attribute__ ((naked))
void PendSV_Handler()
{
	register void *nextsp;
	register void *mySP;
	asm("push {r4 - r11}");
	/* save the magic link */
	asm("mov r10, lr");

	/* save current SP */
	asm volatile ("mov %r0, r13":"=l" (mySP));
	coroutines[coroutineNum.cur].stack = mySP;

	/* set stack pointer */
	nextsp = coroutines[coroutineNum.next].stack;
	asm volatile ("mov sp, %r0\n"::"l" (nextsp));

	coroutineNum.cur = coroutineNum.next;

	/* restore the magic link */
	asm("mov lr, r10");

	/* restore the rest and trigger rti */
	asm("pop {r4 - r11}");
	asm("bx lr");
}

static void coroutine_switchToNext()
{
	/* find the next coroutine */
	coroutineNum.next = findNextCoroutine();
	if (coroutineNum.next == 15) {
		coroutineNum.next = 0;
	}

	/* schedule a PendSV interrupt and wait until it happens */
	if (coroutineNum.next)
		SEGGER_SYSVIEW_OnTaskStartExec((unsigned)
					       &coroutines[coroutineNum.next]);
	else
		SEGGER_SYSVIEW_OnIdle();

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/* we don't need to save anything here,
 * because we are in a destroyed stack frame
 */
__attribute__ ((naked))
void SVC_Handler()
{
	register void *nextsp;
	/* save the magic link */
	asm volatile ("mov r10, lr");

	/* mark the slot as free */
	coroutines[coroutineNum.cur].state = STATE_TERMINATED;

	SEGGER_SYSVIEW_OnTaskStopExec();
	coroutine_switchToNext();
	/* reset the PendSV, as we are doing the switch here now */
	SCB->ICSR &= ~SCB_ICSR_PENDSVSET_Msk;

	coroutineNum.cur = coroutineNum.next;

	/* set stack pointer */
	nextsp = coroutines[coroutineNum.next].stack;
	asm volatile ("mov sp, %r0\n"::"l" (nextsp));

	/* restore the magic link */
	asm("mov lr, r10");

	/* restore the rest and trigger rti */
	asm("pop {r4 - r11}");
	asm("bx lr");
}

static void coroutine_end()
{
	/* use supervisor call to block other interrupts while terminating the coroutine */
	coroutine_abort();
	/* there's nothing more to do */
	while (1) ;
}

static void coroutine_start(void (*func) (int), int param, int n, const char *name)
{
	struct stackFrame *frame;
	SEGGER_SYSVIEW_TASKINFO info;

	coroutines[n].stack =
	    (void *)(SYS_SP - (n + 1) * STACK_SIZE - sizeof(struct stackFrame));
	coroutines[n].state = STATE_RUNNING;
	coroutines[n].trigger = TRIGGER_NONE;
	frame = (struct stackFrame *)coroutines[n].stack;
	frame->PC = (void *)func;
	frame->LR = (void *)coroutine_end;
	frame->R0 = param;
	frame->xPSR = 0x01000000;	/* Thumb bit must be set */
	if (n) {
		SEGGER_SYSVIEW_OnTaskCreate((unsigned)&coroutines[n]);
		memcpy(coroutines[n].name, name, strlen(name));
		info.TaskID = (unsigned)&coroutines[n];
		info.sName = coroutines[n].name;
		info.StackBase = (uint32_t) frame;
		SEGGER_SYSVIEW_SendTaskInfo(&info);
	}
}

/* yield the current coroutine to give the CPU to other stuff */
enum triggers coroutine_yield(enum triggers trigger)
{
	coroutines[coroutineNum.cur].state = STATE_WAITING;
	coroutines[coroutineNum.cur].trigger = trigger;
	if (coroutineNum.cur)
		SEGGER_SYSVIEW_OnTaskStopReady((unsigned)
					       &coroutines[coroutineNum.cur],
					       trigger);
	coroutine_switchToNext();
	return coroutines[coroutineNum.cur].trigger;
	/*asm volatile ("wfi"); */
}

/* set up a new coroutine-slot and invoke it eventually in the current or next tick */
int coroutine_invoke_later(void (*func) (int), int param, const char *name)
{
	int n;
	for (n = 0; n < 15; n++) {
		if (coroutines[n].state == STATE_TERMINATED)
			break;
	}
	if (n == 15)
		return -1;

	coroutine_start(func, param, n, name);
	return n;
}

/* set up a new coroutine-slot and invoke it immediately, suspending the current one */
int coroutine_invoke_urgent(void (*func) (), int param, const char *name)
{
	int result = coroutine_invoke_later(func, param, name);
	if (result >= 0) {
		coroutineNum.next = result;
		SEGGER_SYSVIEW_OnTaskStartExec((unsigned)
					       &coroutines[coroutineNum.next]);
		/* schedule a PendSV interrupt */
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	}

	return result;
}

/* abort the current coroutine and free the slot */
__attribute__ ((__always_inline__)) inline void coroutine_abort()
{
	asm volatile ("svc 0");
}

/* trigger an event */
void coroutine_trigger(enum triggers trigger)
{
	int n;
	for (n = 0; n < 15; n++) {
		if ((coroutines[n].state == STATE_WAITING) && (coroutines[n].trigger & trigger)) {
			coroutines[n].state = STATE_RUNNING;
			coroutines[n].trigger = trigger;
		}
	}
	currentTrigger = trigger;
}
