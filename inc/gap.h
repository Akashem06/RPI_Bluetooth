#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MAX_SERVICES 10
#define MAX_CHARACTERISTICS_PER_SERVICE 10
#define MAX_CONNECTIONS 1
#define ATT_MTU_DEFAULT 23
#define ATT_MTU_MAX 512

typedef enum {
  GAP_ERROR_SUCCESS,
  GAP_ERROR_INVALID_PARAMETERS,
  GAP_ERROR_NOT_INITIALIZED,
  GAP_ERROR_ALREADY_INITIALIZED,
  GAP_ERROR_HCI_ERROR,
  GAP_ERROR_BUSY
} GAPError;

typedef enum {
  GAP_EVENT_CONNECTED,
  GAP_EVENT_DISCONNECTED,
  GAP_EVENT_CONNECTION_UPDATED,
  GAP_EVENT_SCAN_RESULT
} GAPEventType;

typedef struct {
  GAPEventType type;
  uint16_t connection_handle;
  union {
    struct {
      uint8_t addr[6];
      int8_t rssi;
      uint8_t *adv_data;
      uint8_t adv_data_len;
    } scan_result;
  } params;
} GAPEvent;

typedef struct {
  uint16_t connection_handle;
  uint16_t att_mtu;
  bool connected;
  bool services_discovered;
} GAPConnection;

typedef void (*GAPEventCallback)(GAPEvent *event);

/**
 * @brief   Initialize the Generic Access Profile (GAP) layer
 * @details Sets up the GAP layer with the specified callback for events and device address
 * @param   event_callback Function pointer to handle GAP events
 * @param   bt_addr Pointer to 6-byte Bluetooth address to use for this device
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_init(GAPEventCallback event_callback, uint8_t *bt_addr);

/**
 * @brief   Terminate the GAP layer
 * @details Cleans up resources and stops all GAP activities
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_deinit(void);

/**
 * @brief   Set the device name for advertising and discovery
 * @details Updates the GAP device name used in advertising packets
 * @param   name Null-terminated string containing the device name
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_device_name(const char *name);

/**
 * @brief   Start advertising the device's presence
 * @details Begins sending advertising packets at the specified interval
 * @param   interval_ms Advertisement interval in milliseconds
 * @param   connectable If true, allows connections; if false, broadcast only
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_start_advertising(uint16_t interval_ms, bool connectable);

/**
 * @brief   Set the device's advertising data
 * @details Configures the payload sent in advertising packets
 * @param   adv_data Pointer to advertising data bytes
 * @param   adv_data_len Length of advertising data (max 31 bytes)
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_advertising_data(uint8_t *adv_data, uint8_t adv_data_len);

/**
 * @brief   Stop advertising
 * @details Halts all advertising activity
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_stop_advertising(void);

/**
 * @brief   Set advertising parameters
 * @details Configures detailed parameters for the advertising process
 * @param   type Type of advertising (e.g., connectable, scannable)
 * @param   min_interval_ms Minimum advertising interval in milliseconds
 * @param   max_interval_ms Maximum advertising interval in milliseconds
 * @param   channel_map Bitmap of channels to use (bit 0=ch37, bit 1=ch38, bit 2=ch39)
 * @param   filter_policy Policy for filtering scan/connection requests
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_advertising_parameters(uint8_t type, uint16_t min_interval_ms, uint16_t max_interval_ms,
                                        uint8_t channel_map, uint8_t filter_policy);

/**
 * @brief   Start scanning for nearby advertising devices
 * @details Begins scanning for other BLE devices according to the parameters
 * @param   interval_ms Time between scan cycles in milliseconds
 * @param   window_ms Duration of each scan cycle in milliseconds
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_start_scanning(uint16_t interval_ms, uint16_t window_ms);

/**
 * @brief   Stop scanning for devices
 * @details Halts all scanning activity
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_stop_scanning(void);

/**
 * @brief   Set the device's scan response data
 * @details Configures the payload sent when a scanner requests more data
 * @param   scan_data Pointer to scan response data bytes
 * @param   scan_data_len Length of scan response data (max 31 bytes)
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_scan_response_data(uint8_t *scan_data, uint8_t scan_data_len);

/**
 * @brief   Set scan parameters
 * @details Configures detailed parameters for the scanning process
 * @param   active If true, active scanning (requesting scan responses); if false, passive scanning
 * @param   filter_duplicates If true, filter duplicate advertisements
 * @param   filter_policy Policy for filtering advertisers
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_scan_parameters(bool active, bool filter_duplicates, uint8_t filter_policy);

/**
 * @brief   Initiate a connection to a peer device
 * @details Attempts to establish a connection with the specified device
 * @param   peer_addr Pointer to 6-byte Bluetooth address of the peer device
 * @param   scan_interval_ms Time between connection establishment attempts in milliseconds
 * @param   scan_window_ms Duration of each connection attempt in milliseconds
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_connect(uint8_t *peer_addr, uint16_t scan_interval_ms, uint16_t scan_window_ms);

/**
 * @brief   Disconnect from a connected device
 * @details Terminates the connection identified by the connection handle
 * @param   connection_handle Handle identifying the connection to terminate
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_disconnect(uint16_t connection_handle);

/**
 * @brief   Update connection parameters for an existing connection
 * @details Requests parameter changes for the specified connection
 * @param   connection_handle Handle identifying the connection to update
 * @param   min_interval_ms Minimum desired connection interval in milliseconds
 * @param   max_interval_ms Maximum desired connection interval in milliseconds
 * @param   latency Maximum number of connection events that can be skipped
 * @param   timeout_ms Connection supervision timeout in milliseconds
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_update_connection_parameters(uint16_t connection_handle, uint16_t min_interval_ms, uint16_t max_interval_ms,
                                          uint16_t latency, uint16_t timeout_ms);

/**
 * @brief   Set the device's appearance value
 * @details Sets the GAP appearance characteristic value according to Bluetooth SIG definitions
 * @param   appearance Appearance value as defined in the Bluetooth specification
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_appearance(uint16_t appearance);

/**
 * @brief   Get information about a connection
 * @details Retrieves the current status and parameters of an active connection
 * @param   connection_handle Handle identifying the connection to query
 * @param   connection Pointer to GAPConnection structure to fill with information
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_get_connection_info(uint16_t connection_handle, GAPConnection *connection);

/**
 * @brief   Set preferred ATT MTU size
 * @details Sets the maximum transmission unit size for ATT protocol to request during MTU exchange
 * @param   mtu Desired MTU size (23-512 bytes)
 * @return  GAP_ERROR_SUCCESS on success, or appropriate error code
 */
GAPError GAP_set_preferred_mtu(uint16_t mtu);
