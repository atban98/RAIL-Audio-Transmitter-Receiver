#ifndef LDMA_H_
#define LDMA_H_

#include "em_ldma.h"
#include "em_timer.h"

#define ADC_LDMA_CHANNEL 0
#define DAC_LDMA_CHANNEL 3
#define ADC_BUFFER_SIZE 256
#define DAC_BUFFER_SIZE ADC_BUFFER_SIZE
#define LDMA_ADC_IRQ_FLAG 0x1
#define LDMA_DAC_IRQ_FLAG 0x8

void init_VDAC_Ldma(void);
void init_ADC_Ldma(void);

#endif /* LDMA_H_ */
