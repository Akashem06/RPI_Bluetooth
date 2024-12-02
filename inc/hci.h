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
 * @brief   Initialize the Host Controller Interface (HCI) layer
 * @return  HCIError Indicates the result of the initialization process
 */
HCIError HCI_init(void);

/**
 * @brief   Reset the HCI layer to its default state
 * @return  HCIError Indicates the success or failure of the reset operation
 * @details Clears all internal states, cancels ongoing operations, and returns HCI to a known initial condition
 */
HCIError HCI_reset(void);

/**
 * @brief   Encode an HCI packet into a buffer
 * @param   packet_type Type of HCI packet to encode
 * @param   packet_data Pointer to the data (Command/Async etc.) to be encoded
 * @param   buffer Output buffer where the encoded packet will be stored
 * @param   buffer_size Size of the output buffer
 * @return  int Number of bytes written to the buffer, or an error code
 * @details Converts an HCI packet into a byte stream suitable for transmission
 */
int HCI_encode_packet(HCIPacket packet_type, void *packet_data, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief   Decode an HCI packet from a buffer
 * @param   buffer Input buffer containing the packet data
 * @param   buffer_size Size of the input buffer
 * @param   packet_type Output parameter to store the decoded packet type
 * @param   packet_data Output buffer to store the decoded packet data
 * @return  HCIError Indicates the success or failure of the decoding process
 * @details Converts a byte stream back into an HCI packet structure
 */
HCIError HCI_decode_packet(uint8_t *buffer, uint16_t buffer_size, uint8_t *packet_type, void *packet_data);

/**
 * @brief   Send an HCI command to the Bluetooth controller
 * @param   cmd Pointer to the HCI command to be sent
 * @return  HCIError Indicates the success or failure of sending the command
 * @details Transmits a specific HCI command to the Bluetooth hardware
 */
HCIError HCI_send_command(HCICommand *cmd);

/**
 * @brief   Send asynchronous data through the HCI layer
 * @param   data Pointer to the asynchronous data to be sent
 * @return  HCIError Indicates the success or failure of sending the data
 * @details Allows transmission of data that does not require immediate response
 */
HCIError HCI_send_async_data(HCIAsyncData *data);

/**
 * @brief   Configure Bluetooth Low Energy (BLE) advertising parameters
 * @param   adv_interval_min Minimum advertising interval
 * @param   adv_interval_max Maximum advertising interval
 * @param   adv_type Type of advertising
 * @param   own_address_type Address type for the local device
 * @param   direct_address_type Address type for direct addressing
 * @param   direct_address Pointer to direct address (if applicable)
 * @param   adv_channel_map Channels used for advertising
 * @param   adv_filter_policy Filtering policy for advertisements
 * @return  HCIError Indicates the success or failure of setting advertising parameters
 * @details Configures the detailed parameters for BLE advertising
 */
HCIError HCI_BLE_set_advertising_param(uint16_t adv_interval_min, uint16_t adv_interval_max, 
                                    Adv_Type adv_type, Adv_OwnAddressType own_address_type,
                                    Adv_DirectAddressType direct_address_type, uint8_t *direct_address,
                                    Adv_ChannelMap adv_channel_map, Adv_FilterPolicy adv_filter_policy);

/**
 * @brief   Set the advertising data for BLE advertisements
 * @param   adv_data Pointer to the advertising data buffer
 * @param   adv_data_len Length of the advertising data
 * @return  HCIError Indicates the success or failure of setting advertising data
 * @details Configures the payload that will be broadcasted during BLE advertising
 */
HCIError HCI_BLE_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len);


/**
 * @brief   Enable or disable BLE advertising
 * @param   enable Boolean flag to turn advertising on or off
 * @return  HCIError Indicates the success or failure of changing advertising state
 * @details Starts or stops the BLE advertisement process
 */
HCIError HCI_BLE_set_advertising_enable(bool enable);

/**
 * @brief   Configure BLE scanning parameters
 * @param   scan_type Type of scan to perform
 * @param   scan_interval Interval between scan cycles
 * @param   scan_window Duration of each scan cycle
 * @param   own_address_type Address type for the local device
 * @param   scanning_filter_policy Filtering policy for scanning
 * @return  HCIError Indicates the success or failure of setting scan parameters
 * @details Sets up the detailed configuration for BLE device scanning
 */
HCIError HCI_BLE_set_scan_parameters(Scan_Type scan_type, uint16_t scan_interval, uint16_t scan_window,
                                   Scan_OwnAddressType own_address_type, Scan_FilterPolicy scanning_filter_policy);

