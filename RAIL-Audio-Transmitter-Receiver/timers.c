#include "timers.h"
#include "adc_init.h"
#include "ldma.h"
#include "arm_math.h"
extern q15_t dacBuffer1[DAC_BUFFER_SIZE];
extern q15_t dacBuffer2[DAC_BUFFER_SIZE];

void TIMER1_IRQHandler(void)
{
  // Acknowledge the interrupt
  uint32_t flags = TIMER_IntGet(TIMER1);
  TIMER_IntClear(TIMER1, flags);

  // If no packet was received for RX_EXPIRATION_TIME seconds, the dacBuffers are set to 0
  memset(dacBuffer1,0,2*DAC_BUFFER_SIZE);
  memset(dacBuffer2,0,2*DAC_BUFFER_SIZE);
}

// Initialize WTIMER0 for ADC
void init_ADC_Timer(void)
{
    CMU_ClockEnable(cmuClock_WTIMER0, true);

    TIMER_InitCC_TypeDef wtimerCCInit = TIMER_INITCC_DEFAULT;
    TIMER_InitCC(WTIMER0, 0, &wtimerCCInit);

    uint32_t topValue = CMU_ClockFreqGet(cmuClock_HFPER) / ADC_SAMPLING_FREQ;
    TIMER_TopSet(WTIMER0, topValue);
    CMU_ClockEnable(cmuClock_PRS, true);

    PRS_SourceAsyncSignalSet(PRS_CHANNEL, PRS_CH_CTRL_SOURCESEL_WTIMER0, PRS_CH_CTRL_SIGSEL_WTIMER0OF);
    TIMER_Init_TypeDef wtimerInit = TIMER_INIT_DEFAULT;
    TIMER_Init(WTIMER0, &wtimerInit);
    TIMER_Enable(WTIMER0, false);
}

// Initialize TIMER0 for VDAC
void init_VDAC_Timer(void)
{
    CMU_ClockEnable(cmuClock_TIMER0, true);

    TIMER_Init_TypeDef init = TIMER_INIT_DEFAULT;
    init.enable = false;
    TIMER_Init(TIMER0, &init);

    uint32_t topValue = CMU_ClockFreqGet(cmuClock_HFPER) / DAC_SAMPLING_FREQ;
    TIMER_TopBufSet(TIMER0, topValue);

    TIMER0->CTRL |= TIMER_CTRL_DMACLRACT;

    TIMER_Enable(TIMER0, true);
}

// Initialize TIMER1 for RX expiration time
void init_rxExp_Timer(void)
{
    CMU_ClockEnable(cmuClock_TIMER1, true);

    TIMER_InitCC_TypeDef timerCCInit = TIMER_INITCC_DEFAULT;
    timerCCInit.mode = timerCCModeCompare;
    TIMER_InitCC(TIMER1, 0, &timerCCInit);

    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;

    timerInit.enable = false;
    timerInit.prescale = timerPrescale1024;

    TIMER_Init(TIMER1, &timerInit);

    uint32_t compareValue = CMU_ClockFreqGet(cmuClock_TIMER1) * RX_EXPIRATION_TIME/ (1 << timerPrescale1024);

    TIMER_CompareSet(TIMER1, 0, compareValue);

    TIMER_IntEnable(TIMER1, TIMER_IEN_CC0);
    NVIC_EnableIRQ(TIMER1_IRQn);


    TIMER_Enable(TIMER1, true);
}
