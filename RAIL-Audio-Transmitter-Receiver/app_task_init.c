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

           0.000000000000000000,
           -0.000030733384559862,
           0.000129484279144729,
           -0.000310332521128357,
           0.000592460495031738,
           -0.000998644955413067,
           0.001552892620716367,
           -0.002277432173886599,
           0.003189366525274162,
           -0.004297349679967388,
           0.005598674310693276,
           -0.007077135353370884,
           0.008701972506360003,
           -0.010428096023421115,
           0.012197675410737732,
           -0.013943032486843875,
           0.015590643417015184,
           -0.017065933616309627,
           0.018298458155921519,
           -0.019227008837504868,
           0.019804183576481910,
           0.979999775470058010,
           0.019804183576481914,
           -0.019227008837504875,
           0.018298458155921522,
           -0.017065933616309623,
           0.015590643417015184,
           -0.013943032486843879,
           0.012197675410737739,
           -0.010428096023421114,
           0.008701972506360008,
           -0.007077135353370890,
           0.005598674310693276,
           -0.004297349679967389,
           0.003189366525274159,
           -0.002277432173886603,
           0.001552892620716368,
           -0.000998644955413065,
           0.000592460495031740,
           -0.000310332521128357,
           0.000129484279144730,
           -0.000030733384559862,
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
