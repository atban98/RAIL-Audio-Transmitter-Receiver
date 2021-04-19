#include "app_init.h"

static uint8_t tx_buffer[TX_BUFFER_LENGTH];
static uint8_t rx_buffer[RX_BUFFER_LENGTH];

void app_init(void)
{
    uint16_t rx_init_buffer_size = RX_BUFFER_LENGTH;
    RAIL_Handle_t rail_handle = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);

    RAIL_SetTxFifo(rail_handle, tx_buffer, 0, BUFFER_LENGTH);
    RAIL_SetRxFifo(rail_handle, rx_buffer, &rx_init_buffer_size);

    // Remove packet info from packets
    RAIL_ConfigRxOptions(rail_handle, RAIL_RX_OPTION_REMOVE_APPENDED_INFO, RAIL_RX_OPTION_REMOVE_APPENDED_INFO);

    RAIL_StartRx(rail_handle, 0, NULL);
}
