#ifndef APP_TASK_INIT_H_
#define APP_TASK_INIT_H_

#include "os.h"
#include "rail.h"
#define ARM_MATH_CM4 1
#define __FPU_PRESENT 1
#include "arm_math.h"

#define PROPRIETARY_APP_TASK_PRIO         6u
#define FILTER_TASK_PRIO         8u
#define PROPRIETARY_APP_TASK_STACK_SIZE   (1024 / sizeof(CPU_STK))
#define FILTER_TASK_STACK_SIZE   (1024 / sizeof(CPU_STK))

#define NUM_OF_COEFFS 42


#endif /* APP_TASK_INIT_H_ */
