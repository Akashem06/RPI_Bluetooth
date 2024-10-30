#pragma once

#include <stdint.h>
#include <stdbool.h>
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
} HCIError;

typedef struct {
    union {
        struct {
            uint16_t command            : 10;
            uint16_t group              :  6;
        };
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

typedef struct  {
    uint8_t hci_version;
    uint16_t hci_revision;
    uint8_t lmp_version;
    uint16_t manufacturer;
    uint16_t lmp_subversion;
} BCM4345C0Info;

/**
 * @brief
 * @return
 */
HCIError HCI_init(void);

/**
 * @brief
 * @return
 */
HCIError HCI_reset(void);

/**
 * @brief
 * @return
 */
int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief
 * @return
 */
HCIError HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data);

/**
 * @brief
 * @return
 */
HCIError HCI_send_command(HCICommand *cmd);

/**
 * @brief
 * @return
 */
HCIError HCI_send_async_data(HCIAsyncData *data);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, 
                                    Adv_Type adv_type, Adv_OwnAddressType own_address_type,
                                    Adv_DirectAddressType direct_address_type, uint8_t *direct_address,
                                    Adv_ChannelMap adv_channel_map, Adv_FilterPolicy adv_filter_policy);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len);


/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_advertising_enable(bool enable);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_scan_parameters(Scan_Type scan_type, uint16_t scan_interval, uint16_t scan_window,
                                   Scan_OwnAddressType own_address_type, Scan_FilterPolicy scanning_filter_policy);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_scan_enable(bool enable, bool filter_duplicates);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_create_connection(uint16_t scan_interval_ms, uint16_t scan_window_ms,
                                   Conn_InitiatorFilterPolicy filter_policy,
                                   Conn_PeerAddressType peer_address_type, uint8_t *peer_address,
                                   Conn_OwnAddressType own_address_type,
                                   uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms,
                                   uint16_t conn_latency, uint16_t supervision_timeout_ms);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_connection_update(uint16_t connection_handle,
                                   uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms,
                                   uint16_t conn_latency, uint16_t supervision_timeout_ms);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_disconnect(uint16_t connection_handle, Conn_DisconnectReason reason);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_BLE_set_event_mask(uint8_t mask);

/**
 * @brief
 * @details
 * @return
 */
HCIError HCI_set_local_name(const char* name);

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
 */
void HCI_handle_connection_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief
 */
void HCI_handle_disconnection_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief
 */
void HCI_handle_BLE_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

/**
 * @brief
 */
void HCI_handle_BLE_connection_update_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

/**
 * @brief
 */
void HCI_handle_BLE_enhanced_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

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
HCIError HCI_bcm4345_load_firmware();

/**
 * @brief
 * @return
 */
HCIError HCI_bcm4345_set_baudrate(uint32_t baudrate);

/**
 * @brief
 * @return
 */
HCIError HCI_set_bt_addr(uint8_t *bt_addr);

/**
 * @brief
 * @return
 */
HCIError HCI_get_bt_addr(uint8_t *bt_addr);


/**
 * @brief
 */
void HCI_handle_hw_rx(uint8_t byte);

/**
 * @brief
 * @return
 */
uint8_t HCI_buffer_space();

/**
 * @brief
 * @param
 * @param
 * @return
 */
HCIError HCI_parse_event(uint8_t *data, uint16_t length);


/**
 * @brief
 * @param
 * @return
 */
HCIError HCI_get_module_status(BCM4345C0Info *info);

/**
 * @brief
 * @param
 */
void HCI_print_module_status(BCM4345C0Info *info);