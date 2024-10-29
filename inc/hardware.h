#pragma once

#include "bluetooth_stack.h"
#include <stdbool.h>

void hw_init(void);

void hw_transmit_byte(uint8_t byte);
void hw_transmit_buffer(uint8_t *buffer, uint16_t buffer_length);
uint8_t hw_receive_byte(void);
void hw_delay_ms(uint32_t ms);
uint64_t hw_get_time_ms(void);