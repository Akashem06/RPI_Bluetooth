#pragma once

#include <stdint.h>

#include "hci_defs.h"

typedef enum {
    HCI_STATE_IDLE,
    HCI_STATE_WAITING_RESPONSE,
    HCI_STATE_ON,
    HCI_STATE_ADVERTISING,
    HCI_STATE_SCANNING,
    HCI_STATE_CONNECTING,
    HCI_STATE_CONNECTED,
    HCI_STATE_DISCONNECTED,
    HCI_STATE_SLEEP,
    HCI_STATE_ERROR
} HCIState;

typedef enum {
    HCI_ERROR_SUCCESS,
    HCI_ERROR_INVALID_OPCODE,
    HCI_ERROR_INVALID_EVENT,
    HCI_ERROR_UNKNOWN_COMMAND,
    HCI_ERROR_INVALID_PARAMETERS,
    HCI_ERROR_COMMAND_TIMEOUT,
    HCI_ERROR_BUFFER_OVERFLOW,
    HCI_ERROR_UNSUPPORTED_GROUP,
    HCI_ERROR_MEMORY_ALLOCATION_FAILED,
    HCI_ERROR_INTERNAL_ERROR,
    HCI_ERROR_BUSY,
    HCI_ERROR_UNSUPPORTED_VERSION,
    HCI_ERROR_UNKNOWN_PACKET_TYPE
} HCI_Error;

typedef struct {
    uint16_t command            : 10;
    uint16_t group              :  6;
} HCI_CommandOpCode;

typedef struct {
    union {
        HCI_CommandOpCode bit;
        uint16_t          raw;
    } op_code;
    uint8_t  paramter_length;
    uint8_t  *parameters;
} HCICommand;

typedef struct {

} HCIExtendedCommand;

typedef struct {
    uint16_t connection_handle  : 12;
    uint16_t pb_flag            : 2;
    uint16_t bc_flag            : 2;
    uint16_t data_total_length;
    uint8_t  *data;
} HCIAsyncData;

typedef struct {
    uint8_t  event_code;
    uint8_t  parameter_total_length;
    uint8_t  *parameters;
} HCIEvent;

typedef struct {
    uint8_t *data;
    uint32_t size;
} BCM4345_FirmwareChunk;

typedef struct {
    const char *path;
    BCM4345_FirmwareChunk *chunks;
    uint32_t chunk_count;
} BCM4345_FirmwareData;

HCI_Error HCI_init(void);
HCI_Error HCI_reset(void);

HCI_Error HCI_send_command(HCICommand *cmd);
void HCI_handle_event(HCIEvent *event);

HCI_Error HCI_send_async_data(HCIAsyncData *data);
void HCI_handle_async_data(HCIAsyncData *data);

int HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, uint8_t adv_type);
int HCI_BLE_set_advertising_data(uint8_t *adv_data);
int HCI_BLE_set_scan_parameters(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window);
int HCI_BLE_create_connection(uint16_t scan_interval, uint16_t scan_window, uint8_t *peer_address);

int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size);
HCI_Error HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data);
void HCI_handle_error(uint8_t error_code);

void HCI_handle_command_complete_event(uint8_t *parameters, uint8_t parameter_length);
void HCI_handle_command_status_event(uint8_t *parameters, uint8_t parameter_length);

void HCI_handle_BLE_meta_event(HCI_SubEventCode *sub_event);
void HCI_handle_disconnection_complete_event(uint8_t status, uint16_t connection_handle, uint8_t reason);

HCIState HCI_get_state(void);
void HCI_set_state(HCIState new_state);

int HCI_bcm4345_load_firmware();
int HCI_send_firmware_chunk(BCM4345_FirmwareChunk *chunk);
BCM4345_FirmwareData *HCI_read_firmware_file(const char *firmware_path);

int HCI_bcm4345_set_baudrate(uint32_t baudrate);
int HCI_bcm4345_set_bt_addr(BCM4345_FirmwareChunk *chunk);