/**
 * @brief   Enable or disable BLE scanning
 * @param   enable Boolean flag to turn scanning on or off
 * @param   filter_duplicates Flag to filter out duplicate scan responses
 * @return  HCIError Indicates the success or failure of changing scanning state
 * @details Starts or stops the BLE scanning process
 */
HCIError HCI_BLE_set_scan_enable(bool enable, bool filter_duplicates);

/**
 * @brief   Initiate a BLE connection
 * @param   scan_interval_ms Interval between scan cycles in milliseconds
 * @param   scan_window_ms Duration of each scan cycle in milliseconds
 * @param   filter_policy Connection initiator filter policy
 * @param   peer_address_type Address type of the target device
 * @param   peer_address Pointer to the target device's address
 * @param   own_address_type Local device's address type
 * @param   conn_interval_min_ms Minimum connection interval
 * @param   conn_interval_max_ms Maximum connection interval
 * @param   conn_latency Connection latency parameter
 * @param   supervision_timeout_ms Supervision timeout in milliseconds
 * @return  HCIError Indicates the success or failure of creating the connection
 * @details Establishes a new BLE connection with specified parameters
 */
HCIError HCI_BLE_create_connection(uint16_t scan_interval_ms, uint16_t scan_window_ms,
                                   Conn_InitiatorFilterPolicy filter_policy,
                                   Conn_PeerAddressType peer_address_type, uint8_t *peer_address,
                                   Conn_OwnAddressType own_address_type,
                                   uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms,
                                   uint16_t conn_latency, uint16_t supervision_timeout_ms);

/**
 * @brief   Update parameters of an existing BLE connection
 * @param   connection_handle Handle identifying the connection to update
 * @param   conn_interval_min_ms Minimum new connection interval
 * @param   conn_interval_max_ms Maximum new connection interval
 * @param   conn_latency New connection latency
 * @param   supervision_timeout_ms New supervision timeout
 * @return  HCIError Indicates the success or failure of updating the connection
 * @details Modifies the parameters of an active BLE connection
 */
HCIError HCI_BLE_connection_update(uint16_t connection_handle,
                                   uint16_t conn_interval_min_ms, uint16_t conn_interval_max_ms,
                                   uint16_t conn_latency, uint16_t supervision_timeout_ms);

/**
 * @brief   Terminate an active Bluetooth connection
 * @param   connection_handle Handle identifying the connection to terminate
 * @param   reason Reason code for disconnection
 * @return  HCIError Indicates the success or failure of disconnection
 * @details Closes an existing Bluetooth connection
 */
HCIError HCI_disconnect(uint16_t connection_handle, Conn_DisconnectReason reason);

/**
 * @brief   Set the BLE event mask to control which events are reported
 * @param   mask Bitmask defining which events to enable
 * @return  HCIError Indicates the success or failure of setting the event mask
 * @details Configures which Bluetooth Low Energy events will generate notifications
 */
HCIError HCI_BLE_set_event_mask(uint8_t mask);

/**
 * @brief   Set the local Bluetooth device name
 * @param   name Null-terminated string containing the device name
 * @return  HCIError Indicates the success or failure of setting the name
 * @details Configures the human-readable name for the local Bluetooth device
 */
HCIError HCI_set_local_name(const char* name);

/**
 * @brief   Handle incoming asynchronous data
 * @param   data Pointer to the asynchronous data received
 * @details Processes data that is not part of a synchronous request-response cycle
 */
void HCI_handle_async_data(HCIAsyncData *data);

/**
 * @brief   Handle HCI layer errors
 * @param   error_code Numeric code representing the specific error
 * @details Manages and potentially logs error conditions in the HCI layer
 */
void HCI_handle_error(uint8_t error_code);

/**
 * @brief   Process incoming HCI events
 * @param   event Pointer to the HCI event structure
 * @details Central event handler for all HCI-related events
 */
void HCI_handle_event(HCIEvent *event);

/**
 * @brief   Handle command complete events from the Bluetooth controller
 * @param   parameters Pointer to event-specific parameters
 * @param   parameter_length Length of the parameters
 * @details Processes successful command completion notifications
 */
void HCI_handle_command_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief   Handle command status events from the Bluetooth controller
 * @param   parameters Pointer to event-specific parameters
 * @param   parameter_length Length of the parameters
 * @details Processes command status notifications that do not indicate full completion
 */
