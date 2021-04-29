#include "ldma.h"
#include "timers.h"
#include "app_task_init.h"
#include "app_init.h"
#include "adc_init.h"
#include "vdac_init.h"
#define ARM_MATH_CM4 1
#define __FPU_PRESENT 1
#include "arm_math.h"

static void proprietary_app_task(void *p_arg);
static void filter_task(void *p_arg);

void app_process_action(RAIL_Handle_t rail_handle);
void filter_task_loop(void);

static CPU_STK filter_task_stack[FILTER_TASK_STACK_SIZE];
static CPU_STK proprietary_app_task_stack[PROPRIETARY_APP_TASK_STACK_SIZE];
static OS_TCB  proprietary_app_task_TCB;
static OS_TCB  filter_task_TCB;

OS_SEM filterTaskSemaphore;

extern volatile q15_t *dacBufferPtr;

static q15_t firStateQ15[ADC_BUFFER_SIZE + NUM_OF_COEFFS];
const q15_t  firCoeffsQ15[NUM_OF_COEFFS];
q15_t filterBuffer[DAC_BUFFER_SIZE];
arm_fir_instance_q15 S;

const float32_t  firCoeffs_float[NUM_OF_COEFFS] = {
    -0.000747512085353054, 0.000328918563662717,
    0.000368315042629899, -0.00103725499694955,
    0.00124284406781585, -0.000625768725329023,
    -0.000764633465235840, 0.00225398387703400,
    -0.00274056558886538, 0.00137194018550753,
    0.00164688850688745, -0.00474009609390106,
    0.00561410736591202, -0.00273761175470954,
    -0.00320551561991269, 0.00901950400829763,
    -0.0104719484804781, 0.00502126934663080,
    0.00580106284639431, -0.0161648528369124,
    0.0186630612098737,  -0.00894055596446597,
    -0.0103760339529903, 0.0292385260511120,
    -0.0344253255909700, 0.0170060807099908,
    0.0206707236351174, -0.0624277220036095,
    0.0817621939963961, -0.0480729899349089,
    -0.0808601908889898,  0.588329158570319,
    0.588329158570319, -0.0808601908889898,
    -0.0480729899349089, 0.0817621939963961,
    -0.0624277220036095, 0.0206707236351174,
    0.0170060807099908, -0.0344253255909700,
    0.0292385260511120, -0.0103760339529903,
    -0.00894055596446597, 0.0186630612098737,
    -0.0161648528369124, 0.00580106284639431,
    0.00502126934663080, -0.0104719484804781,
    0.00901950400829763, -0.00320551561991269,
    -0.00273761175470954, 0.00561410736591202,
    -0.00474009609390106, 0.00164688850688745,
    0.00137194018550753, -0.00274056558886538,
    0.00225398387703400, -0.000764633465235840,
    -0.000625768725329023, 0.00124284406781585,
    -0.00103725499694955, 0.000368315042629899,
    0.000328918563662717, -0.000747512085353054,
};


void app_task_init(void)
{
  RTOS_ERR err;
  init_VDAC_Ldma();
  init_ADC_Ldma();
  init_VDAC();
  init_ADC();
  init_VDAC_Timer();
  init_ADC_Timer();
  init_rxExp_Timer();

  // Create the Proprietary Application task
  OSTaskCreate(&proprietary_app_task_TCB,
               "Proprietary App Task",
               proprietary_app_task,
               0u,
               PROPRIETARY_APP_TASK_PRIO,
               &proprietary_app_task_stack[0u],
               (PROPRIETARY_APP_TASK_STACK_SIZE / 10u),
               PROPRIETARY_APP_TASK_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  // Create the filter task
  OSTaskCreate(&filter_task_TCB,
               "Filter Task",
               filter_task,
               0u,
               FILTER_TASK_PRIO,
               &filter_task_stack[0u],
               (FILTER_TASK_STACK_SIZE / 10u),
               FILTER_TASK_STACK_SIZE,
               0u,
               0u,
               0u,
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               &err);

  OSSemCreate(&filterTaskSemaphore,"Filter task semaphore",0,&err);
}

static void proprietary_app_task(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);

  app_init();
  RAIL_Handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
  while (DEF_TRUE)
  {
    app_process_action(rail_handle);
  }
}

static void filter_task(void *p_arg)
{
  PP_UNUSED_PARAM(p_arg);
  // Init Q15 filter
  arm_float_to_q15(firCoeffs_float,firCoeffsQ15,NUM_OF_COEFFS);
  arm_fir_init_q15(&S, NUM_OF_COEFFS, (q15_t *)&firCoeffsQ15[0], &firStateQ15[0], DAC_BUFFER_SIZE);
  while(DEF_TRUE)
  {
     RTOS_ERR err;
     filter_task_loop();
     OSSemPost(&filterTaskSemaphore,OS_OPT_POST_1,&err);
  }
}

void filter_task_loop(void)
{
  arm_fir_fast_q15(&S, filterBuffer , dacBufferPtr , DAC_BUFFER_SIZE);
}
