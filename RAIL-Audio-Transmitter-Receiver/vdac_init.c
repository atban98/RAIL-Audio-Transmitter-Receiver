#include "vdac_init.h"

// Initialize VDAC
// VDAC trigger source: TIMER0
// Resolution: 12 bits
void init_VDAC(void)
{
    EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
    EMU_DCDCInit(&dcdcInit);
    CMU_ClockEnable(cmuClock_VDAC0, true);

    VDAC_Init_TypeDef init = VDAC_INIT_DEFAULT;
    init.prescaler = VDAC_PrescaleCalc(1000000, false, 0);
    init.reference = vdacRefAvdd;
    VDAC_Init(VDAC0, &init);

    VDAC_InitChannel_TypeDef initChannel0 = VDAC_INITCHANNEL_DEFAULT;
    VDAC_InitChannel(VDAC0, &initChannel0, 0);

    VDAC0->OPA[0].TIMER &= ~(_VDAC_OPA_TIMER_SETTLETIME_MASK);
    VDAC_Enable(VDAC0, 0, true);
}
