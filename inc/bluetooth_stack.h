#pragma once

#include <stdint.h>

#include "hardware.h"
#include "hci.h"
#include "gap.h"

#define MAX_PACKET_SIZE 256

typedef enum {
    BT_SUCCESS = 0,
    BT_ERROR_TIMEOUT,
    BT_ERROR_INVALID_PARAM,
    BT_ERROR_NOT_SUPPORTED,
    BT_ERROR_HARDWARE,
} BluetoothError;

typedef struct {
    uint8_t addr[6];
} BluetoothAddress;

void bluetooth_stack_init(void);
