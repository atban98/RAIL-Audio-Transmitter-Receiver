#ifndef APP_INIT_H
#define APP_INIT_H

#include "rail.h"
#include "sl_rail_util_init.h"

#define BUFFER_LENGTH 1024
#define RX_BUFFER_LENGTH BUFFER_LENGTH
#define TX_BUFFER_LENGTH BUFFER_LENGTH

void app_init(void);

#endif // APP_INIT_H
