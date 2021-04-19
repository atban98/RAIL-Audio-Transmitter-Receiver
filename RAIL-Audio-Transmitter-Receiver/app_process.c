#include "ldma.h"
#include "app_process.h"
#include "app_task_init.h"


volatile bool sendPacket = false;

extern OS_SEM filterTaskSemaphore;
extern bool adcBufferReady;

extern volatile q15_t *adcBufferPtr;

extern q15_t filterBuffer[DAC_BUFFER_SIZE];

void app_process_action(RAIL_Handle_t rail_handle)
{
    RAIL_RxPacketHandle_t packet_handle;
    RAIL_RxPacketInfo_t packet_info;

    if (adcBufferReady && sendPacket)
    {
        RAIL_WriteTxFifo(rail_handle, adcBufferPtr, 2 * ADC_BUFFER_SIZE, false);

        RAIL_Status_t status = RAIL_StartTx(rail_handle, 0, RAIL_TX_OPTIONS_DEFAULT, NULL);
        if (status != RAIL_STATUS_NO_ERROR)
        {
            sl_led_toggle(&sl_led_led1);
        }
        adcBufferReady = false;
    }

    packet_handle = RAIL_GetRxPacketInfo(rail_handle, RAIL_RX_PACKET_HANDLE_OLDEST_COMPLETE, &packet_info);

    if (packet_handle != RAIL_RX_PACKET_HANDLE_INVALID)
    {
        RTOS_ERR err;
        RAIL_CopyRxPacket(filterBuffer, &packet_info);

        // Reset RX timer when a packet was received
        TIMER_CounterSet(TIMER1,0);

        // Filter the received buffer
        OSSemPend(&filterTaskSemaphore,0,OS_OPT_PEND_BLOCKING,NULL,&err);
        RAIL_ReleaseRxPacket(rail_handle, packet_handle);
    }
}

void sl_rail_util_on_event(RAIL_Handle_t rail_handle, RAIL_Events_t events)
{
    (void)rail_handle;
    if (events & RAIL_EVENTS_TX_COMPLETION)
    {
        if (events & RAIL_EVENT_TX_PACKET_SENT)
            sl_led_toggle(&sl_led_led0);
        else
            sl_led_toggle(&sl_led_led1);
    }
    if (events & RAIL_EVENTS_RX_COMPLETION)
    {
        if (events & RAIL_EVENT_RX_PACKET_RECEIVED)
        {
            RAIL_HoldRxPacket(rail_handle);
            sl_led_toggle(&sl_led_led0);
        }
        else
            sl_led_toggle(&sl_led_led1);
    }
}

void sl_button_on_change(const sl_button_t *handle)
{
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
    {
        sendPacket = !sendPacket;
        if (sendPacket)
        {
            // Reset and enable ADC timer before sending packets
            TIMER_CounterSet(WTIMER0, 0);
            TIMER_Enable(WTIMER0, true);
        }
    }
}
