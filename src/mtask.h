
#ifndef MTASK_H_
#define MTASK_H_

enum triggers {
	TRIGGER_NONE = 0,
	TRIGGER_USB = 1,
	TRIGGER_DATA = 2,
	TRIGGER_TIMEOUT = 4
};

void mtask_init(void);
enum triggers coroutine_yield(enum triggers trigger);
void coroutine_abort(void);
void coroutine_trigger(enum triggers trigger);

int coroutine_invoke_later(void (*func) (int), int param, const char *name);
int coroutine_invoke_urgent(void (*func) (int), int param, const char *name);

#endif				/* MTASK_H_ */