void HCI_handle_command_status_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief   Handle connection complete events
 * @param   parameters Pointer to event-specific parameters
 * @param   parameter_length Length of the parameters
 * @details Processes notifications when a connection is fully established
 */
void HCI_handle_connection_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief   Handle disconnection complete events
 * @param   parameters Pointer to event-specific parameters
 * @param   parameter_length Length of the parameters
 * @details Processes notifications when a connection is fully terminated
 */
void HCI_handle_disconnection_complete_event(uint8_t *parameters, uint8_t parameter_length);

/**
 * @brief   Handle BLE connection complete events
 * @param   subevent_parameters Pointer to subevent-specific parameters
 * @param   subevent_length Length of the subevent parameters
 * @details Processes BLE-specific connection completion notifications
 */
void HCI_handle_BLE_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

/**
 * @brief   Handle BLE connection update complete events
 * @param   subevent_parameters Pointer to subevent-specific parameters
 * @param   subevent_length Length of the subevent parameters
 * @details Processes notifications when a BLE connection's parameters are updated
 */

void HCI_handle_BLE_connection_update_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

/**
 * @brief   Handle BLE enhanced connection complete events
 * @param   subevent_parameters Pointer to subevent-specific parameters
 * @param   subevent_length Length of the subevent parameters
 * @details Processes advanced BLE connection completion notifications
 */
void HCI_handle_BLE_enhanced_connection_complete(uint8_t *subevent_parameters, uint8_t subevent_length);

/**
 * @brief   Retrieve the current HCI layer state
 * @return  HCIState Current state of the HCI layer
 * @details Returns the internal state of the HCI layer for tracking and management
 */
HCIState HCI_get_state(void);

/**
 * @brief   Set the current HCI layer state
 * @param   new_state The state to transition the HCI layer to
 * @details Updates the internal state of the HCI layer
 */
void HCI_set_state(HCIState new_state);

/**
 * @brief   Load firmware for BCM4345 Bluetooth module
 * @return  HCIError Indicates the success or failure of firmware loading
 * @details Loads the appropriate firmware image for the BCM4345 Bluetooth chip
 */
HCIError HCI_bcm4345_load_firmware();

/**
 * @brief   Set the baudrate for the BCM4345 Bluetooth module
 * @param   baudrate Desired communication speed
 * @return  HCIError Indicates the success or failure of setting the baudrate
 * @details Configures the serial communication speed for the Bluetooth module
 */
HCIError HCI_bcm4345_set_baudrate(uint32_t baudrate);

/**
 * @brief   Set the Bluetooth device address
 * @param   bt_addr Pointer to the Bluetooth address to set
 * @return  HCIError Indicates the success or failure of setting the address
 * @details Configures the local Bluetooth device's unique address
 */
HCIError HCI_set_bt_addr(uint8_t *bt_addr);

/**
 * @brief   Retrieve the current Bluetooth device address
 * @param   bt_addr Pointer to buffer where the address will be stored
 * @return  HCIError Indicates the success or failure of retrieving the address
 * @details Fetches the local Bluetooth device's unique address
 */
HCIError HCI_get_bt_addr(uint8_t *bt_addr);

/**
 * @brief   Handle hardware receive interrupt
 * @param   byte Received byte from hardware
 * @details Processes incoming bytes from the Bluetooth hardware interface
 */
void HCI_handle_hw_rx(uint8_t byte);

/**
 * @brief   Check available buffer space
 * @return  uint8_t Number of free buffer slots
 * @details Returns the amount of remaining space in the HCI communication buffer
 */
uint8_t HCI_buffer_space();

/**
 * @brief   Parse an incoming HCI event
 * @param   data Pointer to event data buffer
 * @param   length Length of the event data
 * @return  HCIError Indicates the success or failure of parsing the event
 * @details Decodes and processes an incoming HCI event packet
 */
HCIError HCI_parse_event(uint8_t *data, uint16_t length);

/**
 * @brief Retrieve the status of the BCM4345C0 Bluetooth module
 * @param info Pointer to structure to store module information
 * @return HCIError Indicates the success or failure of retrieving module status
 * @details Fetches detailed status and configuration information about the Bluetooth module
 */
HCIError HCI_get_module_status(BCM4345C0Info *info);

/**
 * @brief   Print the status of the BCM4345C0 Bluetooth module
 * @param   info Pointer to module information structure
 * @details Outputs the current status and configuration of the Bluetooth module to a console or log
 */
void HCI_print_module_status(BCM4345C0Info *info);