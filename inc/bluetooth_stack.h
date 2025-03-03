#pragma once

#include <stdint.h>

#include "gap.h"
#include "hardware_bl.h"
#include "hci.h"

typedef struct {
  uint8_t addr[6];
} BluetoothAddress;

void bluetooth_stack_init(void);
