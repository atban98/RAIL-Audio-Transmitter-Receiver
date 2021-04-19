#include "ldma.h"
#include "adc_init.h"
#include "vdac_init.h"
#include "arm_math.h"

q15_t adcBuffer1[ADC_BUFFER_SIZE];
q15_t adcBuffer2[ADC_BUFFER_SIZE];

q15_t dacBuffer1[DAC_BUFFER_SIZE];
q15_t dacBuffer2[DAC_BUFFER_SIZE];

volatile q15_t *adcBufferPtr = adcBuffer1;
volatile q15_t *dacBufferPtr = dacBuffer1;

// Indicates if an adcBuffer is ready to be sent
volatile bool adcBufferReady = false;

extern volatile bool sendPacket;

void LDMA_IRQHandler(void)
{

    uint32_t pending = LDMA_IntGet();

    if (pending == LDMA_ADC_IRQ_FLAG)
    {
        adcBufferReady = true;
        if (LDMA->CH[ADC_LDMA_CHANNEL].DST == adcBuffer1)
            adcBufferPtr = adcBuffer1;
        else
            adcBufferPtr = adcBuffer2;

        // Disable ADC timer on every second button push
        if (!sendPacket)
            TIMER_Enable(WTIMER0, false);
    }

    if (pending == LDMA_DAC_IRQ_FLAG)
    {
        if (LDMA->CH[DAC_LDMA_CHANNEL].SRC == dacBuffer1)
            dacBufferPtr = dacBuffer2;
        else
            dacBufferPtr = dacBuffer1;
    }

    // Clear interrupt flag
    LDMA_IntClear(pending);
}

// LDMA initialization for ADC
void init_ADC_Ldma(void)
{
    CMU_ClockEnable(cmuClock_LDMA, true);

    static LDMA_Descriptor_t descr[2];

    descr[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_WORD(
        &(ADC0->SINGLEDATA),
        adcBuffer1, // Destination1
        ADC_BUFFER_SIZE,
        1);

    descr[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_P2M_WORD(
        &(ADC0->SINGLEDATA),
        adcBuffer2, // Destination2
        ADC_BUFFER_SIZE,
        -1);

    LDMA_TransferCfg_t trans = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_ADC0_SINGLE);

    descr[0].xfer.blockSize = ADC_DVL - 1;
    descr[1].xfer.blockSize = ADC_DVL - 1;

    descr[0].xfer.size = ldmaCtrlSizeHalf;
    descr[1].xfer.size = ldmaCtrlSizeHalf;

    descr[0].xfer.doneIfs = true;
    descr[1].xfer.doneIfs = true;

    LDMA_StartTransfer(ADC_LDMA_CHANNEL, &trans, descr);
}

// LDMA initialization for VDAC
void init_VDAC_Ldma(void)
{
    static LDMA_Descriptor_t descr[2];
    descr[0] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        dacBuffer1, // Source1
        &VDAC0->CH0DATA,
        DAC_BUFFER_SIZE,
        1);
    descr[1] = (LDMA_Descriptor_t)LDMA_DESCRIPTOR_LINKREL_M2P_BYTE(
        dacBuffer2, // Source2
        &VDAC0->CH0DATA,
        DAC_BUFFER_SIZE,
        -1);
    descr[0].xfer.size = ldmaCtrlSizeHalf;
    descr[1].xfer.size = ldmaCtrlSizeHalf;

    descr[0].xfer.doneIfs = true;
    descr[1].xfer.doneIfs = true;

    LDMA_TransferCfg_t trans = (LDMA_TransferCfg_t)LDMA_TRANSFER_CFG_PERIPHERAL(ldmaPeripheralSignal_TIMER0_UFOF);

    LDMA_Init_t init = LDMA_INIT_DEFAULT;
    LDMA_Init(&init);

    LDMA_StartTransfer(DAC_LDMA_CHANNEL, &trans, descr);

    NVIC_ClearPendingIRQ(LDMA_IRQn);
    NVIC_EnableIRQ(LDMA_IRQn);
}
