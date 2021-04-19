#ifndef ADC_INIT_H_
#define ADC_INIT_H_

#include "em_adc.h"
#include "em_cmu.h"
#include "em_prs.h"

#define ADC_DVL 4
#define ADCFREQ 16000000
#define PRS_CHANNEL 0

void init_ADC(void);

#endif /* ADC_INIT_H_ */
