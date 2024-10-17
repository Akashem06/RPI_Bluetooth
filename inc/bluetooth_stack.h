#pragma once

#include <stdint.h>

#include "hardware.h"
#include "hci.h"
#include "gap.h"

#define MAX_PACKET_SIZE 256

typedef struct {
    uint8_t addr[6];
} BluetoothAddress;

void bluetooth_stack_init(void);
