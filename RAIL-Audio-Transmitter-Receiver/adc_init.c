#include "adc_init.h"

// Initialize ADC
// ADC trigger source: PRS channel 0 (WTIMER0)
// Resolution: 12 bits
void init_ADC(void)
{
    ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
    ADC_InitSingle_TypeDef initSingle = ADC_INITSINGLE_DEFAULT;
    CMU_ClockEnable(cmuClock_ADC0, true);

    init.prescale = ADC_PrescaleCalc(ADCFREQ, 0);

    initSingle.posSel = adcPosSelAPORT2XCH9;
    initSingle.reference = adcRefVDD;
    initSingle.acqTime = adcAcqTime8;

    // Enable PRS trigger and select channel 0
    initSingle.prsEnable = true;
    initSingle.prsSel = (ADC_PRSSEL_TypeDef)PRS_CHANNEL;

    ADC_Init(ADC0, &init);
    ADC_InitSingle(ADC0, &initSingle);

    // Set single data valid level (DVL)
    ADC0->SINGLECTRLX = (ADC0->SINGLECTRLX & ~_ADC_SINGLECTRLX_DVL_MASK) | (((ADC_DVL - 1) << _ADC_SINGLECTRLX_DVL_SHIFT) & _ADC_SINGLECTRLX_DVL_MASK);

    // Clear the Single FIFO
    ADC0->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;
}
