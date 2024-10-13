#pragma once

#include <stdint.h>

#include "hci_defs.h"

typedef struct {
    typedef struct {
        uint16_t command : 10;
        uint16_t group   :  6;
    } op_code;
    uint8_t  paramter_length;
    uint8_t  *parameters;
} HCICommand;

typedef struct {

} HCIExtendedCommand;

typedef struct {

} HCIAsyncData;

typedef struct {
    
} HCIEvent;
