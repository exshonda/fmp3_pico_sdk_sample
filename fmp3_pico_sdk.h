#ifndef _MULTICORE_PROJ_H_
#define _MULTICORE_PROJ_H_

#include <kernel.h>

#define TASK_PRIORITY	10
#define	STACK_SIZE		4096

extern void	task1(EXINF exinf);
extern void	task2(EXINF exinf);

#endif