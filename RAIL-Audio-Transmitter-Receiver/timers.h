#ifndef TIMERS_H_
#define TIMERS_H_

#include "em_timer.h"

#define ADC_SAMPLING_FREQ 14000

#define DAC_SAMPLING_FREQ ADC_SAMPLING_FREQ

#define RX_EXPIRATION_TIME 0.2 // Seconds

void init_ADC_Timer(void);
void init_VDAC_Timer(void);
void init_rxExp_Timer(void);

#endif /* TIMERS_H_ */
