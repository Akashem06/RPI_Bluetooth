#pragma once

#include <stdint.h>
#include <stdbool.h>

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

typedef void (*GAPEventCallback)(GAPEvent* event);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GAPError GAP_init(GAPEventCallback event_callback, uint8_t *bt_addr);

/**
 * @brief
 * @details
 * @param
 * @return
 */
GAPError GAP_set_device_name(const char* name);

/**
 * @brief
 * @details
 * @param
 * @param
 * @return
 */
GAPError GAP_start_advertising(uint16_t interval_ms, bool connectable);

/**
 * @brief
 * @details
 * @return
 */
GAPError GAP_stop_advertising(void);

/**
 * @brief
 * @details
 * @param
 * @param
 * @return
 */
GAPError GAP_start_scanning(uint16_t interval_ms, uint16_t window_ms);

/**
 * @brief
 * @details
 * @return
 */
GAPError GAP_stop_scanning(void);

/**
 * @brief
 * @details
 * @param
 * @param
 * @param
 * @return
 */
GAPError GAP_connect(uint8_t* peer_addr, uint16_t scan_interval_ms, uint16_t scan_window_ms);
/**
 * @brief
 * @details
 * @param
 * @return
 */
GAPError GAP_disconnect(uint16_t connection_handle);
