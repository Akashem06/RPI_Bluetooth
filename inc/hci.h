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
    uint8_t  parameter_length;
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

/**
 * @brief
 * @return
 */
HCI_Error HCI_init(void);

/**
 * @brief
 * @return
 */
HCI_Error HCI_reset(void);

/**
 * @brief
 * @return
 */
int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief
 * @return
 */
HCI_Error HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data);

/**
 * @brief
 * @return
 */
HCI_Error HCI_send_command(HCICommand *cmd);

/**
 * @brief
 * @return
 */
HCI_Error HCI_send_async_data(HCIAsyncData *data);

/**
 * @brief
 * @details
 * @return
 */
HCI_Error HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, 
                                    Adv_Type adv_type, Adv_OwnAddressType own_address_type,
                                    Adv_DirectAddressType direct_address_type, uint8_t *direct_address,
                                    Adv_ChannelMap adv_channel_map, Adv_FilterPolicy adv_filter_policy);

/**
 * @brief
 * @details
 * @return
 */
HCI_Error HCI_BLE_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len);

/**
 * @brief
 * @details
 * @return
 */
int HCI_BLE_set_scan_parameters(uint8_t scan_type, uint16_t scan_interval, uint16_t scan_window,
                             uint8_t own_address_type, uint8_t scanning_filter_policy);

/**
 * @brief
 * @details
 * @return
 */
int HCI_BLE_create_connection(uint16_t scan_interval, uint16_t scan_window, uint8_t *peer_address);


/**
 * @brief
 */
void HCI_handle_async_data(HCIAsyncData *data);

/**
 * @brief
 */
void HCI_handle_error(uint8_t error_code);

/**
 * @brief
 */
void HCI_handle_event(HCIEvent *event);

/**
 * @brief
 */
void HCI_handle_command_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief
 */
void HCI_handle_command_status_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief
 * @return
 */
HCIState HCI_get_state(void);

/**
 * @brief
 */
void HCI_set_state(HCIState new_state);

/**
 * @brief
 * @return
 */
HCI_Error HCI_bcm4345_load_firmware();

/**
 * @brief
 * @return
 */
HCI_Error HCI_bcm4345_set_baudrate(uint32_t baudrate);

/**
 * @brief
 * @return
 */
HCI_Error HCI_set_bt_addr(uint8_t *bt_addr);