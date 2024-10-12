#pragma once

#include "bluetooth_stack.h"

void hw_init(void);

void hw_uart_send_byte(uint8_t byte);
uint8_t hw_uart_receive_byte(void);
bool hw_uart_data_available(void);

void hw_delay_ms(uint32_t ms);
uint32_t hw_get_time_ms(void);